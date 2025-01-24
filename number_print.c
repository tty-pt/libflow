#include <mov.h>
#include <stdio.h>

int mov_run() {
	unsigned number = 0;
	mov_read(0, &number, sizeof(number));
	printf("number: %u\n", number);
	return 0;
}
