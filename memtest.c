#include "types.h"
#include "user.h"

int main() {
	printf(1, "initial state of VM of this process\n");
	pvminfo();
	int buf[100] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,};
	int *a = (int*)malloc(4096*4);
	a[2] = buf[10];
	int *b = (int*)malloc(4096*4);
	b[5] = buf[11];
	pvminfo();
	sleep(100);
	exit();
}
