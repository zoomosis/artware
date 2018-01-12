
int SQReadIndex(char *path)

{
   int index, last;
   char idxname[100], lastname[100];
   unsigned bytes;

   sprintf(idxname, "%s.SQI", path);
   sprintf(lastname, "%s.SQL", path);

   idxmem.n = 0;

   if ((index = open(idxname, O_RDONLY | O_BINARY | SH_DENYNO)) == -1)
      return NULL;

   if ( ((lseek(index, 0L, SEEK_END)) == -1) ||
        ((idxmem.n = tell(index)) == -1) )
      {
      close(index);
      return -1;
      }

   if (idxmem.n <= 0)
      {
      close(index);
      return -1;
      }

   idxmem.idxptr = xmalloc(idxmem.n);

   lseek(index, 0L, SEEK_SET);

   bytes = read(index, idxmem.idxptr, idxmem.n);

   if ( bytes != idxmem.n )
      {
      free(idxmem.idxptr);
      close(index);
      return -1;
      }

   close(index);

   if ( ((last = open(lastname, O_RDONLY | O_BINARY | SH_DENYNO)) == -1))
      idxmem.last = 0;
   else
      {
      if (read(last, &idxmem.last, sizeof(long)) != sizeof(long))
         idxmem.last = 0;
      close(last);
      }

   return 0;

}
