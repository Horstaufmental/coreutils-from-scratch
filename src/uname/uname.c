#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <sys/utsname.h>
#include <malloc.h>
#include <stdbool.h>

#define BUF_SIZE 1025

#ifndef OPERATING_SYSTEM
  #define OPERATING_SYSTEM "unknown"
#endif

#define P_KERNEL     (1 << 0)
#define P_NODE       (1 << 1)
#define P_KERNELREL  (1 << 2)
#define P_KERNELVER  (1 << 3)
#define P_MACHINE    (1 << 4)
#define P_PROCESSOR  (1 << 5)
#define P_HWPLATFORM (1 << 6)
#define P_OS         (1 << 7)
   
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
  {"-a, --all",               "print all information, in the following order,\n"
  "                            except omit -p and -i if unknown:"},
  {"-s, --kernel-name",       "print the kernel name"},
  {"-n, --nodename",          "print the network node hostname"},
  {"-r, --kernel-release",    "print the kernel release"},
  {"-v, --kernel-version",    "print the kernel version"},
  {"-m, --machine",           "print the machine hardware name"},
  {"-p, --processor",         "print the processor type (non-portable)"},
  {"-i, --hardware-platform", "print the hardware platform (non-portable)"},
  {"-o, --operating-system",  "print the operating system"},
  {"    --help",              "display this help and exit"}
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

void print_to_var(char *buf, char *str, bool comma) {
  char buffer[BUF_SIZE];
  buffer[0] = '\0';
  // printf("buf (1) : |%s\n", buf);
  // printf("str     : |%s\n", str);
  snprintf(buffer, sizeof(buffer), "%s", buf ? buf : "");
  // printf("buffer  : |%s\n", buffer);
  if (strlen(buffer) == 0) {
    snprintf(buf, BUF_SIZE, "%s", str);
  } else {
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wformat-truncation"
    snprintf(buf, BUF_SIZE, "%s%s%s", buffer, comma ? "," : " ", str);
    #pragma GCC diagnostic pop
    // printf("buf (2) : |%s\n\n", buf);
  }

  if (strlen(buf) >= BUF_SIZE - 1) {
    fprintf(stderr, "warning: print_to_var() output truncated.\n");
  }
}

int to_print(unsigned int flags, char *proc, char *plat) {
  if (flags == 0) { // "With no OPTION, same as -s"
    flags |= P_KERNEL;
  }
  
  struct utsname buf;
  if (uname(&buf) != 0) {
    return 1;
  }

  char *buffer = malloc(1025);
  if (buffer == NULL) {
    return 1;
  }
  buffer[0] = '\0';
  
  if (flags & P_KERNEL)
    print_to_var(buffer, buf.sysname, false);
  if (flags & P_NODE)
    print_to_var(buffer, buf.nodename, false);
  if (flags & P_KERNELREL)
    print_to_var(buffer, buf.release, false);
  if (flags & P_KERNELVER)
    print_to_var(buffer, buf.version, false);
  if (flags & P_MACHINE)
    print_to_var(buffer, buf.machine, false);
  if (flags & P_PROCESSOR)
    print_to_var(buffer, proc, false);
  if (flags & P_HWPLATFORM)
    print_to_var(buffer, plat, false);
  if (flags & P_OS)
    print_to_var(buffer, OPERATING_SYSTEM, false);
  puts(buffer);

  free(buffer);
  return 0;
}

int main(int argc, char *argv[]) {
  char processor[257];
  char platform[257];
  
  #ifdef SI_ARCHITECTURE
    char proc[257];
    if (0 <= sysinfo(SI_ARCHITECTURE, proc, sizeof(proc)))
      strncpy(processor, proc, sizeof(processor) - strlen(proc) - 1);
    else
      strncpy(processor, "unknown", sizeof(processor) - 9);
  #elif HAVE_GETAUXVAL && defined(AT_PLATFORM)
    char const *p = (char const *) getauxval(AT_PLATFORM);
    if (p != NULL)
      strncpy(processor, p, sizeof(processor) - strlen(p) - 1);
    else
      strncpy(processor, "unknown", sizeof(processor) - 9);
  #else
      strncpy(processor, "unknown", sizeof(processor) - 9);
  #endif

  #ifdef SI_PLATFORM
    char pf[257];
    if (0 <= sysinfo(SI_PLATFORM, pf, sizeof(pf)))
      strncpy(platform, pf, sizeof(platform) - strlen(pf) - 1);
    else
      strncpy(platform, "unknown", sizeof(platform) - 9);
  #else
      strncpy(platform, "unknown", sizeof(platform) - 9);
  #endif
  
  unsigned int flags = 0;
  int opt;

  while ((opt = getopt_long(argc, argv, "asnrvmpio", long_options, 0)) != -1) {
    switch (opt) {
      case 'a':
        flags |= P_KERNEL | P_NODE | P_KERNELREL | P_KERNELVER | P_MACHINE | P_OS;
        if (strcmp(processor, "unknown") != 0) flags |= P_PROCESSOR;
        if (strcmp(platform, "unknown") != 0) flags |= P_HWPLATFORM;
        break;
      case 's':
        flags |= P_KERNEL;
        break;
      case 'n':
        flags |= P_NODE;
        break;
      case 'r':
        flags |= P_KERNELREL;
        break;
      case 'v':
        flags |= P_KERNELVER;
        break;
      case 'm':
        flags |= P_MACHINE;
        break;
      case 'p':
        flags |= P_PROCESSOR;
        break;
      case 'i':
        flags |= P_HWPLATFORM;
        break;
      case 'o':
        flags |= P_OS;
        break;
      case 1:
        print_help(argv[0]);
        return 0;
      case '?':
        printf("Try '%s --help' for more information.\n", argv[0]);
        return 1;
      }
  }

  if (to_print(flags, processor, platform) != 0) {
    fprintf(stderr, "uname: cannot retrieve system info: %s\n", strerror(errno));
  }
 
  return 0;
}
