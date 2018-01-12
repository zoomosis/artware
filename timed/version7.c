#include "includes.h"


/* This table has been modified to minimize searches */
char unwrk[] = " EANROSTILCHBDMUGPKYWFVJXZQ-'0123456789";

void btree         (int (*)(void *, int), long startblock, int onematch);
/* int  get_ver7_info (unsigned long, NETADDR *); */
void unpk          (char *instr,char *outp,int many);
int  addr_compare  (void *, int);
int  name_compare  (void *, int);
int  get7node      (long blockno, struct _ndx  *ndx);
static void reset  (void);
static int addmatch(long offset);


#define ALLMATCHES 0
#define ONEMATCH   1

static long nodestart = 0;
static long namestart = 0;

int  curindex = 0;       /* File handle for index that is searched */
FILE *nodex = NULL;      /* File handle for nodex.dat              */


static ADDRLIST *first  = NULL; /* Pointer to first match found           */

static char matchname[80];
static int namelen;
static NETADDR matchaddress;



char *fancy_str (char *string);


ADDRLIST *ver7find (NETADDR *opus_addr)
{
    char filename[100];
    struct _ndx *ctl;

    reset();

    if(cfg.usr.nodelist[0] == '\0')
       return 0;

    matchaddress.zone  = opus_addr->zone;
    matchaddress.net   = opus_addr->net;
    matchaddress.node  = opus_addr->node;
    matchaddress.point = opus_addr->point;


    sprintf(filename, "%s\\nodex.ndx", cfg.usr.nodelist);

    if ((curindex = sopen(filename, O_RDONLY|O_BINARY, SH_DENYNO)) == -1)
        {
        sprintf(msg, "Can't open %s!", filename);
        Message(msg, -1, 0, YES);
        return NULL;                            /* no file, no work to do */
        }

    if(nodestart == 0)     /* Is root already known? Read ctl-block if not */
       {
       ctl = mem_calloc(1, sizeof(struct _ndx));
       if ( (get7node (0, ctl)) ==  -1 )
          {
          close(curindex);
          mem_free(ctl);
          return NULL;
          }
        nodestart = ctl->ndx.CtlBlk.CtlRoot;
        mem_free(ctl);
        }

    btree (addr_compare, nodestart, ONEMATCH);

    close(curindex);
    if(nodex != NULL) fclose(nodex);

    return first;

}


int addr_compare (void *key, int len)
{
   int k;

   k = ((NETADDR *)key)->zone - matchaddress.zone;
   if (k)
      return (k);

   k = ((NETADDR *)key)->net - matchaddress.net;
   if (k)
      return (k);

   k = ((NETADDR *)key)->node - matchaddress.node;
   if (k)
      return (k);
/*
 * Node matches.
 *
 * The rule for points:
 *  1) If len == 6, treat key value for Point as Zero.
 *  2) Return comparison of key Point and desired Point.
 */
   if (len == 6)
      ((NETADDR *)key)->point = 0;

   return ((NETADDR *)key)->point - matchaddress.point;
}


ADDRLIST *opususer (char *name)
{
   char last_name_first[80];
   char midname[80];
   char *c, *p, *m;
   char filename[100];
   struct _ndx *ctl;


   reset();

   if(cfg.usr.nodelist[0] == '\0')
      return NULL;


   c = midname;                                  /* Start of temp name buff   */
   p = name;                                     /* Point to start of name    */
   m = NULL;                                     /* Init pointer to space     */

   *c = *p++;
   while (*c)                                    /* Go entire length of name  */
   {
      if (*c == ' ')                             /* Look for space            */
         m = c;                                  /* Save location             */
      c++;
      *c = *p++;
   }

   if (m != NULL)                                /* If we have a pointer,     */
   {
      *m++ = '\0';                               /* Terminate the first half  */
      (void) strcpy (last_name_first, m);        /* Now copy the last name    */
      (void) strcat (last_name_first, ", ");     /* Insert a comma and space  */
      (void) strcat (last_name_first, midname);  /* Finally copy first half   */
   }
   else (void) strcpy (last_name_first, midname);/* Use whole name otherwise  */

   (void) fancy_str (last_name_first);           /* Get caps in where needed  */
   namelen = (int) strlen (last_name_first);     /* Calc length now           */

   strcpy(matchname, last_name_first);

   sprintf(filename, "%s\\sysop.ndx", cfg.usr.nodelist);

   if ((curindex = sopen(filename, O_RDONLY|O_BINARY, SH_DENYNO)) == -1)
        {
        sprintf(msg, "Can't open %s!", filename);
        Message(msg, -1, 0, YES);
        return NULL;                            /* no file, no work to do */
        }


   if(namestart == 0)     /* Is root already known? Read ctl-block if not */
       {
       ctl = mem_calloc(1, sizeof(struct _ndx));
       if(get7node(0, ctl) == -1)
          {
          mem_free(ctl);
          close(curindex);
          return NULL;
          }
        namestart = ctl->ndx.CtlBlk.CtlRoot;
        mem_free(ctl);
        }

   btree (name_compare, namestart, ALLMATCHES);

   close(curindex);
   if(nodex != NULL) fclose(nodex);

   return first;

}


int name_compare (void *key, int len)
{
   return (strnicmp ((char *)key, matchname, (unsigned int) min (namelen,len)));
}

/*
 * General V7 nodelist engine. Used by both the by-node and by-sysop
 * lookups.
 *
 * Thanks to Phil Becker for showing me how nice it looks in assembler.
 * It helped me see how nice it could be in C.
 *
 * (I know, Phil, it's still nicer in assembler!)
 *
 */



void btree (int (*compare)(void *key, int len), long startblock, int onematch)
{
    int j, k, l;
    struct _ndx *thisnode;
    struct _IndxRef  *ip = NULL;
    struct _LeafRef  *lp = NULL;
    char aline[160];
    char  *tp;
    char *np;
    long nextblock;
    int count;

    thisnode = mem_calloc(1, sizeof(struct _ndx));

    if (get7node(startblock, thisnode) == -1)
        {
        mem_free(thisnode);
        return;
        }

/*
 * Follow the node tree until we either match a key right in the index
 * node, or locate the leaf node which must contain the entry.
 */

    while (thisnode->ndx.INodeBlk.IndxFirst != -1)
       {
       if ((count = thisnode->ndx.INodeBlk.IndxCnt) == 0)
          {
          mem_free(thisnode);
          return;
          }


       for (j = 0; j < count; j++) /* check 20 or less */
          {
          ip = &(thisnode->ndx.INodeBlk.IndxRef[j]);
          tp = (char  *) thisnode + ip->IndxOfs;

          k = l = ip->IndxLen;

          for (np = aline; k > 0; k--)
              *np++ = *tp++;


          k = (*compare) ((void *)aline, l);

          if (k > 0)
             break;

          if (k == 0)   /* Match right in the index */
             {

             if(!onematch)
                {
                if (j == 0)
                   btree(compare, thisnode->ndx.INodeBlk.IndxFirst, onematch);
                else
                   btree(compare, (thisnode->ndx.INodeBlk.IndxRef[j-1]).IndxPtr, onematch);
                }

             /* Add this match to list */

             if(
                 (addmatch(thisnode->ndx.INodeBlk.IndxRef[j].IndxData) == -1) ||
                 (onematch != 0)
               )
                 {
                 mem_free(thisnode);
                 return;
                 }
             }
          }

       if (j == 0)
          nextblock = thisnode->ndx.INodeBlk.IndxFirst;
       else
          nextblock = (thisnode->ndx.INodeBlk.IndxRef[--j]).IndxPtr;

       if (get7node(nextblock, thisnode) == -1)
          {
          mem_free(thisnode);
          return;
          }

       }

/*
 * We can only get here if we've found the leafnode which must
 * contain our entry.
 *
 * Find our guy here or die trying.
 */

    if ((count = thisnode->ndx.LNodeBlk.IndxCnt) != 0)
       {
       /* Search for a higher key */


       for (j = 0; j < count; j++) /* check 30 or less */
          {
          lp = &(thisnode->ndx.LNodeBlk.LeafRef[j]);
          tp = (char  *) thisnode + lp->KeyOfs;

         /* Leaf node search!!!! */

          k = l = lp->KeyLen;

          for (np = aline; k > 0; k--)

              *np++ = *tp++;

         k = (*compare) ((void *)aline, l);

          if (k > 0)
             break;
          if (k == 0)
             {
             /* Add this match */
             if(
                 (addmatch((thisnode->ndx.LNodeBlk.LeafRef[j]).KeyVal) == -1) ||
                 (onematch != 0)
               )
               break;
             }
          }
       }

    mem_free(thisnode);
}



int get7node(long blockno, struct _ndx  *ndx)
{

   if(blockno < 0)
     {
     Message("Index block number < 0 requested!", -1, 0, YES);
     return -1;
     }

    if(lseek (curindex, (long) blockno * 512L, SEEK_SET) != (long) blockno * 512L)
       {
       Message("Error seeking nodelist!", -1, 0, YES);
       return -1;
       }

    if ( (read (curindex, ndx, sizeof (struct _ndx))) != sizeof(struct _ndx))
       {
       Message("Error reading nodelist!", -1, 0, YES);
       return -1;
       }

    return 0;
}


/* ====================================================================
 * unpack a dense version of a symbol (base 40 polynomial)
 * ====================================================================
 */
void unpk(char *instr,char *outp,int count)
{
    struct chars {
           unsigned char c1;
           unsigned char c2;
           };

    union {
          word w1;
          struct chars d;
          } u;

   register int i, j;
   char obuf[4];

   outp[0] = '\0';

   while (count) {
       u.d.c1 = *instr++;
       u.d.c2 = *instr++;
       count -= 2;
       for(j=2;j>=0;j--) {
           i = u.w1 % 40;
           u.w1 /= 40;
            obuf[j] = unwrk[i];
           }
       obuf[3] = '\0';
       (void) strcat (outp, obuf);
       }
}



char *fancy_str (char *string)
{
   register int flag = 0;
   char *s;

   s = string;

   while (*string)
      {
      if (isalpha (*string))                     /* If alphabetic,     */
         {
         if (flag)                               /* already saw one?   */
            *string = (char) tolower (*string);  /* Yes, lowercase it  */
         else
            {
            flag = 1;                            /* first one, flag it */
            *string = (char) toupper (*string);  /* Uppercase it       */
            }
         }
      else /* if not alphabetic  */ flag = 0;    /* reset alpha flag   */
      string++;
      }

   return (s);
}



static int addmatch(long offset)
{
   ADDRLIST *current;
   static ADDRLIST *last=NULL;
   char temp[256];
   int howmany = 0;
   struct _vers7 vers7;
   char my_phone[41];
   char my_pwd[31];
   char aline[160];
   char aline2[160];


   if(nodex == NULL) /* Open nodex if not already open, and stay open for more matches */
      {
      sprintf(temp, "%s\\nodex.dat", cfg.usr.nodelist);

      if ((nodex = _fsopen (temp, "rb", SH_DENYNO)) == NULL)    /* open it*/
         {
         Message("Can't open nodex.dat!", -1, 0, YES);
         return -1;
         }
      }


    if (fseek (nodex, (long int) offset, SEEK_SET))              /* point at record    */
       {
       Message("Error seeking nodex.dat!", -1, 0, YES);
       return -1;
       }

    if (fread ((char *)&vers7, sizeof (struct _vers7), 1, nodex) != 1)
       {
       Message("Error reading nodelist record!", -1, 0, YES);
       return -1;
       }


    for(current=first; current; current=current->next)
      {
      if(                                      /* Check for dupe */
         (current->address.zone  == vers7.Zone ) &&
         (current->address.net   == vers7.Net  ) &&
         (current->address.node  == vers7.Node ) &&
         (
           ((vers7.NodeFlags & B_point) && (current->address.point == vers7.HubNode)) ||
           ((!(vers7.NodeFlags & B_point)) && (current->address.point == 0))
         )

        )
        return 0;

      if(howmany++ > 50) return -1;    /* Don't get in (semi) endless loops */
      }

   current = mem_calloc(1, sizeof(ADDRLIST));

   current->address.zone  = vers7.Zone;
   current->address.net   = vers7.Net ;
   current->address.node  = vers7.Node;
   current->address.point = (vers7.NodeFlags & B_point) ? vers7.HubNode : 0;

   /* Not needed now */

   memset(my_phone,'\0',40);
   fread (my_phone, min(vers7.Phone_len, 39), 1, nodex);
   memset(current->phone, '\0', sizeof(current->phone));
   strncpy(current->phone, my_phone, 19);

   /* not needed now */

   memset(my_pwd,'\0',30);
   fread (my_pwd, min(vers7.Password_len, 29), 1, nodex);

   memset(aline,  '\0', 160);
   memset(aline2, '\0', 160);

   if (!fread (aline2, min(159,vers7.pack_len), 1, nodex))
        {
        Message("Error reading nodelist!", -1, 0, YES);
        mem_free(current);
        return -1;
        }

   unpk(aline2,aline,vers7.pack_len);

   memcpy (current->system, aline, min(29, vers7.Bname_len));
   current->system[29]='\0';
   fancy_str (current->system);

   memcpy(current->name, aline + vers7.Bname_len, min(39, vers7.Sname_len));
   current->name[39]='\0';
   fancy_str(current->name);

   memcpy(current->location, aline + vers7.Bname_len + vers7.Sname_len, min(29, vers7.Cname_len));
   current->name[30]='\0';
   fancy_str(current->location);

   current->baud = vers7.BaudRate * 300;

   if(vers7.NodeFlags & B_CM)
     strcpy(current->flags, "CM");

   current->next = NULL;         /* Just to be sure, clean link */

   if(first == NULL)             /* first element found */
      first = current;
   else last->next = current;    /* add to list */

   last = current;               /* remember last */


   return 0;

}



static void reset(void)
{

    curindex = 0;       /* File handle for index that is searched */
    nodex    = NULL;    /* File handle for nodex.dat              */
    first    = NULL;    /* Pointer to first match found           */

    memset(matchname, '\0', sizeof(matchname));
    namelen = 0;
    memset(&matchaddress, '\0', sizeof(NETADDR));

}
