#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/wait.h>

#ifdef IS32
#include <asm/unistd_32.h>
#endif

#ifndef WORDLEN
#define WORDLEN 4
#endif


int dumper(char *argv[], const char *SUBSTR, unsigned long long SKIPS, void (*callback)(char *, unsigned long long));
