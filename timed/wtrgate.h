#ifndef __WTRGATE_H__
#define __WTRGATE_H__

#pragma pack(__push, 1)

// Watergate areabase definitions.

// For area.type

#define WTRECHO	  0
#define WTRNET 	  1
#define WTRLOCAL	  2

// for area.base

#define WTRPASSTHRU 0
#define WTRSDM 	  1
#define WTRSQUISH   2
#define WTRJAM 	  3

typedef struct
{
    byte deleted;
    byte type;                  // echo, netmail, local
    char usenetname[61];
    char fidoname[61];
    char desc[61];
    dword group;
    word userlist;              // connected users..?

// Fido  

    byte OriginAka;             // BYTE; { offset in Config.NodeNrs }
    dword SeenByAkas;           // LONGINT; { Bits 0-20 add AKA to SeenBy
                                // }

// Usenet

    byte OriginNr;              // BYTE; { 1/2 for Config.Origins[],
                                // 0=custom }
    char Origin[62];            // STRING[61]; { Custom Origin }

// Fidonet Local Base

    byte base;                  // JAM, Squish etc.
    char path[80];              // FidoMsgPath : STRING[ 79 ]; { Path to
                                // base }
    word age;                   // FidoMsgAge : Integer; { Days to keep }
    word limit;                 // FidoMsgLimit : Integer; { Numb to keep
                                // }

// Usenet special

    byte moderated;             // Moderated : byte; { Moderated }
    char moderator[51];         // Moderator : STRING[50]; { Moderator }

} WTRAREA;

typedef struct
{

    word zone, net, node, point;
    char fill[26];

} WTRAKA;

#pragma pack(__pop)

#endif
