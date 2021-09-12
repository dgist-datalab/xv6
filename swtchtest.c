#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  int i;

  if (argc != 2) {
    printf(1, "Usage: %s <cswitch set number>\n", argv[0]);
    exit();
  }

  for (i = 0; i < 10; i++) {
    printf(1, "%s performed %d context switches\n", argv[0], getcswitch());
  }

  printf(1, "\nAdding delays\n");

  for (i = 0; i < 10; i++) {
    sleep(10);
    printf(1, "%s performed %d context switches\n", argv[0], getcswitch());
  }

  i = atoi(argv[1]);

  printf(1, "\nSetting cswitch to %d\n", i);
  setcswitch(i);
  printf(1, "%s performed %d context switches\n", argv[0], getcswitch());

  exit();
}
