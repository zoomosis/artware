#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>

#ifdef UNIX
#include <sys/types.h>
#include <dirent.h>
#endif

#ifdef WIN32
#include <dirent.h>
#endif

#include "includes.h"
#include "patmat.h"

#define MAXMATCHES 100

static FILE *fp;

struct addr
{
    int zone, net, node;
};

static struct addr prev_node;

static struct
{
    int type;
    struct addr addr;
    char system[256];
    char location[256];
    char name[256];
    char phone[256];
    int baud;
    char flags[256];
}
node;

#define TYPE_NONE   0
#define TYPE_ZONE   1
#define TYPE_REGION 2
#define TYPE_HOST   3
#define TYPE_HUB    4
#define TYPE_DOWN   5
#define TYPE_HOLD   6
#define TYPE_PVT    7

static int iscomment(char *str)
{
    return *str == ';';
}

static char *underscore_to_space(char *str)
{
    char *p;

    p = str;

    while (*p != '\0')
    {
        if (*p == '_')
        {
            *p = ' ';
        }

        p++;
    }

    return str;
}

static char *space_to_underscore(char *str)
{
    char *p;

    p = str;

    while (*p != '\0')
    {
        if (*p == ' ')
        {
            *p = '_';
        }

        p++;
    }

    return str;
}

static void zero_addr(struct addr *x)
{
    x->zone = 0;
    x->net = 0;
    x->node = 0;
}

static int strmatch(char *str1, char *str2)
{
    return strncmp(str1, str2, strlen(str2)) == 0;
}

static int strisdigit(char *str)
{
    char *p;

    p = str;

    while (*p != '\0')
    {
        if (!isdigit(*p))
        {
            return 0;
        }
        p++;
    }

    return 1;
}

static int parse(char *str)
{
    char *p;

    if (iscomment(str))
    {
        return 1;
    }

    node.type = TYPE_NONE;

    p = strtok(str, ",");

    if (p == NULL)
    {
        return 0;
    }

    if (strmatch(p, "Zone"))
    {
        node.type = TYPE_ZONE;

        p = strtok(NULL, ",");

        if (p == NULL)
        {
            return 0;
        }

        node.addr.zone = strtol(p, NULL, 10);
        node.addr.net = node.addr.zone;
        node.addr.node = 0;
    }
    else if (strmatch(p, "Region"))
    {
        node.type = TYPE_REGION;

        p = strtok(NULL, ",");

        if (p == NULL)
        {
            return 0;
        }

        node.addr.zone = prev_node.zone;
        node.addr.net = strtol(p, NULL, 10);
        node.addr.node = 0;
    }
    else if (strmatch(p, "Host"))
    {
        node.type = TYPE_HOST;

        p = strtok(NULL, ",");

        if (p == NULL)
        {
            return 0;
        }

        node.addr.zone = prev_node.zone;
        node.addr.net = strtol(p, NULL, 10);
        node.addr.node = 0;
    }
    else if (strmatch(p, "Hub"))
    {
        node.type = TYPE_HUB;

        p = strtok(NULL, ",");

        if (p == NULL)
        {
            return 0;
        }
    }
    else if (strmatch(p, "Pvt"))
    {
        node.type = TYPE_PVT;

        p = strtok(NULL, ",");

        if (p == NULL)
        {
            return 0;
        }
    }
    else if (strmatch(p, "Hold"))
    {
        node.type = TYPE_HOLD;

        p = strtok(NULL, ",");

        if (p == NULL)
        {
            return 0;
        }
    }
    else if (strmatch(p, "Down"))
    {
        node.type = TYPE_DOWN;

        p = strtok(NULL, ",");

        if (p == NULL)
        {
            return 0;
        }
    }
    else
    {
        /* must be a normal node entry, but make sure it is! */

        if (!strisdigit(p))
        {
            return 0;
        }
    }

    if (node.type != TYPE_ZONE && node.type != TYPE_REGION &&
      node.type != TYPE_HOST)
    {
        node.addr.zone = prev_node.zone;
        node.addr.net = prev_node.net;
        node.addr.node = strtol(p, NULL, 10);
    }

    /* system name */

    p = strtok(NULL, ",");

    if (p == NULL)
    {
        return 0;
    }

    strcpy(node.system, p);

    /* system location */

    p = strtok(NULL, ",");

    if (p == NULL)
    {
        return 0;
    }

    strcpy(node.location, p);

    /* sysop name */

    p = strtok(NULL, ",");

    if (p == NULL)
    {
        return 0;
    }

    strcpy(node.name, p);

    /* phone number */

    p = strtok(NULL, ",");

    if (p == NULL)
    {
        return 0;
    }

    strcpy(node.phone, p);

    /* baud rate */

    p = strtok(NULL, ",");

    if (p == NULL)
    {
        return 0;
    }

    node.baud = strtol(p, NULL, 10);

    /* nodelist flags */

    p += strlen(p) + 1;

    strcpy(node.flags, p);

    prev_node = node.addr;

    return 1;
}

static int newest(char *dest_path, char *src_pattern)
{
    DIR *dp;
    struct dirent *ent;
    char msg[250];
    char pattern[250];
    char dir[250], name[250], ext[250];
    struct stat new_st;

    fnsplit(src_pattern, NULL, dir, name, ext);

    dp = opendir(dir);

    if (dp == NULL)
    {
        /* directory doesn't exist */

        sprintf(msg, "FidoNodelist directory '%s' does not exist!", dir);
        Message(msg, -1, 0, YES);   

    	return EXIT_FAILURE;
    }

    ent = readdir(dp);

    if (ent == NULL)
    {
        /* directory is empty */

        sprintf(msg, "FidoNodelist directory '%s' is empty!", dir);
        Message(msg, -1, 0, YES);   

    	return EXIT_FAILURE;
    }

    *dest_path = '\0';
    memset(&new_st, 0, sizeof new_st);
    
    while (ent != NULL)
    {
        strcpy(pattern, name);
        strcat(pattern, ext);

        if (patmat(ent->d_name, pattern))
        {
            char path[250];
            struct stat st;

            strcpy(path, dir);
            strcat(path, ent->d_name);

            stat(path, &st);

            /* compare timestamps based on modification time.  ignore directories */

            if ((st.st_mode & S_IFDIR) == 0 && st.st_mtime > new_st.st_mtime)
            {
            	strcpy(dest_path, path);
            	new_st = st;
            }
        }

        ent = readdir(dp);
    }

    closedir(dp);

    if (*dest_path == '\0')
    {
    	/* no match was found */

        sprintf(msg, "Could not match FidoNodelist pattern '%s'!", pattern);
        Message(msg, -1, 0, YES);   

    	return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

ADDRLIST *fido_nodelist_lookup(char *name)
{
    ADDRLIST *current, *last = NULL, *first = NULL;
    char buf[1024];
    int i;

    if (*name == '\0')
    {
        return NULL;
    }

    if (newest(buf, cfg.usr.fidonodelist) != EXIT_SUCCESS)
    {
    	return NULL;
    }

    fp = fopen(buf, "r");

    if (fp == NULL)
    {
        char msg[250];

        sprintf(msg, "Can't open FidoNodelist file '%s'!", buf);
        Message(msg, -1, 0, YES);   

        return NULL;
    }

    zero_addr(&node.addr);
    zero_addr(&prev_node);

    space_to_underscore(name);

    i = 0;

    while (fgets(buf, sizeof buf, fp) != NULL)
    {
        StripCR(buf);

        if (parse(buf) && stristr(node.name, name) != NULL && i < MAXMATCHES)
        {
            /* found a match */

            i++;

            current = mem_calloc(1, sizeof *current);

            strcpy(current->system, underscore_to_space(node.system));
            strcpy(current->location, underscore_to_space(node.location));
            strcpy(current->name, underscore_to_space(node.name));
            strcpy(current->phone, underscore_to_space(node.phone));
            strcpy(current->flags, underscore_to_space(node.flags));

            current->baud = node.baud;

            current->address.zone = node.addr.zone;
            current->address.net = node.addr.net;
            current->address.node = node.addr.node;

            if (first == NULL)
            {
                first = current;
            }
            else
            {
                last->next = current;
            }

            last = current;
        }
    }

    underscore_to_space(name);

    fclose(fp);

    return first;
}
