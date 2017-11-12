/****************************************************************************************
 * lpm.c monitors the system calls of file/folder operations and logs into disk file
 *
 * This file is part of LPM project
 * Copyright (C) 2017, Kasahara Lab
 * For more information on LPM project, please visit https://lpm.bio
 * This project is shared and hosted on github, https://jasonyangshadow.github.io/LPM_LIB/
 *
****************************************************************************************/
#include "lpm.h"

static void lpm_strcat(char* result,enum MSG_TYPE type, const char* func_name,const char* fmt, va_list args){
    char buffer[TIME_BUF_SIZE];
    time_t log_time;
    time(&log_time);
    struct tm* tm_info=localtime(&log_time);
    strftime(buffer,TIME_BUF_SIZE,"{\"TIME\":\"%Y-%m-%d %H:%M:%S\",",tm_info);
    //append time part
    strcat(result,buffer);
   
    //append msg type part
    strcat(result,"\"TYPE\":\"");
    strcat(result,MSG_CHAR_TYPE[type]);

    //append the func_name part
    strcat(result,"\",\"FUNC_NAME\":\"");
    strcat(result,func_name);

    //append msg content part
    strcat(result,"\",\"MSG\":\"");
    vsprintf(result+strlen(result),fmt,args);

    //append the tail part
    strcat(result,"\"}\n");
}

static void lpm_strcat_file(char* result,enum OPER_TYPE type,const char* fmt, char* path, va_list args){
    char buffer[TIME_BUF_SIZE];
    time_t log_time;
    time(&log_time);
    struct tm* tm_info=localtime(&log_time);
    strftime(buffer,TIME_BUF_SIZE,"{\"TIME\":\"%Y-%m-%d %H:%M:%S\",",tm_info);
    //append time part
    strcat(result,buffer);
   
     //append msg type part
    strcat(result,"\"TYPE\":\"");
    strcat(result,OPER_CHAR_TYPE[type]);

   //append path
    strcat(result,"\",\"PATH\":\"");
    strcat(result,path);

    //append msg content part
    strcat(result,"\",\"MSG\":\"");
    vsprintf(result+strlen(result),fmt,args);
    //append the tail part
    strcat(result,"\"}\n");
}


static void lpm_log(FILE *stream,enum MSG_TYPE type,const char* func_name,const char* fmt,...){
    fflush(stream);
    char result[STR_BUF_SIZE];
    va_list args;
    va_start(args,fmt);
    lpm_strcat(result,type,func_name,fmt,args);
    va_end(args);
    fflush(stream);
}



static void lpm_abs_path(int fd, const char* path, char* abs_path)
{
	char cwd[PATH_BUF_SIZE], aux[PATH_BUF_SIZE];
	int old_errno = errno;

	/* already absolute (or can't get CWD) */
	if (path[0] == '/' || !getcwd(cwd, PATH_BUF_SIZE)){
		strncpy(abs_path, path, PATH_BUF_SIZE - 1);
    }
	else if (fd < 0) {
		strncpy(abs_path, cwd, PATH_BUF_SIZE - 1);
		strncat(abs_path, "/", PATH_BUF_SIZE - strlen(abs_path) - 1);
		strncat(abs_path, path, PATH_BUF_SIZE - strlen(abs_path) - 1);
	}
	else if (fchdir(fd) == 0 && getcwd(aux, PATH_BUF_SIZE) && chdir(cwd) == 0) {
		strncpy(abs_path, aux, PATH_BUF_SIZE - 1);
		strncat(abs_path, "/", PATH_BUF_SIZE - strlen(abs_path) - 1);
		strncat(abs_path, path, PATH_BUF_SIZE - strlen(abs_path) - 1);
	}
	else{
		strncpy(abs_path, path, PATH_BUF_SIZE - 1);
    }
	abs_path[PATH_BUF_SIZE - 1] = 0;
	errno = old_errno;
}


static void* lpm_dlsym(const char *symbol){
    void* ret;
    if(!(ret=dlsym(RTLD_NEXT,symbol))){
        lpm_log(stderr,ERROR,"lpm_dlsym","%s dlsym error %s", symbol,(char *)dlerror());
        exit(EXIT_FAILURE);
    }
    return ret;
}

static void lpm_init(){
    if(lpm_file){
        return;
    }

    lpm_file = getenv("LPM_OUTPUT_FILE");
    if(!lpm_file || strncmp(lpm_file,"/tmp/",5)!=0){
        lpm_log(stderr,ERROR,"lpm_init","%s","LPM_OUTPUT_FILE does not exist or LPM_OUTPUT_FILE should be set under /tmp/ folder");
        exit(EXIT_FAILURE);
    }

    //define the system hooks
    FUNC_DLSYM (open) 
    FUNC_DLSYM (creat) 
    FUNC_DLSYM (rename) 
    FUNC_DLSYM (link) 
    FUNC_DLSYM (symlink) 
    FUNC_DLSYM (fopen) 
    FUNC_DLSYM (freopen) 
    FUNC_DLSYM (unlink) 
    FUNC_DLSYM (rmdir) 
    FUNC_DLSYM (mkdir) 

    FUNC_DLSYM (open64) 
    FUNC_DLSYM (creat64) 
    FUNC_DLSYM (fopen64) 
    FUNC_DLSYM (freopen64) 

    FUNC_DLSYM (openat) 
    FUNC_DLSYM (renameat) 
    FUNC_DLSYM (linkat) 
    FUNC_DLSYM (symlinkat) 
    FUNC_DLSYM (unlinkat) 
    FUNC_DLSYM (mkdirat) 

    FUNC_DLSYM (openat64)
}

static void lpm_lock(int fd,enum LOCK_TYPE type){
    int old_errno=errno;
    if(fd < 0){
        lpm_log(stderr,ERROR,"lpm_lock","%s","file hander is invalid");
        exit(EXIT_FAILURE);
    }
    int ret=(type==LOCK)?flock(fd,LOCK_EX):flock(fd,LOCK_UN);
    if(ret==-1){
        switch(errno){
            case EBADF:
                lpm_log(stderr,ERROR,"lpm_lock","%s","flock failed with EBADF error");
                break;
            case EINTR:
                lpm_log(stderr,ERROR,"lpm_lock","%s","flock failed with EINTR error");
                break;
            case EINVAL:
                lpm_log(stderr,ERROR,"lpm_lock","%s","flock failed with EINVAL error");
                break;
            case ENOLCK:
                lpm_log(stderr,ERROR,"lpm_lock","%s","flock failed with ENOLCK error");
                break;
            case EWOULDBLOCK:
                lpm_log(stderr,ERROR,"lpm_lock","%s","flock failed with EWOULDBLOCK error");
                break;
        }
        exit(EXIT_FAILURE);
    }
    errno=old_errno;
}

static void lpm_log_file(const char* path,enum OPER_TYPE type,const char* fmt,...){
    char abs_path[PATH_BUF_SIZE];
    char result[STR_BUF_SIZE]="";
    int fd,old_errno=errno;
    va_list args;
    va_start(args,fmt);
    
    lpm_init();

    if((fd = libc_open(lpm_file, O_WRONLY | O_CREAT | O_APPEND, 0644))<0){
        lpm_log(stderr,ERROR,"lpm_log_file","open %s with error: %s ",lpm_file,strerror(errno));
        exit(EXIT_FAILURE);
    }
    //lock file
    lpm_lock(fd,LOCK);

    lpm_abs_path(-1,path,abs_path);
    lpm_strcat_file(result,type,fmt,abs_path,args);
    va_end(args);

    if(write(fd,result,strlen(result))!= strlen(result)){
        lpm_log(stderr,ERROR,"lpm_log_file","write %s failed",lpm_file);
        exit(EXIT_FAILURE);
    }

    lpm_lock(fd,UNLOCK);

    if(close(fd)<0){
        lpm_log(stderr,ERROR,"lpm_log_file","close %s failed",lpm_file);
        exit(EXIT_FAILURE);
    }
    
    errno=old_errno;
}

static void lpm_log_rename(const char* oldpath, const char* newpath)
{
	char oldbuf[PATH_BUF_SIZE], newbuf[PATH_BUF_SIZE];
	struct stat st;
	DIR* dir;
	struct dirent* e;
	size_t oldlen, newlen;
	int old_errno = errno;

	/* The newpath file doesn't exist */
	if (lstat(newpath, &st) < 0) 
		goto goto_end;

	else if (!S_ISDIR(st.st_mode)) {
		lpm_log_file(newpath,RENAME, "%s <-- %s", oldpath);
		goto goto_end;
	}

	/* Make sure we have enough space for the following slashes */
	oldlen = strlen(oldpath);
	newlen = strlen(newpath);
	if (oldlen + 3 > PATH_BUF_SIZE || newlen + 3 > PATH_BUF_SIZE)
		goto goto_end;

	strcpy(oldbuf, oldpath);
	strcpy(newbuf, newpath);
	oldbuf[oldlen] = newbuf[newlen] = '/';
	oldbuf[++oldlen] = newbuf[++newlen] = 0;

	if (!(dir = opendir(newbuf)))
		goto goto_end;

	while ((e = readdir(dir))) {
		if (!strcmp(e->d_name, ".") || !strcmp(e->d_name, ".."))
			continue;
		strncat(oldbuf, e->d_name, PATH_BUF_SIZE- oldlen - 1);
		strncat(newbuf, e->d_name, PATH_BUF_SIZE- newlen - 1);
		lpm_log_rename(oldbuf, newbuf);
		oldbuf[oldlen] = newbuf[newlen] = 0;
	}

	closedir(dir);

goto_end: 
	errno = old_errno;
}



/**System Hooks implementation*/
int open(const char* path, int flags, ...)
{
	va_list a;
	int mode, accmode, ret;

	/* this fixes a bug when the installer program calls jemalloc 
	   (thanks Masahiro Kasahara) */
	if (lpm_file && !strncmp(path, "/proc/", 6))
		return __open(path, flags);

	lpm_init();
	
	va_start(a, flags);
	mode = va_arg(a, int);
	va_end(a);
	
	if ((ret = libc_open(path, flags, mode)) != -1) {
		accmode = flags & O_ACCMODE;
		if (accmode == O_WRONLY || accmode == O_RDWR)
			lpm_log_file(path,OPEN,"open %s",path);
	}

	return ret;
}


int creat(const char* path, mode_t mode)
{
	int ret;
	
	lpm_init();
	
	if ((ret = libc_creat(path, mode)) != -1)
		lpm_log_file(path,CREAT, "create %s,mode:0%o",path,(int)mode);
	
	return ret;
}


int rename(const char* oldpath, const char* newpath)
{
	int ret;
	
	lpm_init();
	
	if ((ret = libc_rename(oldpath, newpath)) != -1)
		lpm_log_rename(oldpath, newpath);

	return ret;
}


int link(const char* oldpath, const char* newpath)
{
	int ret;
	
	lpm_init();
	
	if ((ret = libc_link(oldpath, newpath)) != -1)
		lpm_log_file(newpath,LINK, "link %s <--- %s",newpath, oldpath);
	
	return ret;
}


int symlink(const char* oldpath, const char* newpath)
{
	int ret;
	
	lpm_init();
	
	if ((ret = libc_symlink(oldpath, newpath)) != -1)
		lpm_log_file(newpath,SYMLINK, "symlink %s <--- %s",newpath, oldpath);
	
	return ret;
}


FILE* fopen(const char* path, const char* mode)
{
	FILE* ret;
	
	lpm_init();
	
	if ((ret = libc_fopen(path, mode)) && strpbrk(mode, "wa+"))
		lpm_log_file(path,FOPEN, "fopen %s,mode:%s",path,mode);
	
	return ret;
}


FILE* freopen(const char* path, const char* mode, FILE* stream)
{
	FILE* ret;
	
	lpm_init();
	
	if ((ret = libc_freopen(path, mode, stream)) && strpbrk(mode, "wa+"))
		lpm_log_file(path,FREOPEN, "freopen %s,mode:%s",path,mode);
	
	return ret;
}

int unlink(const char* path){
    int ret;
    
    lpm_init();

    if((ret = libc_unlink(path))!=-1){
	    lpm_log_file(path,UNLINK, "unlink %s",path);
    }
    return ret;
}

int rmdir(const char* path){
    int ret;
    
    lpm_init();

    if((ret = libc_rmdir(path))!=-1){
	    lpm_log_file(path,RMDIR, "rmdir %s",path);
    }
    return ret;

}

int mkdir(const char* path, mode_t mode){
    int ret;

    lpm_init();

    if((ret = libc_mkdir(path,mode))!=-1){
        fprintf(stderr,"%s\n","test");
        lpm_log_file(path,MKDIR,"mkdir %s,mode:%s",path,mode);
    }
    return ret;
}

int open64(const char* path, int flags, ...)
{
	va_list a;
	int mode, accmode, ret;
	
	if (!lpm_file && path && !strncmp(path, "/proc/", 6))
		return __open64(path, flags);

	lpm_init();
	
	va_start(a, flags);
	mode = va_arg(a, int);
	va_end(a);
	
	if ((ret = libc_open64(path, flags, mode)) != -1) {
		accmode = flags & O_ACCMODE;
		if (accmode == O_WRONLY || accmode == O_RDWR)
			lpm_log_file(path,OPEN64, "open %s",path);
	}

	return ret;
}


int creat64(const char* path, mode_t mode)
{
	int ret;
	
	lpm_init();
	
	if ((ret = libc_creat64(path, mode)) != -1)
		lpm_log_file(path,CREAT64, "create %s,mode:0%o",path,mode);
	
	return ret;
}


FILE* fopen64(const char* path, const char* mode)
{
	FILE* ret;
	
	lpm_init();
	
	ret = libc_fopen64(path, mode);
	if (ret && strpbrk(mode, "wa+"))
		lpm_log_file(path,FOPEN64, "fopen %s,mode:%s",path,mode);
	
	return ret;
}


FILE* freopen64(const char* path, const char* mode, FILE* stream)
{
	FILE* ret;
	
	lpm_init();
	
	ret = libc_freopen64(path, mode, stream);
	if (ret && strpbrk(mode, "wa+"))
		lpm_log_file(path,FREOPEN64, "freopen %s, mode:%s",path,mode);
	
	return ret;
}

int openat(int fd, const char* path, int flags, ...)
{
	va_list a;
	int mode, accmode, ret;
	char abs_path[PATH_BUF_SIZE];

	lpm_init();
	
	va_start(a, flags);
	mode = va_arg(a, int);
	va_end(a);
	
	if ((ret = libc_openat(fd, path, flags, mode)) != -1) {
		accmode = flags & O_ACCMODE;
		if (accmode == O_WRONLY || accmode == O_RDWR) {
			lpm_abs_path(fd, path, abs_path);
			lpm_log_file(abs_path,OPENAT, "open %s,fd:%d", abs_path,fd);
		}
	}

	return ret;
}


int renameat(int oldfd, const char* oldpath, int newfd, const char* newpath)
{
	int ret;
	char old_abs_path[PATH_BUF_SIZE];
	char new_abs_path[PATH_BUF_SIZE];
	
	lpm_init();

	if ((ret = libc_renameat(oldfd, oldpath, newfd, newpath)) != -1) {
		lpm_abs_path(oldfd, oldpath, old_abs_path);
		lpm_abs_path(newfd, newpath, new_abs_path);
		lpm_log_rename(old_abs_path, new_abs_path);
	}

	return ret;
}


int linkat(int oldfd, const char* oldpath, 
           int newfd, const char* newpath, int flags)
{
	int ret;
	char new_abs_path[PATH_BUF_SIZE];
	
	lpm_init();

	if ((ret = libc_linkat(oldfd, oldpath, newfd, newpath, flags)) != -1) {
		lpm_abs_path(newfd, newpath, new_abs_path);
		lpm_log_file(new_abs_path,LINKAT, "link %s <-- %s,fd: %d <- %d", new_abs_path,
			oldpath,newfd,oldfd);
	}

	return ret;
}


int symlinkat(const char* oldpath, int newfd, const char* newpath)
{
	int ret;
	char new_abs_path[PATH_BUF_SIZE];
	
	lpm_init();
	
	if ((ret = libc_symlinkat(oldpath, newfd, newpath)) != -1) {
		lpm_abs_path(newfd, newpath, new_abs_path);
		lpm_log_file(new_abs_path,SYMLINKAT, "symlink %s <- %s,fd:%d",new_abs_path,oldpath,newfd);
	}

	return ret;
}

int unlinkat(int fd, const char* path, int flags){
    char abs_path[PATH_BUF_SIZE];
    int ret;

    lpm_init();

    if((ret = libc_unlinkat(fd,path,flags))!=-1){
        lpm_abs_path(fd,path,abs_path);
        lpm_log_file(abs_path,UNLINKAT,"unlink %s,fd: %d",abs_path,fd);
    }
    return ret;
}

int mkdirat(int fd,const char* path,mode_t mode){
    char abs_path[PATH_BUF_SIZE];
    int ret;

    lpm_init();

    if((ret = libc_mkdirat(fd,path,mode))!=-1){
        lpm_abs_path(fd,path,abs_path);
        lpm_log_file(abs_path,MKDIRAT,"mkdir %s,fd:%d",abs_path,fd);
    }

    return ret;
}

int openat64(int fd, const char* path, int flags, ...)
{
	va_list a;
	int mode, accmode, ret;
	char abs_path[PATH_BUF_SIZE];

	lpm_init();
	
	va_start(a, flags);
	mode = va_arg(a, int);
	va_end(a);
	
	if ((ret = libc_openat64(fd, path, flags, mode)) != -1) {
		accmode = flags & O_ACCMODE;
		if (accmode == O_WRONLY || accmode == O_RDWR) {
			lpm_abs_path(fd, path, abs_path);
			lpm_log_file(abs_path,OPENAT64, "open %s,fd:%d",abs_path,fd);
		}
	}

	return ret;
}
