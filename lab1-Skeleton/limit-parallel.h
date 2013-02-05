#ifndef LIMIT_PARALLEL_H
#define LIMIT_PARALLEL_H

#include <unistd.h>

void setparallel(int n);
pid_t limitfork();
pid_t limitwait();

#endif // LIMIT_PARALLEL_H
