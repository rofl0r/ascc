#ifdef __ASCC__
/* we don't use import in ascc because that would cause farcalls
   to all functions, which require more code and aren't implemented
   in agssim. therefore we emit empty funcs that are later removed
   by make-executable.py */
#define DECL(...) __VA_ARGS__ {}
#else
#define DECL(...) extern __VA_ARGS__ ;
#endif

DECL(int puts(char*a))
DECL(long strlen(char*a))
DECL(char* strcpy(char*a,char*b))

