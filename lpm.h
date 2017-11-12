#ifndef LPM_HEADER
#define LPM_HEADER
#include <dirent.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <stdarg.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/file.h>
#include <sys/stat.h>

/**used for dlsym to find the injected funcs*/
#ifndef RTLD_NEXT
#define RTLD_NEXT ((void *) -1l)
#endif

/**Variables defination*/
#define TIME_BUF_SIZE 48 
#define STR_BUF_SIZE 1024
#define PATH_BUF_SIZE 4096
static char* lpm_file;
/**Define system functions macro*/
#define FUNC_DEC(RET,NAME,...) static RET (NAME) (__VA_ARGS__); 

#define FUNC_DLSYM(NAME) libc_ ## NAME = lpm_dlsym(#NAME);

/**COMMON FUNCS*/
FUNC_DEC (int,*libc_open,const char*,int,...)
FUNC_DEC (int,*libc_creat,const char*,mode_t)
FUNC_DEC (int,*libc_rename,const char*,const char*)
FUNC_DEC (int,*libc_link,const char*,const char*)
FUNC_DEC (int,*libc_symlink,const char*,const char*)
FUNC_DEC (FILE*,*libc_fopen,const char*,const char*)
FUNC_DEC (FILE*,*libc_freopen,const char*,const char*,FILE*)
FUNC_DEC (int,*libc_unlink,const char*)
FUNC_DEC (int,*libc_rmdir,const char*)
FUNC_DEC (int,*libc_mkdir,const char*,mode_t)

/**AT FUNCS*/
FUNC_DEC (int,*libc_openat,int,const char*,int,...)
FUNC_DEC (int,*libc_renameat,int,const char*,int,const char*)
FUNC_DEC (int,*libc_linkat,int,const char*,int,const char*,int)
FUNC_DEC (int,*libc_symlinkat,const char*,int,const char*)
FUNC_DEC (int,*libc_unlinkat,int,const char*,int)
FUNC_DEC (int,*libc_mkdirat,int,const char*,mode_t)

/**64bits FUNCS*/
FUNC_DEC (int,*libc_open64,const char*,int,...)
FUNC_DEC (int,*libc_creat64,const char*,mode_t)
FUNC_DEC (FILE*,*libc_fopen64,const char*,const char*)
FUNC_DEC (FILE*,*libc_freopen64,const char*,const char*,FILE*)

/**openat64*/
FUNC_DEC (int,*libc_openat64,int,const char*,int,...)

/**msg type enum*/
static char *MSG_CHAR_TYPE[4]={"DEBUG","INFO","WARNING","ERROR"};
enum MSG_TYPE{DEBUG,INFO,WARNING,ERROR};
enum LOCK_TYPE{LOCK,UNLOCK};
static char *OPER_CHAR_TYPE[21]={"OPEN","CREAT","RENAME","LINK","SYMLINK","FOPEN","FREOPEN","UNLINK","RMDIR","MKDIR","OPENAT","RENAMEAT","LINKAT","SYMLINKAT","UNLINKAT","MKDIRAT","OPEN64","CREAT64","FOPEN64","FREOPEN64","OPENAT64"};
enum OPER_TYPE{OPEN,CREAT,RENAME,LINK,SYMLINK,FOPEN,FREOPEN,UNLINK,RMDIR,MKDIR,OPENAT,RENAMEAT,LINKAT,SYMLINKAT,UNLINKAT,MKDIRAT,OPEN64,CREAT64,FOPEN64,FREOPEN64,OPENAT64};


/* Fake declarations of libc's internal __open and __open64 */
int __open(const char*, int, ...);
int __open64(const char*, int, ...);

#endif
