#include "got.h"

void got_error(char *errmsg)
{
  write(2, "got failed! error: ", 19);
  write(2, errmsg, strlen(errmsg));
  if (errno != ENOENT)
    write(2, strerror(errno), strlen(strerror(errno)));
  write(2, "\n", 1);
  exit(EXIT_FAILURE);
}

// ============================================

int main(int argc, char *argv[])
{
  errno = ENOENT;
  if (argc == 1)
    got_error("\nUsage:\n\tgot init\n\tmore later...\n");
    // the exit that was here didn't need to be here

  if (strcmp(argv[1], "init") == 0) {
    int init = got_init();
    if (init == 1)
      got_error("got already initialized! terminating...");
    else if (init == -1)
      got_error("failed to init: ");
  }

  if (gotcfg == -1)
    gotcfg = open(".gotdir/.gotconfig", O_RDWR);

  if (gotcfg == -1) // got is not set up in this projekt
    got_error("got not initialized! type \"got init\"\n");
    // the exit that was here didn't need to be here

  version = get_version();

  if (strcmp(argv[1], "add") == 0) {
    if (argc < 3)
      got_error("usage: \"got add <filename>\"\n");
      // the exit that was here didn't need to be here
    got_add(argv[2]);
  }

  if (strcmp(argv[1], "commit") == 0)
    got_commit();

  if (strcmp(argv[1], "status") == 0)
    got_status();

  if (strcmp(argv[1], "reset") == 0)
  {
    if (argc < 3)
      got_error("usage: \"got reset <desired version>\"\n");
    if (got_reset(argv[2]) == -1)
      got_error("Invalid version select");
  }

  close(gotcfg);
  exit(EXIT_SUCCESS);
}

// ============================================
// got_init()
// Arguments: none
// Returns:   1 if init already happened
//           -1 if failed to init
//            0 on success
// ============================================

int got_init(void)
{
  if (access(".gotdir/.gotconfig", F_OK) != -1)
    return 1;
  
  int d = mkdir(".gotdir", MODE);
  if (d == -1)
    return -1;

  gotcfg = open(".gotdir/.gotconfig", O_RDWR | O_CREAT, 0600);
  if (gotcfg != -1)
    write_cfg(NULL, 0);
  else
    return -1;

  return 0;
}

// ============================================
// write_cfg(unsigned int version)
// Arguments: filepaths (?)
//            version number (unsigned for
//            obvious reasons)
// Returns:   none
// ============================================

void write_cfg(char** filepaths, unsigned int version)
{
  char vbuf[4];
  sprintf(vbuf, "%u", version);
  write(gotcfg, vbuf, strlen(vbuf));
  write(gotcfg, "\n", 1);

  if (filepaths != NULL) {
    char *path = filepaths[0];

    while(path != NULL) {
      write(gotcfg, path, strlen(path));
      write(gotcfg, "\n", 1);
      path++;
    }
  }
}

// ============================================
// get_version()
// Arguments: none
// Returns:   unsigned int for version num
// ============================================

unsigned int get_version(void)
{
  char *line = malloc(1024*sizeof(char));
  lseek(gotcfg, 0, SEEK_SET);  // like rewind
  if (read_line(gotcfg, line) == -1)  // happens when file is empty
    got_error("get_version error: ");

  strtok(line, "\n"); // chomp \n with destructive strtok
  unsigned int r = atoi(line);
  free(line);
  
  return r;
}

// ============================================
// got_add(char* path)
// Arguments: path string to find file to add
// Returns:   none
// ============================================

void got_add(char* path)
{
  if (access(path, F_OK) < 0)
    got_error("cannot add file: ");
    // the exit that was here didn't need to be here

  char *line = malloc(PATH_MAX * sizeof(char));
  lseek(gotcfg, 0, SEEK_SET);
  while(read_line(gotcfg, line) != -1) {
    strtok(line, "\n"); // chomp \n
    if (strcmp(line, path) == 0)
      got_error("file already staged for commit");
      // the exit that was here didn't need to be here
  }

  write(gotcfg, path, strlen(path));
  write(gotcfg, "\n", 1);
  free(line);
}

// ============================================
// got_commit()
// Arguments: none
// Returns:   none
// ============================================

void got_commit(void)
{
  int count;
  char *line = malloc(PATH_MAX * sizeof(char));

  lseek(gotcfg, 0, SEEK_CUR);
  read_line(gotcfg, line);

  char vdir[13];
  sprintf(vdir, ".gotdir/%u/", version + 1);
  mkdir(vdir, MODE);

  char *fullpath = malloc(PATH_MAX * sizeof(char));


  while(read_line(gotcfg, line) != -1) {
    strtok(line, "\n"); // chomp \n
    strncpy(fullpath, vdir, PATH_MAX);
    strcat(fullpath, line);
    write(STDOUT_FILENO, "Path: ", 6);
    write(STDOUT_FILENO, fullpath, strlen(fullpath));
    write(STDOUT_FILENO, "\n", 1);

    if (access(line, F_OK) < 0) // file disappeared at some point between staging and committing?
      got_error("failed to commit file(s)");
      // the exit that was here didn't need to be here

    // TODO: create folder
    int oldfile = open(line, O_RDONLY);
    int newfile = open(fullpath, O_WRONLY | O_CREAT, 0600);
    copy_file(oldfile, newfile);
    count++;

    close(oldfile);
    close(newfile);
  }

  version++;
  close(gotcfg);
  gotcfg = open(".gotdir/.gotconfig", O_RDWR | O_TRUNC);
  write_cfg(NULL, version);
  free(fullpath);
  free(line);
}

// ============================================
// copy_file(int src_file, int dest_file)
// Arguments: the source file
//            the destination file
// Returns:   none
// ============================================

void copy_file(int src_file, int dest_file)
{
  char c;
  while(read(src_file, &c, 1) != 0) {
    write(dest_file, &c, 1);
  }
}

// ============================================
// read_line(int fd, char line[])
// Arguments: file to read from
//            line to save to
// Returns:   -1 on err
//            0 on success
// ============================================

int read_line(int fd, char *line)
{
  char c;
  int i = 0;
  while(c != '\n') {
    if (read(fd, &c, 1) == 0) {
      return -1;
    }
    line[i] = c;
    i++;
  }
  return 0;
}

// ============================================
// affirm_permit(DIR *directory)
// Arguments: directory to check
// Returns:   none
// ============================================

void affirm_permit(DIR *directory)
{
  if (directory == NULL)
    got_error("Invlaid permissions.");
}

// ============================================
// affirm_exists(char *buf)
// Arguments: file/directory name to check
// Returns:   none
// ============================================

void affirm_exist(char *buf)
{
  if (access(buf, F_OK) == -1)
    got_error("File/Directory does not exits.");
}

// ============================================
// got_status()
// Arguments: none
// Returns:   none
// ============================================

void got_status(void)
{
  char ver[4];
  write(STDOUT_FILENO, "Current version: ", 17);
  sprintf(ver, "%u", version);

  write(STDOUT_FILENO, ver, strlen(ver));
  write(STDOUT_FILENO, "\n", 1);
}

// ============================================
// got_reset(char *set_back)
// Arguments: version number to reset to
// Returns:  -1 on err
//            0 otherwise
// ============================================

int got_reset(char *set_back)
{
  // err handle
  if (set_back[0] < '0' || set_back[0] > '9')
    return -1;

  int x = atoi(set_back);
  char *file_path, *cur_file;
  DIR *directory;
  struct dirent *cur;
  int rd,wr;

  if (x < 0 || x == version)
    return -1;
  // load_version obsolete
  // ver_file will be set_back

  // get file path to that version's dir
  file_path = malloc(PATH_MAX * sizeof(char));
  memset(file_path, 0, PATH_MAX);
  strcat(file_path, ".gotdir/");
  strcat(file_path, set_back);

  // open version dir
  affirm_exist(file_path);
  directory = opendir(file_path);
  affirm_permit(directory);

  cur_file = malloc(PATH_MAX * sizeof(char));
  memset(cur_file, 0, PATH_MAX);

  cur = readdir(directory);
  while(cur != NULL) {
    if (cur->d_type == DT_REG) {
      strcpy(cur_file, file_path);
      strcat(cur_file, "/");
      strcat(cur_file, cur->d_name);

      rd = open(cur_file, O_RDONLY);
      wr = open(cur->d_name, O_WRONLY);

      copy_file(rd, wr);
      write(STDOUT_FILENO, "Updated ", 8);
      write(STDOUT_FILENO, cur->d_name, strlen(cur->d_name));
      write(STDOUT_FILENO, "\n", 1);

      close(rd);
      close(wr);
    }
    memset(cur_file, 0, PATH_MAX);
    cur = readdir(directory);
  }

  free(file_path);
  free(cur_file);
  return 0;
}
