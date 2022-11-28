#include "param.h"
#include "types.h"
#include "stat.h"
#include "fs.h"
#include "fcntl.h"
#include "syscall.h"
#include "traps.h"
#include "memlayout.h"

//#define stat xv6_stat  // avoid clash with host struct stat
#include "user.h"

unsigned long randstate = 1;
unsigned int rand()
{
  randstate = randstate * 1664525 + 1013904223;
  return randstate;
}

int hash_list[MAXFILE];

void hash_init()
{
  for (int i = 0; i < MAXFILE; i++) {
    hash_list[i] = 0;
  }
}

unsigned int hash_func(int num)
{
  if (hash_list[num]) {
    return hash_list[num];
  } else {
    hash_list[num] = rand();
    return hash_list[num];
  }
}

char buf[8192];
char buf2[MAXFILE / 2];
void integrity_edge()
{
  int fd, i, total, cc;

  printf(1, "integrity tests\n");

  unlink("bigfile");
  fd = open("bigfile", O_CREATE | O_RDWR);
  if (fd < 0) {
    printf(1, "cannot create bigfile");
    exit();
  }
  printf(1, "create test done\n");
  for (i = 0; i < MAXFILE; i++) {
    if (i % (MAXFILE / 10) == 0)
      printf(2, "\r%d%%", i / (MAXFILE / 10) * 10);

    memset(buf, hash_func(i) % 128, 511);
    if (write(fd, buf, 511) != 511) {
      printf(1, "write bigfile failed\n");
      exit();
    }
  }
  printf(2, "\n");
  close(fd);

  printf(1, "write test done\n");
  fd = open("bigfile", 0);
  if (fd < 0) {
    printf(1, "cannot open bigfile\n");
    exit();
  }
  total = 0;
  for (i = 0;; i++) {
    if (i % (MAXFILE / 10) == 0)
      printf(2, "\r%d%%", i / (MAXFILE / 10) * 10);
    cc = read(fd, buf, 511);
    if (cc < 0) {
      printf(1, "read bigfile failed\n");
      exit();
    }
    if (cc == 0)
      break;
    if (cc != 511) {
      printf(1, "short read bigfile\n");
      exit();
    }
    if (buf[0] != (hash_func(i) % 128)
        || buf[510] != (hash_func(i) % 128)) {
      printf(1, "read bigfile wrong data! buf[0] = %d, not %d/ buf[510]= %d, not %d\n", buf[0], (hash_func(i) % 128), buf[510], (hash_func(i) % 128));
      exit();
    }
    total += cc;
  }
  printf(2, "\n");
  close(fd);
  printf(1, "read test done\n");
  if (total != MAXFILE * 511) {
    printf(1, "read bigfile wrong total\n");
    exit();
  }
  printf(1, "before unlink\n");
  unlink("bigfile");
  printf(1, "after unlink\n");

  printf(1, "integrity tests ok\n");
}

void integrity_big()
{
  int fd, total, cc;

  int wn;
  printf(1, "integrity test 2\n");

  unlink("bigfile");
  fd = open("bigfile", O_CREATE | O_RDWR);
  if (fd < 0) {
    printf(1, "cannot create bigfile");
    exit();
  }
  printf(1, "create test done\n");

  memset(buf2, hash_func(1) % 128, sizeof(buf2));
  if ((wn = write(fd, buf2, sizeof(buf2))) != sizeof(buf2)) {
    printf(1, "write bigfile failed with %d\n", wn);
    exit();
  }
  close(fd);

  printf(1, "write test done\n");
  fd = open("bigfile", 0);
  if (fd < 0) {
    printf(1, "cannot open bigfile\n");
    exit();
  }
  total = 0;
  cc = read(fd, buf2, sizeof(buf2));
  if (cc < 0) {
    printf(1, "read bigfile failed\n");
    exit();
  }
  if (cc == 0)
    if (cc != sizeof(buf2)) {
      printf(1, "short read bigfile\n");
      exit();
    }
  if (buf2[0] != (hash_func(1) % 128)
      || buf2[sizeof(buf2) - 1] != (hash_func(1) % 128)) {
    printf(1, "read bigfile wrong data! buf2222[0] = %d, not %d/ buf2[199]= %d, not %d\n", buf2[0], (1 % 128) / 2, buf2[512], (1 % 128) / 2);
    exit();
  }
  total += cc;
  close(fd);
  printf(1, "read test done\n");
  if (total != sizeof(buf2)) {
    printf(1, "read bigfile wrong total\n");
    exit();
  }
  printf(1, "before unlink\n");
  unlink("bigfile");
  printf(1, "after unlink\n");

  printf(1, "integrity test 2 ok\n");
}

int main()
{
  integrity_edge();
  printf(1, "---------------\n");
  integrity_big();
  exit();
}
