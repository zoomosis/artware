#include <stdio.h>
#include <string.h>

#include "fidoadr.h"

/*
 * Split a fido address into it's seperate parts.  The following are valid
 * fido address specifictions.
 */
void            fidoadr_split(char *addr, FIDOADR * fadr)
{
    char           *p,
                   *q;
    char            st[255];
    /*
     * Zone
     */
    p = strchr(addr, ':');
    if ((p!=NULL) && (*p == '*'))
        fadr->zone = 65535;
    else if (p)
        fadr->zone = (word) atol(addr);
    else
        fadr->zone = 0;
    /*
     * Net
     */
    p = strchr(addr, '/');
    if (p) {
        p--;
        while (strchr("0123456789*", *p) && (p >= addr))
            p--;
        p++;
        if ((*p == '*') || (fadr->zone == 65535))
            fadr->net = 65535;
        else
            fadr->net = (word) atol(p);
    } else
        fadr->net = 0;
    /*
     * Node
     */
    p = strchr(addr, '/');
    if (p) {
        p++;
        if ((*p == '*') || (fadr->net == 65535))
            fadr->node = 65535;
        else
            fadr->node = (word) atol(p);
    } else
        fadr->node = (word) atol(addr);
    /*
     * Point
     */
    p = strchr(addr, '.');
    if (p) {
        p++;
        if ((*p == '*') || (fadr->node == 65535))
            fadr->point = 65535;
        else
            fadr->point = (word) atol(p);
    } else
        fadr->point = 0;
    /*
     * Domain
     */
    p = strchr(addr, '@');
    if (p) {
        p++;
        strcpy(fadr->domain, p);
    } else
        *(fadr->domain) = '\0';
}
/*
 * Merge the parts specified in FIDOADR into a ASCII fidonet specification
 *
 * Beware of results if you don't pass good values in FIDOADR.
 *
 */
char           *fidoadr_merge(char *addr, FIDOADR * fadr)
{
    static char     tmp[64];
    *addr = '\0';
    if (fadr->zone) {
        ltoa((long) (fadr->zone), tmp, 10);
        strcat(addr, tmp);
        strcat(addr, ":");
    }
    if (fadr->zone || fadr->net) {
        ltoa((long) fadr->net, tmp, 10);
        strcat(addr, tmp);
        strcat(addr, "/");
    }
    ltoa((long) fadr->node, tmp, 10);
    strcat(addr, tmp);
    if (fadr->point && fadr->node) {
        strcat(addr, ".");
        ltoa((long) fadr->point, tmp, 10);
        strcat(addr, tmp);
    }
    if (*(fadr->domain)) {
        strcat(addr, "@");
        strcat(addr, fadr->domain);
    }
    return (addr);
}
/*
 * Split simple 3-part address
 */
void            fidosplit(char *src, word * zone, word * net, word * node)
{
    FIDOADR         fadr = DEF_FIDOADR;
    fidoadr_split(src, &fadr);
    *zone = fadr.zone;
    *net = fadr.net;
    *node = fadr.node;
}
