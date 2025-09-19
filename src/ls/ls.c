#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/types.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>

#define true 1
#define false 0

int includeALL = false;
int includeALLshort = false;
int longFormat = false;
int humanReadable = false;
struct help_entry
{
  const char *opt;
  const char *desc;
};

static struct option long_options[] = {
  {"all", no_argument, 0, 'a'},
  {"almost-all", no_argument, 0, 'A'},
  {"help", no_argument, 0, 1},
  {0, no_argument, 0, 'l'},
  {0, 0, 0, 0}
};

static struct help_entry help_entries[] = {
  {"-a, --all", "show hidden and 'dot' files. Use this twice to also\n"
  "              show the '.' and '..' directories"},
  {"-A, --almost-all", "equivalent to --all; included for compatibility with `ls -A"},
  {"-h, --human-readable", "with -l, print sizes in human readable format (e.g., 1K 234M 2G)"},
  {"-l", "display extended file metadata as a table"},
  {"--help", "display this help and exit"}};

void print_help(const char *name)
{
  printf("Usage: %s [OPTION]... DIRECTORY...\n", name);
  printf("Create the DIRECTORY(ies), if they do not already exist.\n\n");
  printf("Mandatory arguments to long options are mandatory for short options too.\n");

  // find longest option string
  int maxlen = 0;
  for (int i = 0; help_entries[i].opt; i++)
  {
    int len = (int)strlen(help_entries[i].opt);
    if (len > maxlen)
      maxlen = len;
  }

  // print each option aligned
  for (int i = 0; help_entries[i].opt; i++)
  {
    printf("  %-*s  %s\n", maxlen, help_entries[i].opt, help_entries[i].desc);
  }
}

void byteToHR(long long bytes, char* buffer, int bufferSize) {
  const char* units[] = {"B", "K", "M", "G", "T", "P", "E"};
  long unsigned int uIndx = 0;
  double value = (double)bytes;

  while (value >= 1024 && uIndx < sizeof(units) / sizeof(units[0]) - 1) {
    value /= 1024;
    uIndx++;
  }
  snprintf(buffer, bufferSize, "%.1f%s", value, units[uIndx]);
}

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
void printlongOutput(char file[]) {
  struct stat file_stat;

  char fileType;

  // TODO: check for network file
  if (stat(file, &file_stat) == 0) {
    if (S_ISREG(file_stat.st_mode)) {
      fileType = '-';
    } else if (S_ISDIR(file_stat.st_mode)) {
      fileType = 'd';
    } else if (S_ISLNK(file_stat.st_mode)) {
      fileType = 'l';
    } else if (S_ISSOCK(file_stat.st_mode)) {
      fileType = 's';
    } else if (S_ISFIFO(file_stat.st_mode)) {
      fileType = 'p';
    } else if (S_ISCHR(file_stat.st_mode)) {
      fileType = 'c';
    } else if (S_ISBLK(file_stat.st_mode)) {
      fileType = 'b';
    }
  }
  //printf("filetype %c\n", fileType);
  // permissions
  char permissions[10];
  permissions[0] = (file_stat.st_mode & S_IRUSR) ? 'r' : '-';
  permissions[1] = (file_stat.st_mode & S_IWUSR) ? 'w' : '-';
  permissions[2] = (file_stat.st_mode & S_IXUSR) ? 'x' : '-';
  permissions[3] = (file_stat.st_mode & S_IRGRP) ? 'r' : '-';
  permissions[4] = (file_stat.st_mode & S_IWGRP) ? 'w' : '-';
  permissions[5] = (file_stat.st_mode & S_IXGRP) ? 'x' : '-';
  permissions[6] = (file_stat.st_mode & S_IROTH) ? 'r' : '-';
  permissions[7] = (file_stat.st_mode & S_IWOTH) ? 'w' : '-';
  permissions[8] = (file_stat.st_mode & S_IXOTH) ? 'x' : '-';
  permissions[9] = '\0';

  // hard link count
  nlink_t hardLinkCount = file_stat.st_nlink;
  // owning user
  struct passwd *owningUser = getpwuid(file_stat.st_uid);
  // owning group
  struct group *owningGroup = getgrgid(file_stat.st_gid);
  // file size
  off_t fileSize = file_stat.st_size;
  // last modified timestamp
  struct tm *modTime = localtime(&file_stat.st_mtime);
  char timeString[20];
  strftime(timeString, sizeof(timeString), "%b %d %H:%M", modTime);
  // filename
  char *fileName = file;
  // print all info
  if (humanReadable) {
    char buffer[50];
    byteToHR(fileSize, buffer, sizeof(buffer));
    printf("%c%s %d %s %s %s %s %s\n", fileType, permissions, hardLinkCount, owningUser->pw_name, owningGroup->gr_name, buffer, timeString, fileName);
    return;
  }
  printf("%c%s %d %s %s %li %s %s\n", fileType, permissions, hardLinkCount, owningUser->pw_name, owningGroup->gr_name, fileSize, timeString, fileName);     
}

int main(int argc, char *argv[]) {
  DIR *d;
  struct dirent *dir;
  int opt;
  
  while ((opt = getopt_long(argc, argv, "aAhl", long_options, 0)) != -1) {
    switch (opt) {
    case 'a':
      includeALL = true;
      break;
    case 'A':
      includeALLshort = true;
      break;
    case 'h':
      humanReadable = true;
      break;
    case 'l':
      longFormat = true;
      break;
    case 1:
      print_help(argv[0]);
      return 0;
    default:
      print_help(argv[0]);
      return 1;
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
    if (longFormat) {
      printlongOutput(dir->d_name);
      continue;
    }
    printf("%s  ", dir->d_name);
  }
  if (!humanReadable) printf("\n");
  closedir(d);
  return (0);
}
