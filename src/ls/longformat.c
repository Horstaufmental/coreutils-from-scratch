#define _XOPEN_SOURCE 700

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

#include "args.h"
#include "longformat.h"
#include "bytetohr.h"

#define PATH_MAX 4096

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
void printlongOutput(const char *fullPath, const char *fileName)
{
    struct stat file_stat;

    char fileType;

    // TODO: check for network file
    if (lstat(fullPath, &file_stat) == 0)
    {
        if (S_ISREG(file_stat.st_mode))
        {
            fileType = '-';
        }
        else if (S_ISDIR(file_stat.st_mode))
        {
            fileType = 'd';
        }
        else if (S_ISLNK(file_stat.st_mode))
        {
            fileType = 'l';
        }
        else if (S_ISSOCK(file_stat.st_mode))
        {
            fileType = 's';
        }
        else if (S_ISFIFO(file_stat.st_mode))
        {
            fileType = 'p';
        }
        else if (S_ISCHR(file_stat.st_mode))
        {
            fileType = 'c';
        }
        else if (S_ISBLK(file_stat.st_mode))
        {
            fileType = 'b';
        }
    } else {
        perror(fullPath);
        return;
    }
    // printf("filetype %c\n", fileType);
    //  permissions
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

    // handle NULL user/group
    const char *userName = owningUser ? owningUser->pw_name : "unknown";
    const char *groupName = owningGroup ? owningGroup->gr_name : "unknown";

    // file size
    off_t fileSize = file_stat.st_size;
    // last modified timestamp
    struct tm *modTime = localtime(&file_stat.st_mtime);
    char timeString[20];
    strftime(timeString, sizeof(timeString), "%b %d %H:%M", modTime);
    // print all info
    if (humanReadable)
    {
        char buffer[50];
        byteToHR(fileSize, buffer, sizeof(buffer));
        printf("%c%s %lu %s %s %s %s %s\n", fileType, permissions, hardLinkCount, userName, groupName, buffer, timeString, fileName);
        return;
    }
    printf("%c%s %lu %s %s %li %s %s\n", fileType, permissions, hardLinkCount, userName, groupName, fileSize, timeString, fileName);
}