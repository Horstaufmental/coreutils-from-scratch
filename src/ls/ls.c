#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <unistd.h>

#define true 1
#define false 0

/*
`-l option`
Selects the long output format which extends the default output of the file name
with additional information including type, permissions, hard link count,
owning user and group, size, last-modified timestamp

Example:
-r-xr-xr-x   1 fjones bookkeepers 8460 Jan 16  2022  edit.sh
drwxr--r--   1 fjones editors     4096 Mar  2 12:52  drafts

┌─────────── file (not a directory)
|┌─────────── read-write (no execution) permissions for the owner
|│  ┌───────── read-only permissions for the group
|│  │  ┌─────── read-only permissions for others
|│  │  │     ┌── 3 hard links
|│  │  │     │   ┌── owning user
|│  │  │     │   │     ┌── owning group
|│  │  │     │   │     │          ┌── file size in bytes
|│  │  │     │   │     │          │    ┌── last modified on
|│  │  │     │   │     │          │    │                ┌── filename
-rw-r--r--   3 fjones editors    30405 Mar  2 12:52  edition-32

File Types:
"-" for regular file
"d" for directory
"l" for symbolic link
"n" for network file
"s" for socket
"p" for named pipe (FIFO)
"c" for character special file
"b" for block special file

`-h option`
Output sizes as so-called human readable by using units of KB, MB, GB instead of
bytes.
*/
char longOutput(char file) {
  struct stat file_stat;
  struct statfs fs_info;

  /*
  0 = regular
  1 = directory
  2 = symbolic link
  3 = network file
  4 = socket
  5 = named pipe
  6 = character special file
  7 = block special file
  */
  int fileType;

  // TODO: check for network file
  if (stat(&file, &file_stat) == 0) {
    if (S_ISREG(file_stat.st_mode)) {
      fileType = 0;
    } else if (S_ISDIR(file_stat.st_mode)) {
      fileType = 1;
    } else if (S_ISLNK(file_stat.st_mode)) {
      fileType = 2;
    } else if (S_ISSOCK(file_stat.st_mode)) {
      fileType = 4;
    } else if (S_ISFIFO(file_stat.st_mode)) {
      fileType = 5;
    } else if (S_ISCHR(file_stat.st_mode)) {
      fileType = 6;
    } else if (S_ISBLK(file_stat.st_mode)) {
      fileType = 7;
    }
  }

  char typeI[2];
  switch (fileType) {
    case 1:
      strncpy(typeI, "-", sizeof(typeI) - 1);
      typeI[sizeof(typeI) - 1] = '\0';
  }
}

int main(int argc, char *argv[]) {
  DIR *d;
  struct dirent *dir;
  int opt;

  int includeALL = false;
  int includeALLshort = false;
  
  while ((opt = getopt(argc, argv, "aA")) != -1) {
    switch (opt) {
    case 'a':
      includeALL = true;
      break;
    case 'A':
      includeALLshort = true;
      break;
    }
  }

  if (argc - optind == 0) {
    d = opendir(".");
  } else if (argc - optind == 1) {
    d = opendir(argv[optind]);
  } else {
    fprintf(stderr, "Error: please provide only 1 input.\n");
  }

  if (!d) {
    perror("opendir");
    return 1;
  }
  
  while ((dir = readdir(d)) != NULL) {
    // note to self: continue means to skip over the current item
    if ((strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0) && includeALL == false) {
      // printf("\nskipped over: '%s', bool: %d\n", dir->d_name, includeALL);
      continue;
    }
    if (dir->d_name[0] == '.' && (includeALL == false && includeALLshort == false)) {
      // printf("\nskipped over: '%s', -a bool: %d, -A bool: %d\n", dir->d_name, includeALL, includeALLshort);
      continue;
    }
    printf("%s  ", dir->d_name);
  }
  printf("\n");
  closedir(d);
  return (0);
}
