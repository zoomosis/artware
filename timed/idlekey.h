#ifndef __IDLEKEY_H__
#define __IDLEKEY_H__

int get_idle_key(char allowstuff, int scope);
void stuffkey(int key);
void kbflush(void);
void check_enhanced(void);
int xkbhit(void);
void MacroStart(sword i);

#endif
