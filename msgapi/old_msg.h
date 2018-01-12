#ifndef __OLD_MSG_H_DEFINED
#define __OLD_MSG_H_DEFINED

struct _omsg
   {
      byte from[36];
      byte to[36];
      byte subj[72];
      byte date[20];       /* Obsolete/unused ASCII date information        */
/**/  word times;          /* FIDO<tm>: Number of times read                */
      sword dest;          /* Destination node                              */
      sword orig;          /* Origination node number                       */
/**/  word cost;           /* Unit cost charged to send the message         */

      sword orig_net;      /* Origination network number                    */
      sword dest_net;      /* Destination network number                    */

                           /* A TIMESTAMP is a 32-bit integer in the Unix   */
                           /* flavor (ie. the number of seconds since       */
                           /* January 1, 1970).  Timestamps in messages are */
                           /* always Greenwich Mean Time, never local time. */

      struct _stamp date_written;   /* When user wrote the msg              */
      struct _stamp date_arrived;   /* When msg arrived on-line             */

      word reply;          /* Current msg is a reply to this msg number     */
      word attr;           /* Attribute (behavior) of the message           */
      word up;             /* Next message in the thread                    */
   };

#endif

