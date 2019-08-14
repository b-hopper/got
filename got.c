#include "include/got.h"

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
    got_error("\nUsage:\n\tgot init\n\tgot add <filename>\n\tgot commit\n\tgot status\n\tgot reset <version>\n");

  if (strcmp(argv[1], "init") == 0) {
    int init = got_init();
    if (init == 1)
      got_error("got already initialized! terminating...");
    else if (init == -1)
      got_error("failed to init: ");
  }

  if (gotcfg == -1)
    gotcfg = open(".gotdir/.gotconfig", O_RDWR);
  if (gotcfg == -1) // got is not set up in this project
    got_error("got not initialized! type \"got init\"\n");

  version = get_version();

  if (strcmp(argv[1], "add") == 0) {
    if (argc < 3)
      got_error("usage: \"got add <filename>\"\n");
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

  char *line = malloc(PATH_MAX * sizeof(char));
  lseek(gotcfg, 0, SEEK_SET);
  while(read_line(gotcfg, line) != -1) {
    strtok(line, "\n"); // chomp \n
    if (strcmp(line, path) == 0)
      got_error("file already staged for commit");
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
  int fcount;
  int readfds[MAX_COMMITS];
  int writefds[MAX_COMMITS];
  char vdir[13];

  sprintf(vdir, ".gotdir/%u/", version + 1);
  mkdir(vdir, MODE);
  
  char *fullpath = malloc(PATH_MAX * sizeof(char));
  strncpy(fullpath, vdir, PATH_MAX);
  write(STDOUT_FILENO, "Path: ", 6);
  write(STDOUT_FILENO, fullpath, strlen(fullpath));
  write(STDOUT_FILENO, "\n", 1);

  fcount = open_fds(readfds, writefds, fullpath);
  copy_files(readfds, writefds, fcount);

  close_fds(readfds, fcount);
  close_fds(writefds, fcount);

  version++;
  close(gotcfg);
  gotcfg = open(".gotdir/.gotconfig", O_RDWR | O_TRUNC);
  write_cfg(NULL, version);
  free(fullpath);
}

//Returns num of files to be read/written to new ver
// TODO: make this work with folders / able to create them
int open_fds(int readfds[], int writefds[], char *path)
{
  char* line = malloc(PATH_MAX * sizeof(char));
  char* fullpath = malloc(PATH_MAX * sizeof(char));
  int i = 0;

  lseek(gotcfg, 0, SEEK_CUR);
  read_line(gotcfg, line);

  while (read_line(gotcfg, line) != -1)
  {
    if (i > MAX_COMMITS)
      got_error("Maximum number of files to commit reached.\n");
    strtok(line, "\n"); // chomp \n
    strncpy(fullpath, path, PATH_MAX);
    strcat(fullpath, line);
    affirm_exist(line);

    readfds[i] = open(line, O_RDONLY);
    writefds[i] = open(fullpath, O_WRONLY | O_CREAT, 0600);
    ++i;    
  }
  free(line);
  free(fullpath);
  return i; 
}

//close all open fds in an array of fds
void close_fds(int fds[], int count)
{
  for (int i = 0; i < count; ++i)
    close(fds[i]);
}

//Sig handler for SIGUSR1, when sig recieved informs
//copy_files that file writing can happen.
//May be useful later on if anything ever wants to be done
//while files are still being read, before writing occurs.
void async_done(int signo)
{
  if (signo == SIGUSR1)
    ASYNC_DONE = 1;
}

//Using async w/ lio_listio using NOWAIT more because it is nice
//practice, less because it is very helpful in this situation.
void copy_files(int readfds[], int writefds[], int count)
{
  struct sigaction sa; // Seg fault here??
  struct sigevent se;
  struct aiocb * aio_op[count];  // Rather than using aiocb[], just use an iterable pointer

  //SIGUSR1 sent when reads complete
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sa.sa_handler = async_done;
  if (sigaction(SIGUSR1, &sa, NULL) < 0)
    got_error("Failed to establish signal handler.\n");
  
  //se establishes SIGUSR1 as the sig to send when lio_listio completes
  se.sigev_notify = SIGEV_SIGNAL;
  se.sigev_signo = SIGUSR1; 

  int sz = 0;
  //Setup aiocb array for reads
  for (int i = 0; i < count; ++i)
  {
    aio_op[i] = malloc(sizeof(struct aiocb));
    memset(aio_op[i], 0, sizeof(struct aiocb));
    aio_op[i]->aio_fildes = readfds[i];

    sz = lseek(readfds[i], 0, SEEK_END); // Get size of file
    lseek(readfds[i], 0, SEEK_SET); // Set offset back to start of file

    aio_op[i]->aio_buf = malloc(sz * sizeof(char));
    aio_op[i]->aio_nbytes = sz;
    aio_op[i]->aio_lio_opcode = LIO_READ;
  }

  
  //Carrys out all reads, in any order.
  if (lio_listio(LIO_NOWAIT, aio_op, count, &se) < 0)
    got_error("Error while queueing files for reading.\n");

  while (ASYNC_DONE < 0) 
    pause(); // Everytime a signal is caught, this will check
             // if writes are ready (SIGUSR1 was sent).
             // So, reads: "Pause until we can write."
  //Setup aiocb array for writes

  for (int i = 0; i < count; ++i)
  { // No need to reset buffer, we're not changing the aiocb so now 
    // it has read into the buffer and we're writing that same buffer to a new file
    aio_op[i]->aio_fildes = writefds[i]; 
    aio_op[i]->aio_lio_opcode = LIO_WRITE;
  }
  if (lio_listio(LIO_WAIT, aio_op, count, &se) != 0) // Seems to have some issues writing  
    got_error("Forgot to write files.\n");           // in LIO_NOWAIT, so wait on this one
  
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
  write(STDOUT_FILENO, "Files staged for commit:\n", 25);

  char *line = malloc(PATH_MAX * sizeof(char));
  lseek(gotcfg, 0, SEEK_CUR);
  read_line(gotcfg, line); // Skip version line

  while(read_line(gotcfg, line) != -1) {
    strtok(line, "\n"); // chomp \n
    write(STDOUT_FILENO, "   - ", 5);
    write(STDOUT_FILENO, line, strlen(line));
    write(STDOUT_FILENO, "\n", 1);
  }
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

  if (x < 0 || x > version)
    return -1;

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
      wr = open(cur->d_name, O_WRONLY | O_TRUNC);

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
