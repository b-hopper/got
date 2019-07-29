#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <linux/limits.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

unsigned int get_version();
int write_cfg(FILE * file, char** filepaths, unsigned int version);
int got_add(int cfg_fd, char* path);
int copy_file(FILE * src_file, FILE * dest_file);
int copy_fd(int src_fd, int dest_fd);

unsigned int version;

FILE * gotcfg;

char cwd[PATH_MAX];
char cfg[PATH_MAX];
char gotdir[PATH_MAX];

mode_t MODE = S_IRWXU | S_IRWXG | S_IRWXO;
