#include "ftp.hpp"
#include "ui.hpp"
#include <chrono>
#include <iostream>
#include <sched.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <thread>

// 8mb stack
#define STACK_SIZE (1024 * 1024 * 8)
#define TRY(_call_)                                                            \
    {                                                                          \
        if ((_call_) == -1) {                                                  \
            std::cout << strerror(errno) << std::endl;                         \
            goto failed;                                                       \
        }                                                                      \
    }

int wait_child(pid_t pid) {
    int status;
    if (waitpid(pid, &status, 0) == -1)
        return -1;
    return (WIFEXITED(status) && WEXITSTATUS(status) == 2);
}

void test_dc() {
    ftp::Ftp f;
    f.login("10.1.1.100", 21, "user", "pass");
    f.set_active();

    f.list();
    f.list();

    f.store("src/main.cpp", "main.cpp");
    f.store("src/main.cpp", "main.cpp");

    f.store("src/main.cpp", "src/main.cpp");
    f.store("src/main.cpp", "src/main.cpp");

    f.retrieve("/tmp/main.cpp", "main.cpp");
    f.retrieve("/tmp/main.cpp", "main.cpp");

    f.retrieve("/tmp/main.cpp", "src/main.cpp");
    f.retrieve("/tmp/main.cpp", "src/main.cpp");

    f.set_passive();

    f.list();
    f.list();

    f.store("src/main.cpp", "main.cpp");
    f.store("src/main.cpp", "main.cpp");

    f.store("src/main.cpp", "src/main.cpp");
    f.store("src/main.cpp", "src/main.cpp");

    f.retrieve("/tmp/main.cpp", "main.cpp");
    f.retrieve("/tmp/main.cpp", "main.cpp");

    f.retrieve("/tmp/main.cpp", "src/main.cpp");
    f.retrieve("/tmp/main.cpp", "src/main.cpp");

    f.quit();
}

int main() {
    char *stack = (char *)malloc(STACK_SIZE);
    if (stack == nullptr)
        return 1;
    char *stack_top = stack + STACK_SIZE;

    ftp::Ftp f;

    int stop = 0;
    while (stop != 1) {
        sigset_t set;
        TRY(sigemptyset(&set));
        TRY(sigprocmask(SIG_SETMASK, &set, nullptr));

        pid_t pid;
        TRY(pid = clone(ui::run, stack_top, SIGCHLD | CLONE_VM | CLONE_FILES,
                        &f));

        TRY(sigaddset(&set, SIGCHLD));
        TRY(sigaddset(&set, SIGINT));
        TRY(sigprocmask(SIG_BLOCK, &set, nullptr));

        siginfo_t info;
        TRY(sigwaitinfo(&set, &info));

        if (info.si_signo == SIGINT) {
            kill(pid, SIGINT);
            TRY(sigemptyset(&set));
            TRY(sigaddset(&set, SIGCHLD));
            TRY(sigwaitinfo(&set, &info));
            if (info.si_signo == SIGCHLD) {
                TRY(stop = wait_child(pid));
                std::cout << std::endl;
            } else {
                goto failed;
            }
        } else if (info.si_signo == SIGCHLD) {
            TRY(stop = wait_child(pid));
        } else {
            goto failed;
        }
    }
    free(stack);
    return 0;
failed:
    free(stack);
    return EXIT_FAILURE;
}
