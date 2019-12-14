#ifndef FUNCDECLS_H
#define FUNCDECLS_H

#ifdef __ASCC__
/* we don't use import in ascc because that would cause farcalls
   to all functions, which require more code and aren't implemented
   in agssim. therefore we emit empty funcs that are later removed
   by make-executable.py */
#define DECL(...) __VA_ARGS__ {}
#define ARGV int*
#define STATIC
#else
#define DECL(...) extern __VA_ARGS__ ;
#define ARGV char**
#define STATIC static
extern int printf(char*, ...);
void print(char*a) { printf("%s", a); }
#endif

DECL(void print(char*a))
DECL(int puts(char*a))
DECL(long strlen(char*a))
DECL(char* strcpy(char*a,char*b))

#endif

