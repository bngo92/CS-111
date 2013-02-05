#ifndef LIMIT_PARALLEL_H
#define LIMIT_PARALLEL_H

#include <unistd.h>

void setparallel(size_t n);
pid_t limitfork();
pid_t limitwait(int *status);
pid_t limitwaitpid(pid_t pid, int *status, int options);

#endif // LIMIT_PARALLEL_H
