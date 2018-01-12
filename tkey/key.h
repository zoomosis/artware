
typedef struct
{
   char   rand1[12];
   dword  generated;
   char   rand2[10];
   dword  keyval;
   char   rand3[7];
   dword  ascvals;
   dword  crc2;
   char   regsite;   // 1 == Gerard, 2 == Evin
   word   serial;
   dword  ascvals2;
   dword  ascvals3;
   char   rand4[39];
   dword  total;
   char   rand5[5];

} STRKEY;


typedef struct
{

   char   bytes[100];

} ARRKEY;


union KEY
{
   STRKEY strkey;
   ARRKEY arrkey;
}

