#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  if(argc < 2){
    printf(2, "Usage: hostname [new hostname]\n");
    exit();
  }

  sethostname(argv[1]);

  exit();
}
