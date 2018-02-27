/*
**  xfile.h -- definitions for fast line buffered files
*/

#ifndef __XFILE_H__
#define __XFILE_H__

XFILE *xopen(char const *);
void xclose(XFILE *);
char *xgetline(XFILE *);

#endif
