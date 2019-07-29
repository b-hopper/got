#include "include/got.h"

int main(int argc, char* argv[])
{
  if (argc == 1) 
  {
    write(1, "Usage: \n   got init\n   more later...\n", 37);
    exit(0);
  }

  if (strcmp(argv[1], "init") == 0)
  { // got init

    if (access(".gotdir/.gotconfig", F_OK) != -1)
    {
      write(2, "got already initialized! terminating...\n", 40);
//      exit(-1);
    }

    int d = mkdir(".gotdir", MODE); 

    gotcfg = fopen(".gotdir/.gotconfig", "w+");
    if (gotcfg != NULL) write_cfg(gotcfg, NULL, 0);
    else { 
      write(2, "failed to init: ", 16);
      write(2, strerror(errno), 26);
      write(2, "\n", 1);
      exit(-1);
     } 
  }

  if (gotcfg == NULL) gotcfg = fopen(".gotdir/.gotconfig", "r+");
  if (gotcfg == NULL) // got is not set up in this project!
  {
    write(2, "got not initialized! type \"got init\"\n", 38);
    exit(-1);
  }
  version = get_version();

  if (strcmp(argv[1], "add") == 0)
  { // got add
    
  }

  fclose(gotcfg);
}

int write_cfg(FILE * file, char** filepaths, unsigned int version)
{
  rewind(file);
  char vbuf[4];
  sprintf(vbuf, "%u", version);
  fputs(vbuf, file);
  fputc('\n', file);

  if (filepaths != NULL)
  {
    char* path = filepaths[0];

    while (path != NULL)
    {
      fputs(path, file);
      fputc('\n', file);
      path++;
    }
  }
  return 1;
}

int copy_file(FILE * src_file, FILE * dest_file)
{
  int c;
  while ((c = fgetc(src_file)) != EOF)
    fputc(c, dest_file);
  return 1;
}

unsigned int get_version()
{
  char* line = malloc(1024*sizeof(char));
  rewind(gotcfg);
  char buf[1024];
  if (fgets(line, PATH_MAX, gotcfg) == NULL)
  {
    strerror_r(errno, buf, 1024);
    write(2, "Error: ", 7);
    write(2, buf, 1024);
    write(2, "\n", 1);
  }
  strtok(line, "\n");
  write(1, "line: [", 7);
  write(1, line, sizeof(line));
  write(1, "]\n", 2);
  unsigned int r = atoi(line);
  free(line);
  return r;

}
