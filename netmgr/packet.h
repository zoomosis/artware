
typedef struct pktheadertype
{
  word          orignode,
                destnode,
                year,
                month,
                day,
                hour,
                minute,
                second,
                baud,
                pkttype,
                orignet,    // -1 if from a point
                destnet;
  char          productcode,
                revision,
                password[8];
  word          origzone,
                destzone,
                auxnet,
                CWvalcopy;  // inverted capability word
  unsigned char ProductCode,
                Revision;
  word          capabilword,
                origzone2,
                destzone2,
                origpoint2,
                destpoint2;
  unsigned char productdata[4];

} PKTHEADER;

// ===================================================

typedef struct packedmsgtype
{

  word  msgtype,    // 2
        orignode,
        destnode,
        orignet,
        destnet,
        attribute,
        cost;

} PACKEDMSG;


