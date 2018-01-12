
#ifndef __TYPEDEFS_H_DEFINED
#define __TYPEDEFS_H_DEFINED

#if defined(__386__) || defined(__FLAT__) || defined(__GNUC__)
  typedef unsigned      bit;

  typedef unsigned char byte;
  typedef signed char   sbyte;

  typedef unsigned short word;
  typedef signed short   sword;

  typedef unsigned int  dword;
  typedef signed int    sdword;

#ifndef __GNUC__
  typedef unsigned short ushort;
#endif  
  typedef   signed short sshort;

#ifndef __GNUC__
  typedef unsigned long  ulong;
#endif  
  typedef   signed long  slong;
#else
  typedef unsigned      bit;

  typedef unsigned char byte;
  typedef signed char   sbyte;

  typedef unsigned int  word;
  typedef signed int    sword;

  typedef unsigned long dword;
  typedef signed long   sdword;

  typedef unsigned short ushort;
  typedef   signed short sshort;

  typedef unsigned long  ulong;
  typedef   signed long  slong;
#endif

#endif

