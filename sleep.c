#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  int i;

  if(argc < 2){
    printf(2, "Usage: sleep [seconds]\n");
    exit();
  }

  i = atoi(argv[1]);
  sleep(i * 100);

  exit();
}
