#ifndef DIR_HEADER
#define DIR_HEADER

#ifdef unix
#define DIR_CHAR '/'
#else
#define DIR_CHAR '\\'
#endif

int mkdir(const char *path);

#endif /* DIR_HEADER */
