#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <linux/limits.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <dirent.h>
#include <signal.h>
#include <aio.h>

#define MAX_COMMITS 5
#define BSZ 4096 

int open_fds(int rfds[], int wfds[], char* path);
void close_fds(int fds[], int count);
void async_done(int signo);
void copy_files(int rfds[], int wfds[], int count);

int got_reset(char *set_back);
void affirm_permit(DIR *directory);
void affirm_exist(char *buf);
void got_status(void);
unsigned int get_version(void);

void got_add(char* path);
void got_commit(void);
int got_init(void);
void write_cfg(char** filepaths, unsigned int version);
int read_line(int fd, char *line);
void copy_file(int src_file, int dest_file);

//Globals:
unsigned int version;
int gotcfg = -1;
char** staged_filepaths;
mode_t MODE = S_IRWXU | S_IRWXG | S_IRWXO;
int ASYNC_DONE = -1;
