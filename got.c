#include "include/got.h"

int main(int argc, char* argv[])
{
  if (argc == 1) 
  {
    write(1, "Usage: \n   got init\n   more later...\n", 37);
    exit(0);
  }

  getcwd(cwd, sizeof(cwd));

  strncpy(gotdir, cwd, PATH_MAX);
  strncat(gotdir, "/", 1);
  strncat(gotdir, ".gotdir", 8);

  strncpy(cfg, gotdir, PATH_MAX);
  strncat(cfg, "/.gotconfig",11);


  if (strcmp(argv[1], "init") == 0)
  { // got init

    if (access(cfg, F_OK) != -1)
    {
      write(2, "got already initialized! terminating...\n", 40);
//      exit(-1);
    }

    int d = mkdir(gotdir, MODE); 

    gotcfg = open(cfg, (O_CREAT | O_WRONLY), MODE);
    if (gotcfg != -1) write_cfg_new(gotcfg, NULL, 0);
    close(gotcfg);
    
  }

  gotcfg = open(cfg, (O_APPEND), MODE);
  gotfile = fdopen(gotcfg, "r+");

  

  close(gotcfg);
}

int write_cfg_new(int fd, char** filepaths, unsigned int version)
{
  char vbuf[4];
  sprintf(vbuf, "%u", version);
  write(fd, "version: ", 9);
  write(fd, vbuf, sizeof(vbuf));
  write(fd, "\n", 1);
  write(1, "yes\n", 4);

  if (filepaths != NULL)
  {
    char* path = filepaths[0];

    while (path != NULL)
    {
      write(fd, path, sizeof(path));
      write(fd, "\n", 1);
      path++;
    }
  }

}

int copy_file(FILE * src_file, FILE * dest_file)
{
  int c;
  while ((c = fgetc(src_file)) != EOF)
    fputc(c, dest_file);
}

int copy_fd(int src_fd, int dest_fd)
{
  // TODO
}
