#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include <stdio.h>

#ifndef OPERATING_SYSTEM
# define OPERATING_SYSTEM "unknown"
#endif

/*
0 = all,
1 = kernel name,
2 = node name,
3 = kernel release,
4 = kernel version,
5 = machine hardware name,
6 = processor type (non-portable)
7 = hardware platform (non-portable)
8 = operating system

With no OPTION, same as 1
*/
int infoT = 1;

struct help_entry {
  const char *opt;
  const char *desc;
};

static struct option long_options[] = {
  {"all", no_argument, 0, 'a'},
  {"kernel-name", no_argument, 0, 's'},
  {"nodename", no_argument, 0, 'n'},
  {"kernel-release", no_argument, 0, 'r'},
  {"kernel-version", no_argument, 0, 'v'},
  {"machine", no_argument, 0, 'm'},
  {"processor", no_argument, 0, 'p'},
  {"hardware-platform", no_argument, 0, 'i'},
  {"operating-system", no_argument, 0, 'o'},
  {"help", no_argument, 0, 1},
  {0, 0, 0, 0}
};

static struct help_entry help_entries[] = {
  {"-a, --all", "print all information, in the following order,\n"
  "                           except omit -p and -i if unknown:"},
  {"-s, --kernel-name", "print the kernel name"},
  {"-n, --nodename", "print the network node hostname"},
  {"-r, --kernel-release", "print the kernel release"},
  {"-v, --kernel-version", "print the kernel version"},
  {"-m, --machine", "print the machine hardware name"},
  {"-p, --processor", "print the processor type (non-portable)"},
  {"-i, --hardware-platform", "print the hardware platform (non-portable)"},
  {"-o, --operating-system", "print the operating system"},
  {"    --help", "display this help and exit"}
};

void print_help(const char *name) {
  printf("Usage: %s [OPTION]...\n", name);
  printf("Print certain system information.  With no OPTION, same as -s.\n\n");

  // find longest option string
  int maxlen = 0;
  for (int i = 0; help_entries[i].opt; i++) {
    int len = (int)strlen(help_entries[i].opt);
    if (len > maxlen) maxlen = len;
  }

  // print each option aligned
  for (int i = 0; help_entries[i].opt; i++) {
    printf("  %-*s  %s\n", maxlen, help_entries[i].opt, help_entries[i].desc);
  }
}

// yeah im redefining sys/utsname.h
struct utsname {
  char sysname[65];  /* Operating system name (e.g., "Linux") */
  char nodename[65]; /* Name within communications network
                        to which the node is attached, if any */
  char release[65];  /* Operating system release
                        (e.g., "2.6.28") */
  char version[65];  /* Operating system version */
  char machine[65];  /* Hardware type identifier */
};

int uname(struct utsname *buf) {
  if (syscall(63, buf) != 0)
    return -1;
  return 0;
}

int main(int argc, char *argv[]) {
  
  int opt;
  struct utsname sys_info;

  while ((opt = getopt_long(argc, argv, "asnrvmpio", long_options, 0)) != -1) {
    switch (opt) {
      case 'a':
        infoT = 0;
        break;
      case 's':
        infoT = 1;
        break;
      case 'n':
        infoT = 2;
        break;
      case 'r':
        infoT = 3;
        break;
      case 'v':
        infoT = 4;
        break;
      case 'm':
        infoT = 5;
        break;
      case 'p':
        infoT = 6;
        break;
      case 'i':
        infoT = 7;
        break;
      case 'o':
        infoT = 8;
        break;
      case 1:
        print_help(argv[0]);
        return 0;
      case '?':
        if (1) {}

        // getopt_long automatically prints out "unrecognized option"
        char buffer[1024];
        sprintf(buffer, "Try '%s --help' for more information.\n", argv[0]);
        write(2, buffer, strlen(buffer));
        return 1;
      }
  }

  if (uname(&sys_info) == -1) {
    char buffer[256] = "rm: cannot retrieve system info: ";
    strncat(buffer, strerror(errno), sizeof(buffer) - strlen(buffer) - strlen(strerror(errno))); strncat(buffer, "\n", sizeof(buffer) - strlen(buffer) - 1);
    write(2, buffer, strlen(buffer));
  }
  char processor[257];
  char platform[257];

  // might break, who knows?
  if (infoT == 6) {
  # ifdef SI_ARCHITECTURE
      char proc[257];
      if (0 <= sysinfo(SI_ARCHITECTURE, proc, sizeof(proc)))
        strncpy(processor, proc, sizeof(processor) - strlen(proc) - 1);
      else
        strncpy(processor, "unknown\n", sizeof(processor) - 9);
  # elif HAVE_GETAUXVAL && defined(AT_PLATFORM)
      char const *p = (char const *) getauxval(AT_PLATFORM);
      if (p != NULL)
        strncpy(processor, p, sizeof(processor) - strlen(p) - 1);
      else
        strncpy(processor, "unknown\n", sizeof(processor) - 9);
  # else
      strncpy(processor, "unknown\n", sizeof(processor) - 9);
  # endif
  }

  if (infoT == 7) {
    # ifdef SI_PLATFORM
      char pf[257];
      if (0 <= sysinfo(SI_PLATFORM, pf, sizeof(pf)))
        strncpy(platform, pf, sizeof(platform) - strlen(pf) - 1);
      else
        strncpy(platform, "unknown\n", sizeof(platform) - 9);
    # else
        strncpy(platform, "unknown\n", sizeof(platform) - 9);
    # endif
  }
  
  // maybe i should take a look into coreutils actual source
  // just to write a better logic for printing
  char msg[1024];
  switch (infoT) {
    case 0:
      if (1) {}
      sprintf(msg, "%s %s %s %s %s %s\n", sys_info.sysname, sys_info.nodename, sys_info.release, sys_info.version, sys_info.machine, OPERATING_SYSTEM);
      write(1, msg, strlen(msg));
      break;
    case 1:
      strncpy(msg, sys_info.sysname, sizeof(msg) - strlen(sys_info.sysname) - 1);
      strncat(msg, "\n", sizeof(msg) - 2);
      write(1, msg, strlen(msg));
      break;
    case 2:
      strncpy(msg, sys_info.nodename, sizeof(msg) - strlen(sys_info.nodename) - 1);
      strncat(msg, "\n", sizeof(msg) - 2);
      write(1, msg, strlen(msg));
      break;
    case 3:
      strncpy(msg, sys_info.release, sizeof(msg) - strlen(sys_info.release) - 1);
      strncat(msg, "\n", sizeof(msg) - 2);
      write(1, msg, strlen(msg));
      break;
    case 4:
      strncpy(msg, sys_info.version, sizeof(msg) - strlen(sys_info.version) - 1);
      strncat(msg, "\n", sizeof(msg) - 2);
      write(1, msg, strlen(msg));
      break;
    case 5:
      strncpy(msg, sys_info.machine, sizeof(msg) - strlen(sys_info.machine) - 1);
      strncat(msg, "\n", sizeof(msg) - 2);
      write(1, msg, strlen(msg));
      break;
    case 6:
      write(1, processor, strlen(processor));
      break;
    case 7:
      write(1, platform, strlen(platform));
      break;
    case 8:
      strncpy(msg, OPERATING_SYSTEM, sizeof(msg) - strlen(OPERATING_SYSTEM) - 1);
      strncat(msg, "\n", sizeof(msg) - 2);
      write(1, msg, strlen(msg));
      break;
  }
  return 0;
}
