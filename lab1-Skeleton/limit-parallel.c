#include "limit-parallel.h"

#include <sys/wait.h>

static size_t current_processes = 0;
static size_t max_processes = -1;

void setparallel(size_t n)
{
	max_processes = n;
}

pid_t limitfork()
{
	if (current_processes >= max_processes)
		return -1;
	current_processes++;
	return fork();
}

pid_t limitwait(int *status)
{
	if (current_processes == 0)
		return -1;
	pid_t ret = wait(status);
	if (ret == -1)
		return -1;
	if (ret == 0)
		return ret;
	current_processes--;
	return ret;
}

pid_t limitwaitpid(pid_t pid, int *status, int options)
{
	if (current_processes == 0)
		return -1;
	pid_t ret = waitpid(pid, status, options);
	if (ret == -1)
		return -1;
	if (ret == 0)
		return ret;
	current_processes--;
	return ret;
}
