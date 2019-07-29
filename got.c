#include "include/got.h"

void got_error(char* errmsg)
{
  write(2, "got failed! error: ", 19);
  write(2, errmsg, strlen(errmsg));
  if (errno != ENOENT)
    write(2, strerror(errno), 32);
  write(2, "\n", 1);
  exit(-1);
}

int main(int argc, char* argv[])
{
  errno = ENOENT;
  if (argc == 1) 
  {
    got_error("\nUsage: \n   got init\n   more later...\n");
    exit(0);
  }

  if (strcmp(argv[1], "init") == 0)
  { // got init

    if (access(".gotdir/.gotconfig", F_OK) != -1)
    {
      got_error("got already initialized! terminating...");
    }

    int d = mkdir(".gotdir", MODE); 

    gotcfg = fopen(".gotdir/.gotconfig", "w+");
    if (gotcfg != NULL) write_cfg(gotcfg, NULL, 0);
    else got_error("failed to init: ");
  }

  if (gotcfg == NULL) gotcfg = fopen(".gotdir/.gotconfig", "r+");
  if (gotcfg == NULL) // got is not set up in this project!
  {
    got_error("got not initialized! type \"got init\"\n");
    exit(-1);
  }
  version = get_version();

  if (strcmp(argv[1], "add") == 0)
  { // got add
    if (argv[2] == NULL)
    {
      got_error("usage: \"got add <filename>\"\n");
      exit(-1);
    }
    got_add(argv[2]);
  }

  if (strcmp(argv[1], "commit") == 0)
  { // got commit
    got_commit();
  }

  fclose(gotcfg);
}

int write_cfg(FILE * file, char** filepaths, unsigned int version)
{
  freopen(NULL, "w+", gotcfg); // We've already read the version number, 
  char vbuf[4];                // this will recreate the config file from scratch
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
  if (fgets(line, PATH_MAX, gotcfg) == NULL)
  {
    got_error("get_version error: ");
  }
  strtok(line, "\n"); // Chomp \n with destructive strtok
  unsigned int r = atoi(line);
  free(line);
  return r;

}

int got_add(char* path)
{
  if (access(path, F_OK) < 0){
    got_error("cannot add file: ");
    exit(-1);
  }
  char* line = malloc(PATH_MAX * sizeof(char));
  rewind(gotcfg);
  while (fgets(line, PATH_MAX, gotcfg) != NULL)
  {
    strtok(line, "\n"); // Chomp \n
    if (strcmp(line, path) == 0)
    {
      got_error("file already staged for commit");
      exit(-1);
    }
  }
  fputs(path, gotcfg);
  fputc('\n', gotcfg);
  free(line);
}

int got_commit()
{
  int count;
  char* line = malloc(PATH_MAX * sizeof(char));
  
  rewind(gotcfg);
  fgets(line, PATH_MAX, gotcfg);  // Skip version number (which could be >1 digit, so just fgets to skip to \n)

  char vdir[13];
  sprintf(vdir, ".gotdir/%u/", version+1);
  mkdir(vdir, MODE);

  char* fullpath = malloc(PATH_MAX * sizeof(char));


  while (fgets(line, PATH_MAX, gotcfg) != NULL)
  {
    strtok(line, "\n"); // Chomp \n
    strncpy(fullpath, vdir, PATH_MAX);
    strcat(fullpath, line);
    if (access(line, F_OK) < 0) // File disappeared at some point between staging and committing?
    {
      got_error("failed to commit file(s)");
      exit(-1);
    }
    FILE * oldfile = fopen(line, "r");
    FILE * newfile = fopen(fullpath, "w");
    copy_file(oldfile, newfile);
    count++;
    fclose(oldfile);
    fclose(newfile);
  }
  version++;
  write_cfg(gotcfg, NULL, version);
  free(fullpath);
  free(line);
}
