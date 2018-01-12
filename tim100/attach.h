
int check_attach   (XMSG *hdr, MSG *areahandle, AREA *area);
void     writefa(XMSG *hdr, MSG *areahandle, char *subject, AREA *area);

typedef struct _filelist

{

   char              name[30];   /* Name of the file */
   char              *path;      /* Full path to location */
   long              size;       /* Size of the file */
   int               tagged;     /* 1 = tagged */
   struct _filelist  *next;      /* Link to next file-rec */

} FILELIST;
