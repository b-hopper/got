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

#define MAX_COMMITS 5

//Note: all these change comments and stuff should
//      be out by demo time in master branch at least.

// asyncDev
int open_fds(int (*rfds)[], int (*wfds)[], char* path);
// new functions (bradDev)
int got_reset(char *set_back);
void affirm_permit(DIR *directory);
void affirm_exist(char *buf);
void got_status(void);
// original functions (got-master)
unsigned int get_version(void);
void got_add(char* path);
void got_commit(void);
int got_init(void);
void write_cfg(char** filepaths, unsigned int version);
int read_line(int fd, char *line);
void copy_file(int src_file, int dest_file);

unsigned int version;

int gotcfg = -1;

char** staged_filepaths;

mode_t MODE = S_IRWXU | S_IRWXG | S_IRWXO;
