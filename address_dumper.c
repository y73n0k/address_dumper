#include "address_dumper.h"

typedef union
{
    long val;
    char s[WORDLEN];
} l2b;  // long to string converter


char *string_dumper(pid_t pid, unsigned long long address) {
    // just dumping string from tracee's address 
    char *string = malloc(WORDLEN * sizeof(char));
    unsigned long long old_length, length = 0, capacity = WORDLEN, counter = 0;
    unsigned char finished = 0;
    l2b t;
    while (!finished) {
        old_length = length;
        t.val = ptrace(PTRACE_PEEKDATA, pid, address + WORDLEN * counter, 0);
        for (unsigned char i = 0; i < WORDLEN; ++i) {
            ++length;
            if (t.s[i] == 0) {
                finished = 1;
                break;
            }
        }

        if (length >= capacity) {
            capacity = 2 * length;
            string = realloc(string, sizeof(char) * capacity);
        }
        memcpy(string + old_length, t.s, WORDLEN);
        ++counter;
    }
    return string;
}


void step(pid_t pid, int *wait_status){
    // PTRACE_SYSCALL is catching entering and exiting from syscall
    ptrace(PTRACE_SYSCALL, pid, 0, 0);
    waitpid(pid, wait_status, 0);
}


int dumper(char *argv[], const char *SUBSTR, unsigned long long SKIPS, void (*callback)(char *, unsigned long long)) {
    pid_t pid = fork();

    if (pid < 0) {
        perror("Fork failed");
        exit(1);
    }

    if (pid == 0) {
        // run tracee
        ptrace(PTRACE_TRACEME, 0, NULL, 0);

        if (execvp(argv[1], argv + 1) == -1) {
            printf("%s: %s\n", argv[1], strerror(errno));
            exit(2);
        }

    } else {
        // tracer logic for finding library base
        int wait_status;
        struct user_regs_struct state;
        long fd;
        char found = 0, cont = 0;
        char *path = NULL;

        waitpid(pid, &wait_status, 0);

        if (!SKIPS) {
            ptrace(PTRACE_SETOPTIONS, pid, 0, PTRACE_O_TRACESYSGOOD | PTRACE_O_EXITKILL);
        } else {
            // PTRACE_O_TRACEEXEC ignores until execv syscall
            for (unsigned long long i = 0; i < SKIPS; ++i) {
                ptrace(PTRACE_SETOPTIONS, pid, 0, PTRACE_O_TRACESYSGOOD | PTRACE_O_EXITKILL | PTRACE_O_TRACEEXEC);
                ptrace(PTRACE_CONT, pid, 0, 0);
                waitpid(pid, &wait_status, 0);
            }
        }

        while (!WIFEXITED(wait_status)) {
            step(pid, &wait_status);

            ptrace(PTRACE_GETREGS, pid, 0, &state);

	    if (cont) ptrace(PTRACE_CONT, pid, 0, 0);

	    if (!found) {
                // searching for openat syscall with path which contains SUBSTR
                #ifdef IS32
                if (state.orig_rax == __NR_openat) {
		    free(path);
                    path = string_dumper(pid, state.rcx);
                #else
                if (state.orig_rax == SYS_openat) {
                    free(path);
                    path = string_dumper(pid, state.rsi);
                #endif

                    // printf("PATH: %s\n", path);

                    if (strstr(path, SUBSTR) != NULL) {
                        found = 1;
                        step(pid, &wait_status);
                        ptrace(PTRACE_GETREGS, pid, 0, &state);
                        fd = state.rax;  // openat returns fd, so save it to find mmap syscall with this fd
                    } else {
                        step(pid, &wait_status);
                    }
                }
            } else {
                // find first mmap(NULL,...,...,...,fd,...) after opening path
                #ifdef IS32
                if (state.orig_rax == __NR_mmap2) {
                #else
                if (state.orig_rax == SYS_mmap) {
                #endif
                    ptrace(PTRACE_GETREGS, pid, 0, &state);
                    #ifdef IS32
                    if (state.rdi == fd && !state.rbx) {
                    #else
                    if (state.r8 == fd && !state.rdi) {
                    #endif
                        step(pid, &wait_status);
                        ptrace(PTRACE_GETREGS, pid, 0, &state);
                        callback(path, state.rax); // mmap returns base of library
                        free(path);
			cont = 1;
                    }
                }
                step(pid, &wait_status);
            }
        }
        // ptrace(PTRACE_CONT, pid, 0, 0);
    }
    return 0;
}
