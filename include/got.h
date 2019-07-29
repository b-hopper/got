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
int got_add(char* path);
int copy_file(FILE * src_file, FILE * dest_file);
int got_commit();

unsigned int version;

FILE * gotcfg;

char** staged_filepaths;

mode_t MODE = S_IRWXU | S_IRWXG | S_IRWXO;
