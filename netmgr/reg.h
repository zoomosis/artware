
#define TIMEDCODE 0xFAFA9090L

typedef struct
{
   char   rand1[12];
   dword  generated;
   char   rand2[10];
   dword  keyval;
   char   rand3[7];
   dword  ascvals;
   dword  crc2;
   char   regsite;
   word   serial;
   dword  ascvals2;
   dword  ascvals3;
   char   rand4[17];
   word   ascvals4;
   word   zero;
   char   rand5[14];
   dword  total;
   char   rand6[5];

} STRKEY;


typedef struct
{

   char   bytes[100];

} ARRKEY;


union KEY
{
   STRKEY strkey;
   ARRKEY arrkey;
};

#define REGISTERED *(cfg.usr.registered) == 1

// ====================================================
