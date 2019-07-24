#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <linux/limits.h>
#include <stdio.h>
#include <sys/stat.h>


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
  strncat(gotdir, "/", 1);
  strncat(gotdir, ".gotdir", 8);
  if (strcmp(argv[1], "init") == 0)
  { // got init
//    fd = fopen(
    mode_t MODE = S_IRWXU | S_IRWXG | S_IRWXO;  // Full permissions for everybody
    int d = mkdir(gotdir, MODE);                // May need to change later?
    
  }

  write(1, cwd, strlen(cwd));
  write(1, "\n", 1);
  write(1, gotdir, strlen(gotdir));
  write(1, "\n", 1);

}
