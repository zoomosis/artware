
//    msgbasetype   = (Hudson,Msg,PassThrough,Squish,jam);

    #define xHMB  0
    #define xSDM  1
    #define xPT   2
    #define xSQ   3
    #define xJAM  4

//    AddressRec    = RECORD
//                      Zone,
//                      Net,
//                      Node,
//                      Point  : WORD;
//                      Domain : STRING [20];
//                    END;  { of RECORD }

//    MailTypeDef   = (Echomail, Netmail, Localmail, Newmail);

    #define xECHO  0
    #define xNET   1
    #define xLOCAL 2
    #define xNEW   3

// Name           : STRING [40]; { Area Name                    }
// Descr          : STRING [60]; { Description                  }
// MailType       : MailTypeDef; { local, netmail or what       }
// StoreType      : msgbasetype;
// MsgDirectory   : STRING [60]; { ... and the corresponding directory }
// ReadSecurity   : Word;        { Read access level            }
// WriteSecurity  : Word;        { Write access level           }
// OriginLine     : String [60]; { Origin line for this echo    }
// AreaAddress    : AddressRec;  { Adress of sender for this echo      }

typedef struct
{

   char name[41];
   char desc[61];
   byte type;
   byte base;
   char dir[61];
   word fill1;
   word fill2;
   char fill3[61];
   NETADDR aka;
   byte group;
   char fill4[161];

} xMailArea;

