#include <mov.h>

int mov_run() {
	unsigned number = 3;
	mov_write(0, &number, sizeof(number));
	return 0;
}
