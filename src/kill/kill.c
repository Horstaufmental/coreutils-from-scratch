#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <errno.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/wait.h>

struct help_entry {
  char *opt;
  char *desc;
};

// most distros will ship kill from utils-linux (such as the case with my arch linux)
// however, our goal is to be recreating **GNU** Coreutils, not utils-linux
struct option long_options[] = {
  {"signal", required_argument, 0, 's'},
  {"list", no_argument, 0, 'l'},
  {"table", no_argument, 0, 't'},
  {"help", no_argument, 0, 1},
  {0, 0, 0, 0}
};

static struct help_entry help_entries[] = {
  {"-s, --signal=SIGNAL, -SIGNAL", "specify the name or number of the signal\n"
   "                                to be sent"},
  {"-l, --list", "list signal names, or convert signal names to/from numbers"},
  {"-t, --table", "print a table of signal information"},
  {"    --help", "display this help and exit"},
  {0, 0}
};

void print_help(const char *name) {
  printf("Usage: %s [-s SIGNAL | -SIGNAL] PID...\n"
         "  or:  %s -l [SIGNAL]...\n"
         "  or:  %s -t [SIGNAL]...\n", name, name, name);
  puts("Send signals to processes, or list signals.\n");

  puts("Mandatory arguments to long options are mandatory for short options too.\n");

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
  puts("\nSIGNAL may be a signal name like 'HUP' (except for -SIGNAL), or a signal number like '1',\n"
       "or the exit status of a process terminated by a signal.\n"
       "PID is an integer; if negative it identifies a process group."
  );
}

struct signalLists {
  const char *name;
  int num;
  const char *desc;
};
  
int signalFromName(const char *name, struct signalLists *sigs) {
  if (strncasecmp(name, "SIG", 3) == 0)
    name += 3;

  for (int i = 0; sigs[i].name; i++) {
    if (strcasecmp(name, sigs[i].name) == 0)
      return sigs[i].num;
  }

  return -1;
}

const char *signalFromNumber(int number, struct signalLists *sigs) {
  for (int i = 0; sigs[i].name; i++) {
    if (sigs[i].num == number)
      return sigs[i].name;
  }
  return NULL;
}

#define PRINT_TABLE (1 << 0)
#define PRINT_LIST  (1 << 1)
int parseStringToInt(const char *name, int *num) {
  char *endptr;
  errno = 0;

  double val = strtod(name, &endptr);

  if (endptr == name) return 0;   // no number found
  if (errno == ERANGE) return 0; // overflow/underflow
  if (*endptr != '\0') return 0; // extra junk at end

  *num = val;
  return 1;
}

int printSignals(const char *name, struct signalLists *sigs, int flags) {
  int num = -1;
  if (name != NULL)
    if (parseStringToInt(name, &num) != 1) {
      return 1;
    }
  if (flags & PRINT_LIST) {
    if (num == -1 && name == NULL) {
      // no arguments specified, print all names
      for (int i = 0; sigs[i].name; i++) {
        puts(sigs[i].name);
      }
    } else if (num > -1 && name == NULL) {
      // number to string
      const char *signal = signalFromNumber(num, sigs);
      if (signal == NULL) {
        return 1;
      }
      puts(signal);
    } else if (num == -1 && name != NULL) {
      // string to number
      int signal = signalFromName(name, sigs);
      if (signal == -1) {
        return 1;
      }
      printf("%d\n", signal);
    }
  } else if (flags & PRINT_TABLE) {
    for (int i = 0; sigs[i].name; i++) {
      printf("%2d %-8s %s\n", sigs[i].num, sigs[i].name, sigs[i].desc);
    }
  }
  return 0;
}

int main(int argc, char *argv[]) {
  int SigRealtimeMin = SIGRTMIN;
  int SigRealtimeMax = SIGRTMAX;
  struct signalLists sigs[] = {
    {"EXIT", 0, "Unknown signal 0"},
    {"HUP", SIGHUP, "Hangup"},
    {"INT", SIGINT, "Interrupt"},
    {"QUIT", SIGQUIT, "Quit"},
    {"ILL", SIGILL, "Illegal instruction"},
    {"TRAP", SIGTRAP, "Trace/breakpoint trap"},
    {"ABRT", SIGABRT, "Aborted"},
    {"BUS", SIGBUS, "Bus error"},
    {"FPE", SIGFPE, "Floating point exception"},
    {"KILL", SIGKILL, "Killed"},
    {"USR1", SIGUSR1, "User defined signal 1"},
    {"SEGV", SIGSEGV, "Segmentation fault"},
    {"USR2", SIGUSR2, "User defined signal 2"},
    {"PIPE", SIGPIPE, "Broken pipe"},
    {"ALRM", SIGALRM, "Alarm clock"},
    {"TERM", SIGTERM, "Terminated"},
    {"STKFLT", SIGSTKFLT, "Stack fault"},
    {"CHLD", SIGCHLD, "Child exited"},
    {"CONT", SIGCONT, "Continued"},
    {"STOP", SIGSTOP, "Stopped (signal)"},
    {"TSTP", SIGTSTP, "Stopped"},
    {"TTIN", SIGTTIN, "Stopped (tty input)"},
    {"TTOU", SIGTTOU, "Stopped (tty output)"},
    {"URG", SIGURG, "Urgent I/O condition"},
    {"XCPU", SIGXCPU, "CPU time limit exceeded"},
    {"XFSZ", SIGXFSZ, "File size limit exceeded"},
    {"VTALRM", SIGVTALRM, "Virtual timer expired"},
    {"PROF", SIGPROF, "Profiling timer expired"},
    {"WINCH", SIGWINCH, "Window changed"},
    {"POLL", SIGPOLL, "I/O possible"},
    {"PWR", SIGPWR, "Power failure"},
    {"SYS", SIGSYS, "Bad system call"},
    {"RTMIN", SigRealtimeMin, "Real-time signal 0"},
    {"RTMIN+1",  (SigRealtimeMin+1  <= SigRealtimeMax) ? SigRealtimeMin+1  : SigRealtimeMax, "Real-time signal 1"},
    {"RTMIN+2",  (SigRealtimeMin+2  <= SigRealtimeMax) ? SigRealtimeMin+2  : SigRealtimeMax, "Real-time signal 2"},
    {"RTMIN+3",  (SigRealtimeMin+3  <= SigRealtimeMax) ? SigRealtimeMin+3  : SigRealtimeMax, "Real-time signal 3"},
    {"RTMIN+4",  (SigRealtimeMin+4  <= SigRealtimeMax) ? SigRealtimeMin+4  : SigRealtimeMax, "Real-time signal 4"},
    {"RTMIN+5",  (SigRealtimeMin+5  <= SigRealtimeMax) ? SigRealtimeMin+5  : SigRealtimeMax, "Real-time signal 5"},
    {"RTMIN+6",  (SigRealtimeMin+6  <= SigRealtimeMax) ? SigRealtimeMin+6  : SigRealtimeMax, "Real-time signal 6"},
    {"RTMIN+7",  (SigRealtimeMin+7  <= SigRealtimeMax) ? SigRealtimeMin+7  : SigRealtimeMax, "Real-time signal 7"},
    {"RTMIN+8",  (SigRealtimeMin+8  <= SigRealtimeMax) ? SigRealtimeMin+8  : SigRealtimeMax, "Real-time signal 8"},
    {"RTMIN+9",  (SigRealtimeMin+9  <= SigRealtimeMax) ? SigRealtimeMin+9  : SigRealtimeMax, "Real-time signal 9"},
    {"RTMIN+10", (SigRealtimeMin+10 <= SigRealtimeMax) ? SigRealtimeMin+10 : SigRealtimeMax, "Real-time signal 10"},
    {"RTMIN+11", (SigRealtimeMin+11 <= SigRealtimeMax) ? SigRealtimeMin+11 : SigRealtimeMax, "Real-time signal 11"},
    {"RTMIN+12", (SigRealtimeMin+12 <= SigRealtimeMax) ? SigRealtimeMin+12 : SigRealtimeMax, "Real-time signal 12"},
    {"RTMIN+13", (SigRealtimeMin+13 <= SigRealtimeMax) ? SigRealtimeMin+13 : SigRealtimeMax, "Real-time signal 13"},
    {"RTMIN+14", (SigRealtimeMin+14 <= SigRealtimeMax) ? SigRealtimeMin+14 : SigRealtimeMax, "Real-time signal 14"},
    {"RTMIN+15", (SigRealtimeMin+15 <= SigRealtimeMax) ? SigRealtimeMin+15 : SigRealtimeMax, "Real-time signal 15"},
    {"RTMAX-14", (SigRealtimeMax-14 <= SigRealtimeMax) ? SigRealtimeMax-14 : SigRealtimeMax, "Real-time signal 16"},
    {"RTMAX-14", (SigRealtimeMax-14 <= SigRealtimeMax) ? SigRealtimeMax-14 : SigRealtimeMax, "Real-time signal 17"},
    {"RTMAX-14", (SigRealtimeMax-14 <= SigRealtimeMax) ? SigRealtimeMax-14 : SigRealtimeMax, "Real-time signal 18"},
    {"RTMAX-14", (SigRealtimeMax-14 <= SigRealtimeMax) ? SigRealtimeMax-14 : SigRealtimeMax, "Real-time signal 19"},
    {"RTMAX-14", (SigRealtimeMax-14 <= SigRealtimeMax) ? SigRealtimeMax-14 : SigRealtimeMax, "Real-time signal 20"},
    {"RTMAX-9",  (SigRealtimeMax-9  <= SigRealtimeMax) ? SigRealtimeMax-9  : SigRealtimeMax, "Real-time signal 21"},
    {"RTMAX-8",  (SigRealtimeMax-8  <= SigRealtimeMax) ? SigRealtimeMax-8  : SigRealtimeMax, "Real-time signal 22"},
    {"RTMAX-7",  (SigRealtimeMax-7  <= SigRealtimeMax) ? SigRealtimeMax-7  : SigRealtimeMax, "Real-time signal 23"},
    {"RTMAX-6",  (SigRealtimeMax-6  <= SigRealtimeMax) ? SigRealtimeMax-6  : SigRealtimeMax, "Real-time signal 24"},
    {"RTMAX-5",  (SigRealtimeMax-5  <= SigRealtimeMax) ? SigRealtimeMax-5  : SigRealtimeMax, "Real-time signal 25"},
    {"RTMAX-4",  (SigRealtimeMax-4  <= SigRealtimeMax) ? SigRealtimeMax-4  : SigRealtimeMax, "Real-time signal 26"},
    {"RTMAX-3",  (SigRealtimeMax-3  <= SigRealtimeMax) ? SigRealtimeMax-3  : SigRealtimeMax, "Real-time signal 27"},
    {"RTMAX-2",  (SigRealtimeMax-2  <= SigRealtimeMax) ? SigRealtimeMax-2  : SigRealtimeMax, "Real-time signal 28"},
    {"RTMAX-1",  (SigRealtimeMax-1  <= SigRealtimeMax) ? SigRealtimeMax-1  : SigRealtimeMax, "Real-time signal 29"},
    {"RTMAX", SigRealtimeMax, "Real-time signal 30"},
    {NULL, 0, NULL}
  };

  unsigned int flags;
  int sig = SIGTERM; // default if no SIGNAL is specified
  int opt;
  const char *arg;

  // WHY IS IT SO HARD TO IMPLEMENT '-SIGNAL'?
  // yknow what? im tired of dealing with named signals fuck this shit
  opterr = 0; // suppress getopt_long's error messages
  while (1) {
    opt = getopt_long(argc, argv, "+s:lt", long_options, NULL);

    if (opt == -1)
      break;

    if (opt == '?') {
      // lets take a look... :eyes:
      const char *maybe = NULL;

      if (optind > 0 && optind <= argc)
        maybe = argv[optind - 1];
      else if (optind < argc)
        maybe = argv[optind];

      if (maybe && maybe[0] == '-' && maybe[1] != '\0' && maybe[1] != '-') {
        optind--;
        break;
      }

      fprintf(stderr, "%s: invalid option -- '%s'\n"
      "Try '%s --help' for more information.\n",
      argv[0], maybe ? maybe : "?", argv[0]);
      return 1;
    }

    switch (opt) {
      case 's':
        if (optarg != NULL) { // check if is number
          int num;
          if (parseStringToInt(optarg, &num) != 1) { // is string
            if ((num = signalFromName(optarg, sigs)) == -1) {
              fprintf(stderr, "kill: '%s': invalid signal\n", optarg);
              return 1;
            }
          }
          if (num < 0 || num > 64) {
            fprintf(stderr, "kill: '%d': invalid signal\n", num);
            return 1;
          }
          sig = num;
        }
        break;
      case 'l':
        flags |= PRINT_LIST;
        break;
      case 't':
        flags |= PRINT_TABLE;
        break;
      case 1:
        print_help(argv[0]);
        return 0;
    }
  }


  if (optind < argc) {
    arg = argv[optind];

    if (strcmp(arg, "--") == 0) {
      optind++;
    } else if (arg[0] == '-' && arg[1] != '\0' && arg[1] != '-') {
      int num;
      const char *sigstr = arg + 1;

      if (parseStringToInt(sigstr, &num) != 1) {
        num = signalFromName(sigstr, sigs);
      }

      if (num != -1) {
        sig = num;
        optind++;
      }
    }
  }


  if (flags & PRINT_LIST) {
    if (arg == NULL) {
      if ((flags & (PRINT_LIST | PRINT_TABLE)) != (PRINT_LIST | PRINT_TABLE)) // PRINT_LIST !=NAND=! PRINT_TABLE
        printSignals(NULL, sigs, PRINT_LIST);
      else {
        fprintf(stderr, "kill: multiple -l or -t options specified\n"
        "Try '%s --help' for more information.\n", argv[0]);
        return 1;
      }
      return 0;
    } else {
      // check if is number
      int num;
      if (parseStringToInt(arg, &num) != 1) { // is string
        if ((num = signalFromName(arg, sigs)) == -1) {
          fprintf(stderr, "kill '%s': invalid signal\n", arg);
          return 1;
        }
        printf("%d\n", num);
        return 0;
      } else { // is number
        if (num < 0 || num > 64) {
          fprintf(stderr, "kill: '%d': invalid signal\n", num);
          return 1;
        }
        const char *name = signalFromNumber(num, sigs);
        if (name == NULL) {
          fprintf(stderr, "kill '%d': invalid signal\n", num);
          return 1;
        }
        puts(name);
        return 0;
      }
    }
  } else if (flags & PRINT_TABLE) {
    if ((flags & (PRINT_LIST | PRINT_TABLE)) != (PRINT_LIST | PRINT_TABLE)) // PRINT_LIST !=NAND=! PRINT_TABLE
      printSignals(NULL, sigs, PRINT_TABLE);
    else {
      fprintf(stderr, "kill: multiple -l or -t options specified\n"
      "Try '%s --help' for more information.\n", argv[0]);
      return 1;
    }
    return 0;
  } else if (flags & (PRINT_LIST | PRINT_TABLE)) {
    fprintf(stderr, "kill: multiple -l or -t options specified\n"
    "Try '%s --help' for more information.\n", argv[0]);
    return 1;
  }

  if (optind == argc) {
    fprintf(stderr, "kill: no process ID specified\n"
                    "Try '%s --help' for more information.\n", argv[0]
    );
    return 1;
  }

  for (; optind < argc; optind++) {
    pid_t pid;
    if (parseStringToInt(argv[optind], &pid) != 1) {
      fprintf(stderr, "kill: '%s': invalid process id\n", argv[optind]);
      return 1;
    }
    if (kill(pid, sig) != 0) {
      fprintf(stderr, "kill: sending signal to %d failed: %s\n", pid, strerror(errno));
      return 1;
    }
  }
  return 0;
}
