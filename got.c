#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <linux/limits.h>
#include <stdio.h>
#include <sys/stat.h>
#include <errno.h>


int copy_file(FILE * src_file, FILE * dest_file)
{
  int c;
  while ((c = fgetc(src_file)) != EOF)
  {
    fputc(c, dest_file);
  }
}

int main(int argc, char* argv[])
{
  FILE* fd;


  if (argc == 1) 
  {
    write(1, "Usage: \n   got init\n   more later...\n", 37);
    exit(0);
  }

  char cwd[PATH_MAX];
  getcwd(cwd, sizeof(cwd));

  char gotdir[PATH_MAX];

  strncpy(gotdir, cwd, PATH_MAX);
  strncat(gotdir, "/.gotdir", 9);
  if (strcmp(argv[1], "init") == 0)
  { // got init
    mode_t MODE = S_IRWXU | S_IRWXG | S_IRWXO;  // Full permissions for everybody
    int d = mkdir(gotdir, MODE);                // May need to change later?
    if (d == -1)
    {
      switch(errno)
      {
        case EEXIST: write(2, "Directory already exists\n", 25);
        default: break;
      }
    }
  }

  if (strcmp(argv[1], "test") == 0)
  {
    char destp[PATH_MAX];
    strncpy(destp, gotdir, PATH_MAX);
    strncat(destp, "/dest", 4);
    char srcp[PATH_MAX];
    strncpy(srcp, cwd, PATH_MAX);
    strncat(srcp, "/", 1);
    strncat(srcp, argv[2], strlen(argv[2]));
    FILE* src = fopen(srcp, "r");
    FILE* dest = fopen(destp, "w+");
    copy_file(src, dest);


  }
}
