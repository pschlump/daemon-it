#include <sys/resource.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int main (int argc, char *argv[])
{
  struct rlimit limit;

  limit.rlim_cur = 65535;
  limit.rlim_max = 65535;
  if (setrlimit(RLIMIT_NOFILE, &limit) != 0) {
    printf("setrlimit() failed with errno=%d\n", errno);
    return 1;
  }

  /* Get max number of files. */
  if (getrlimit(RLIMIT_NOFILE, &limit) != 0) {
    printf("getrlimit() failed with errno=%d\n", errno);
    return 1;
  }

  printf("The soft limit is %llu\n", limit.rlim_cur);
  printf("The hard limit is %llu\n", limit.rlim_max);

  /* Also children will be affected: */
  system("bash -c 'ulimit -a'");

  return 0;
}
