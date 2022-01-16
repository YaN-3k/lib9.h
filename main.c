#include "libc.h"
#define UTIL_IMPL
#define UTIL_MEMCHECK
#include "util.h"

int main() {
	(void) malloc(sizeof(int));
	dumpmem();
	return 0;
}
