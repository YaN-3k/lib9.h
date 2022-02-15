/* #include "libc.h" */

struct fifo {
	size_t out;
	size_t in;
	size_t size;
	size_t len;
	pthread_spinlock_t lock;
	char data[1];
};

#define FIFO_CAST(name) (struct fifo *)&(name)
#define FIFO_INIT(name) {0, 0, SIZE(name.data), 0, 0, {0}}

#define FIFO_INIT_LOCK(fifo) \
        TRY(pthread_spin_init, &(fifo)->lock, PTHREAD_PROCESS_PRIVATE)

#define FIFO_DESTROY_LOCK(fifo) \
        TRY(pthread_spin_destroy, &(fifo)->lock)

#define DECLARE_FIFO(name, sz)           \
        struct {                         \
                size_t out;              \
                size_t in;               \
                size_t size;             \
                size_t len;              \
		pthread_spinlock_t lock; \
                char data[sz];           \
        } name
#define FIFO(name, sz) DECLARE_FIFO(name, sz) = FIFO_INIT(name)
#define INIT_FIFO(fifo) do {         \
        fifo.out = fifo.in = 0;      \
        fifo.size = SIZE(fifo.data); \
        fifo.len = 0;                \
} while (0)

struct fifo *fifo_alloc(size_t size);
void fifo_free(struct fifo *fifo);
void fifo_reset(struct fifo *fifo);
size_t fifo_in(struct fifo *fifo, const void *data, size_t len);
size_t fifo_peek(struct fifo *fifo, void *data, size_t len);
size_t fifo_skip(struct fifo *fifo, size_t len);
size_t fifo_out(struct fifo *fifo, void *data, size_t len);
size_t fifo_avail(struct fifo *fifo);
size_t fifo_len(struct fifo *fifo);
size_t fifo_size(struct fifo *fifo);
int fifo_is_empty(struct fifo *fifo);
int fifo_is_full(struct fifo *fifo);

#ifdef FIFO_IMPL

struct fifo *
fifo_alloc(size_t size)
{
	struct fifo *fifo;

	fifo = emalloc(offsetof(struct fifo, data) + size);
	fifo->out = fifo->in = 0;
	fifo->size = size;
	fifo->len = 0;
        FIFO_INIT_LOCK(fifo);
	return fifo;
}

void
fifo_free(struct fifo *fifo)
{
        FIFO_DESTROY_LOCK(fifo);
	free(fifo);
}

void
fifo_reset(struct fifo *fifo)
{
	TRY(pthread_spin_lock, &fifo->lock);
	fifo->out = fifo->in = 0;
	fifo->len = 0;
	TRY(pthread_spin_unlock, &fifo->lock);
}

size_t
fifo_in(struct fifo *fifo, const void *data, size_t len)
{
	size_t total, remaining, l;

	TRY(pthread_spin_lock, &fifo->lock);
	total = remaining = MIN(len, fifo->size - fifo->len);
	while (remaining > 0) {
		l = MIN(remaining, fifo->size - fifo->in);
		memcpy(fifo->data + fifo->in, data, l);
		fifo->in = (fifo->in + l) % fifo->size;
		fifo->len += l;
		data = (char *)data + l;
		remaining -= l;
	}
	TRY(pthread_spin_unlock, &fifo->lock);
	return total;
}

size_t
fifo_peek(struct fifo *fifo, void *data, size_t len)
{
	size_t total, remaining, out, l;

	TRY(pthread_spin_lock, &fifo->lock);
	out = fifo->out;
	total = remaining = MIN(len, fifo->len);
	while (remaining > 0) {
		l = MIN(remaining, fifo->size - out);
		memcpy(data, fifo->data + out, l);
		out = (out + l) % fifo->size;
		data = (char *)data + l;
		remaining -= l;
	}
	TRY(pthread_spin_unlock, &fifo->lock);
	return total;
}

size_t
fifo_skip(struct fifo *fifo, size_t len)
{
	size_t total, remaining, l;

	TRY(pthread_spin_lock, &fifo->lock);
	total = remaining = MIN(len, fifo->len);
	while (remaining > 0) {
		l = MIN(remaining, fifo->size - fifo->out);
		fifo->out = (fifo->out + l) % fifo->size;
		remaining -= l;
	}
	fifo->len -= total;
	TRY(pthread_spin_unlock, &fifo->lock);
	return total;
}

size_t
fifo_out(struct fifo *fifo, void *data, size_t len)
{
	fifo_peek(fifo, data, len);
	return fifo_skip(fifo, len);
}

size_t
fifo_len(struct fifo *fifo)
{
	size_t len;

	TRY(pthread_spin_lock, &fifo->lock);
	len = fifo->len;
	TRY(pthread_spin_unlock, &fifo->lock);
	return len;
}

size_t
fifo_avail(struct fifo *fifo)
{
	size_t avail;

	TRY(pthread_spin_lock, &fifo->lock);
	avail = fifo->size - fifo->len;
	TRY(pthread_spin_unlock, &fifo->lock);
	return avail;
}

size_t
fifo_size(struct fifo *fifo)
{
	size_t size;

	TRY(pthread_spin_lock, &fifo->lock);
	size = fifo->size;
	TRY(pthread_spin_unlock, &fifo->lock);
	return size;
}

int
fifo_is_empty(struct fifo *fifo)
{
	int empty;

	TRY(pthread_spin_lock, &fifo->lock);
	empty = fifo->len == 0;
	TRY(pthread_spin_unlock, &fifo->lock);
	return empty;
}

int
fifo_is_full(struct fifo *fifo)
{
	int full;

	TRY(pthread_spin_lock, &fifo->lock);
	full = fifo->size == fifo->len;
	TRY(pthread_spin_unlock, &fifo->lock);
	return full;
}

#endif

/* vim: set noet ts=8: */
