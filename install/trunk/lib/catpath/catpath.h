#ifndef CATPATH_H
#define CATPATH_H

#ifdef __cplusplus
extern "C" {
#endif

#define CP_MAXPATH 260
#define CP_MAXDIR 255

char * catpath(char *path, 
               const char *drive, 
               const char *dir,
               const char *fname,
               const char *ext);

void slicepath(const char *path, 
               char *drive, 
               char *dir,
               char *fname,
               char *ext);

#ifdef __cplusplus
}
#endif

#endif
