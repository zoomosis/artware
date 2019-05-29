#if !defined(__FIDOADR_DEF_)
#define __FIDOADR_DEF_

#include <stdlib.h>

#if !defined(word)
 #ifdef __OS2__
  #define word unsigned short int
 #else
  #define word    unsigned int
  #endif
#endif

#if !defined(byte)
#define byte    unsigned char
#endif

/*
 * Net node spec
 */
typedef struct {
    word            zone;
    word            net;
    word            node;
    word            point;
    char            domain[64];
}               FIDOADR;
#define DEF_FIDOADR {0,0,0,0,{0}}

/*
 * Functions
 */
void            fidoadr_split(char *addr, FIDOADR * fadr);
char           *fidoadr_merge(char *addr, FIDOADR * fadr);
char           *fidostr(char *dest, word zone, word net, word node);
char           *fidoadr_default(char *oldaddr, char *defa);
void            fidosplit(char *src, word * zone, word * net, word * node);
void            hexadr_split(char *hexadr, word * net, word * node);
char           *hexadr_merge(char *hexadr, word net, word node);
#endif
