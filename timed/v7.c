#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>
#include <io.h>
#include <time.h>
#include <share.h>

#include "includes.h"
#include "cbtreex.h"

/* This table has been modified to minimize searches */
char unwrk[] = " EANROSTILCHBDMUGPKYWFVJXZQ-'0123456789";


static Cbtree *cbt;             // Handle for Mix database toolkit
static FILE *nodex = NULL;      // File handle for NODEX.DAT file


int V7init(int syslookup);
short int V7search(char *searchitem, int len, ADDRLIST * found);
int ReadNodex(long offset, ADDRLIST * current);
short int name_compare(char *keya, short int lena, char *keyb,
                       short int lenb);
short int addr_compare(char *keya, short int lena, char *keyb,
                       short int lenb);

void unpk(char *instr, char *outp, int count);
char *LastNameFirst(char *name);
char *fancy_str(char *string);
void ConvertV7Flags(ADDRLIST * current, word node, char modem);


// ==============================================================

int V7init(int syslookup)
{
    char temp[120];

    if (cbinit(DEFAULT_BUFCNT, DEFAULT_BUFSIZE) != OK)
    {
        return -1;
    }

    if (syslookup)
    {
        sprintf(temp, "%s\\sysop.ndx", cfg.usr.nodelist);
        cbt = cbopen(temp, name_compare);
    }
    else
    {
        sprintf(temp, "%s\\nodex.ndx", cfg.usr.nodelist);
        cbt = cbopen(temp, addr_compare);
    }

    if (cbt == NULL)
    {
        cbexit();
        return -1;
    }

    return 0;

}

// ==============================================================

int V7close(void)
{

    if (nodex)
        fclose(nodex);
    nodex = NULL;
    cbclose(cbt);
    cbexit();
    return 0;

}


// ==============================================================

short int V7search(char *searchitem, int len, ADDRLIST * found)
{
    Item item;
    short int status;

    status = cbfind(cbt, searchitem, len, &item);

    if (status == ERROR)
    {
        return ERROR;
    }

    if (status != FOUND)        // Not found, end of list.
        return status;

    // Otherwise we are doing OK. Let's get the node details!

    if (found)
    {
        if (ReadNodex(item, found) != 0)
            return ERROR;
    }

    return status;

}

// ==============================================================

short int V7curr(ADDRLIST * found)
{
    Item item;
    short status;

    status = cbcurr(cbt, &item);

    if (status != OK)
        return status;

    if (found)
    {
        if (ReadNodex(item, found) != 0)
            return ERROR;
    }

    return OK;

}

// ==============================================================

short int V7next(ADDRLIST * found)
{
    Item item;
    short status;

    status = cbnext(cbt, &item);

    if (status != OK)
        return status;

    if (found)
    {
        if (ReadNodex(item, found) != 0)
            return ERROR;
    }

    return OK;

}


// ==============================================================

short int V7prev(ADDRLIST * found)
{
    Item item;
    short status;

    status = cbprev(cbt, &item);

    if (status != OK)
        return status;

    if (found)
    {
        if (ReadNodex(item, found) != 0)
            return ERROR;
    }

    return OK;

}

// ==============================================================

short int V7head(ADDRLIST * found)
{
    Item item;
    short status;

    status = cbhead(cbt, &item);

    if (status != OK)
        return status;

    if (found)
    {
        if (ReadNodex(item, found) != 0)
            return ERROR;
    }

    return OK;

}

// ==============================================================

short int V7tail(ADDRLIST * found)
{
    Item item;
    short status;

    status = cbtail(cbt, &item);

    if (status != OK)
        return status;

    if (found)
    {
        if (ReadNodex(item, found) != 0)
            return ERROR;
    }

    return OK;

}

// ==============================================================

short int V7mark(void)
{

    return cbmark(cbt);

}

// ==============================================================

short int V7findmark(ADDRLIST * found)
{
    Item item;
    short status;

    status = cbfindmark(cbt, &item);

    if (status != OK)
        return status;

    if (found)
    {
        if (ReadNodex(item, found) != 0)
            return ERROR;
    }

    return OK;

}

// ==============================================================


short int name_compare(char *keya, short int lena, char *keyb,
                       short int lenb)
{
    int result;

    result = strnicmp(keya, keyb, (unsigned int)min(lena, lenb));

    if (!result)
        return EQUAL;

    return result < 0 ? LESS : GREATER;
}

// ==============================================================

short int addr_compare(char *keya, short int lena, char *keyb,
                       short int lenb)
{
    int k;
    int pointa, pointb;

    k = ((NETADDR *) keya)->zone - ((NETADDR *) keyb)->zone;
    if (k)
        goto endit;

    k = ((NETADDR *) keya)->net - ((NETADDR *) keyb)->net;
    if (k)
        goto endit;

    k = ((NETADDR *) keya)->node - ((NETADDR *) keyb)->node;
    if (k)
        goto endit;

/*
 * Node matches.
 *
 * The rule for points:
 *  1) If len == 6, treat key value for Point as Zero.
 *  2) Return comparison of key Point and desired Point.
 */
    if (lena == 6)
        pointa = 0;
    else
        pointa = ((NETADDR *) keya)->point;

    if (lenb == 6)
        pointb = 0;
    else
        pointb = ((NETADDR *) keyb)->point;

    k = pointa - pointb;

  endit:

    if (!k)
        return EQUAL;

    return k < 0 ? LESS : GREATER;
}

// ==============================================================

int ReadNodex(long offset, ADDRLIST * current)
{
    char temp[256];
    struct _vers7 vers7;
    char my_phone[41];
    char my_pwd[31];
    char aline[160];
    char aline2[160];

    memset(current, '\0', sizeof(ADDRLIST));

    if (nodex == NULL)          /* Open nodex if not already open, and
                                   stay open for more matches */
    {
        sprintf(temp, "%s\\nodex.dat", cfg.usr.nodelist);

        if ((nodex = _fsopen(temp, "rb", SH_DENYNO)) == NULL) /* open it */
        {
            Message("Can't open nodex.dat!", -1, 0, YES);
            return 0;
        }
    }

    if (fseek(nodex, (long int)offset, SEEK_SET)) /* point at record */
    {
        Message("Error seeking nodex.dat!", -1, 0, YES);
        return 0;
    }

    if (fread((char *)&vers7, sizeof(struct _vers7), 1, nodex) != 1)
    {
        Message("Error reading nodelist record!", -1, 0, YES);
        return 0;
    }

    current->address.zone = vers7.Zone;
    current->address.net = vers7.Net;
    current->address.node = vers7.Node;
    current->address.point =
        (vers7.NodeFlags & B_point) ? vers7.HubNode : 0;

    /* Not needed now */

    memset(my_phone, '\0', 40);
    fread(my_phone, min(vers7.Phone_len, 39), 1, nodex);
    memset(current->phone, '\0', sizeof(current->phone));
    strncpy(current->phone, my_phone, 19);

    /* not needed now */

    memset(my_pwd, '\0', 30);
    fread(my_pwd, min(vers7.Password_len, 29), 1, nodex);

    memset(aline, '\0', 160);
    memset(aline2, '\0', 160);

    if (!fread(aline2, min(159, vers7.pack_len), 1, nodex))
    {
        Message("Error reading nodelist!", -1, 0, YES);
        return -1;
    }

    unpk(aline2, aline, vers7.pack_len);

    memcpy(current->system, aline, min(29, vers7.Bname_len));
    current->system[29] = '\0';
    fancy_str(current->system);

    memcpy(current->name, aline + vers7.Bname_len,
           min(39, vers7.Sname_len));
    current->name[39] = '\0';
    fancy_str(current->name);

    memcpy(current->location, aline + vers7.Bname_len + vers7.Sname_len,
           min(29, vers7.Cname_len));
    current->name[30] = '\0';
    fancy_str(current->location);

    current->baud = (dword) ((dword) vers7.BaudRate * 300L);

    ConvertV7Flags(current, vers7.NodeFlags, vers7.ModemType);

    return 0;

}

// ==============================================================

void ConvertV7Flags(ADDRLIST * current, word node, char modem)
{
    int i;
    int match;


    for (i = 0; i < MAXV7FLAGS; i++)
    {
        if (cfg.V7flags[i].value == 0) // No more flags defined.
            break;

        match = 0;
        switch (cfg.V7flags[i].type)
        {
        case MODEMBIT:
            if (cfg.V7flags[i].value & modem)
                match = 1;
            break;

        case MODEMVALUE:
            if (cfg.V7flags[i].value == modem)
                match = 1;
            break;

        case NODEBIT:
            if (cfg.V7flags[i].value & node)
                match = 1;
            break;
        }

        if (match)              // A match, so add this flag.
        {
            if ((strlen(current->flags) + 1 +
                 strlen(cfg.V7flags[i].flag)) < 60)
            {
                if (current->flags[0] != '\0')
                    strcat(current->flags, ",");
                strcat(current->flags, cfg.V7flags[i].flag);
            }
        }
    }

}

// ==============================================================

char *fancy_str(char *string)
{
    register int flag = 0;
    char *s;

    s = string;

    while (*string)
    {
        if (isalpha(*string))   /* If alphabetic, */
        {
            if (flag)           /* already saw one? */
                *string = (char)tolower(*string); /* Yes, lowercase it */
            else
            {
                flag = 1;       /* first one, flag it */
                *string = (char)toupper(*string); /* Uppercase it */
            }
        }
        else                    /* if not alphabetic */
            flag = 0;           /* reset alpha flag */
        string++;
    }

    return (s);
}



/* ====================================================================
 * unpack a dense version of a symbol (base 40 polynomial)
 * ====================================================================
 */

void unpk(char *instr, char *outp, int count)
{
    static int num = 0;

    struct chars
    {
        unsigned char c1;
        unsigned char c2;
    };

    union
    {
        word w1;
        struct chars d;
    } u;

    register int i, j;
    char obuf[4];

    outp[0] = '\0';
    num++;

    while (count)
    {
        u.d.c1 = *instr++;
        u.d.c2 = *instr++;
        count -= 2;
        for (j = 2; j >= 0; j--)
        {
            i = u.w1 % 40;
            u.w1 /= 40;
            obuf[j] = unwrk[i];
        }
        obuf[3] = '\0';
        (void)strcat(outp, obuf);
    }

}
