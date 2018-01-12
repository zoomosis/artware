#include "includes.h"

/* void address_expand(char *line, ADDR *fa, int aka); */

#define TEXTLEN 100





ADDRLIST *fido_lookup(char *name)
{
    int     low, high, mid, f, cond, namelen;
    struct stat buf;
    char    midname[TEXTLEN];
    char    last_name_first[TEXTLEN];
    char   *c, *p, *m, temp[80];
    int     reclength;
    int     nrecs, count=0;
    ADDRLIST *current, *last=NULL, *first=NULL;

    if(name[0] == '\0') return NULL;

    memset(midname, 0, sizeof midname);

    c = midname;     /* Start of temp name buff     */
    p = name;        /* Point to start of name     */
    m = NULL;        /* Init pointer to space     */

    while ((*c = *p++) != '\0') {    /* Go entire length of name  */
        if (*c == ' ')               /* Look for space            */
            m = c;                   /* Save location             */
        c++;
    }

    if (m != NULL) {                         /* If we have a pointer,     */
        *m++ = '\0';                         /* Terminate the first half  */
        strcpy(last_name_first, m);          /* Now copy the last name    */
        strcat(last_name_first, ", ");       /* Insert a comma and space  */
        strcat(last_name_first, midname);    /* Finally copy first half   */
    }
    else
        strcpy(last_name_first, midname);    /* Use whole name otherwise  */

    strlwr(last_name_first);                 /* all lower case            */
    namelen = strlen(last_name_first);       /* Calc length now           */

   stat(cfg.usr.fidouser, &buf);   /* get the file size */

   if ((f = sopen(cfg.usr.fidouser, O_RDONLY|O_BINARY, SH_DENYNO)) == -1) {
        reclength = -1; /* Reset all on open failure */
        return NULL;
    }

    memset(midname, 0, sizeof(midname));
    read(f, midname, sizeof(midname)-1);     /* Read 1 record */
    if(strchr(midname, '\n') == NULL)
       {
       reclength = -1; /* Reset all on open failure */
       return NULL;
       }

    reclength = (int) (strchr(midname, '\n') - midname) + 1; /* FindEnd       */
    nrecs = (int) (buf.st_size / reclength);   /* Now get num of records      */

    /* Binary search algorithm */
    low = 0;
    high = nrecs - 1;

    while (low <= high) {
        mid = low + (high - low) / 2;
        lseek(f, (long) ((long) mid * (long) reclength), SEEK_SET);
        read(f, midname, reclength);
        midname[reclength] = '\0';
        strlwr(midname);
        if ((cond = strncmp(last_name_first, midname, namelen)) < 0)
            high = mid - 1;
        else {
            if (cond > 0)
                low = mid + 1;
            else {               /* Match, match! */

                first=NULL;
                while(--mid >= 0)
                  {
                  lseek(f, (long) ((long) mid * (long) reclength), SEEK_SET);
                  read(f, midname, reclength);
                  midname[reclength] = '\0';
                  strlwr(midname);
                  if (strncmp(last_name_first, midname, namelen) != 0)
                     break;
                  }

                /* We are now positioned just before the first match,
                   or at offset -1 */

                lseek(f, (long) ((long) ++mid * (long) reclength), SEEK_SET);   /* goto first match */

                while(mid <= high)
                  {
                  read(f, midname, reclength);
                  midname[reclength] = '\0';
                  strlwr(midname);
                  if (strncmp(last_name_first, midname, namelen) != 0)
                     break;  /* No match, stop here */

                  if(count++ > 50) break;      /* Too many matches */

                  mid++;

                  c = midname + reclength - 1;

                  while (isspace(*c)) *c-- = '\0'; c--;
                  while (!isspace(*c)) c--;

                  current = mem_calloc(1, sizeof(ADDRLIST));
                  strcpy(current->system, "n.a.");
                  strcpy(current->location, "n.a.");
                  address_expand(c, &current->address, 0);

                  /* find name and terminate, copy */

                  c--;
                  while(isspace(*c)) c--; c++; *c = '\0';
                  if( (c = strchr(midname, ',')) != NULL)
                     {
                     sprintf(current->name, "%s ", (char *)c+2);
                     memset(temp, '\0', sizeof(temp));
                     memcpy(temp, midname, (size_t) (c - midname));
                     strcat(current->name, temp);
                     }
                  else strcpy(current->name, midname);

                  fancy_str(current->name);

                  if(!first)
                     first = current;
                  else
                     last->next = current;

                  last = current;
                  }

                close(f);

                return first;
            }
        }
    }
    close(f);
    return NULL;
}
