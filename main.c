#define LIBC_IMPL
#define FIFO_IMPL
#include "libc.h"
#include "fifo.h"

#define TASK_LENGHT pow(10, 5) /* 0.1s */
#define TASK_COUNT 100
#define THREADS_CAP 1

void *
process_task(void *datap)
{
	struct fifo *tasks = datap;
	int data;

	while (1) {
		puts("it blocks there");
		if (!fifo_is_empty(tasks)) {
			puts("this code never execute");
			fifo_out(tasks, &data, sizeof(data));
			if (data == -1) pthread_exit(0);
			printf("%lu => %d\n", pthread_self(), data);
			usleep(TASK_LENGHT);
		}
		puts("this code never execute");
	}
}

int
main(void) {
	pthread_t threads[THREADS_CAP];
	struct fifo *tasks;
	int data;
	size_t i;

	tasks = fifo_alloc(128);
	for (i = 0; i < THREADS_CAP; i++)
		pthread_create(threads + i, NULL, process_task, &tasks);

	for (i = 0; i < TASK_COUNT; i++)
		fifo_in(tasks, &i, sizeof(i));

	for (i = 0; i < THREADS_CAP; i++) {
		data = -1;
		fifo_in(tasks, &data, sizeof(i));
	}

	for (i = 0; i < THREADS_CAP; i++)
		pthread_join(threads[i], NULL);

	fifo_free(tasks);
}
