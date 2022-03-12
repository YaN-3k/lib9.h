struct fifo {
	size_t in;
	size_t out;
	size_t mask;
	size_t esize;
	char *data;
};

#define DECLARE_FIFO(name, type, size) struct {                               \
        struct fifo fifo;                                                     \
        type buf[size < 2 || size & (size - 1) ? -1 : size];                  \
} name[1]

#define FIFO_INIT(name) {{                                                    \
        {                                                                     \
                .in    = 0,                                                   \
                .out   = 0,                                                   \
                .mask  = nelem(name->buf) - 1,                                \
                .esize = sizeof(*name->buf),                                  \
                .data  = (char *)name->buf                                    \
        },                                                                    \
        .buf = {0}                                                            \
}}

#define FIFO(name, type, size) DECLARE_FIFO(name, type, size) = FIFO_INIT(name)

#define INIT_FIFO(ptr) do {                                                   \
        ptr->fifo.out   = ptr->fifo.in = 0;                                   \
        ptr->fifo.mask  = nelem(ptr->buf);                                    \
        ptr->fifo.esize = sizeof(*ptr->buf);                                  \
        ptr->fifo.data  = (char *)ptr->buf;                                   \
} while (0)

struct fifo *fifo_alloc(size_t size, size_t esize);
#define fifo_ealloc(...) ASSERT_PTR(fifo_alloc, __VA_ARGS__)
void fifo_free(struct fifo *fifo);
size_t fifo_len(const struct fifo *fifo);
size_t fifo_size(const struct fifo *fifo);
size_t fifo_avail(const struct fifo *fifo);
size_t fifo_esize(const struct fifo *fifo);
void fifo_reset(struct fifo *fifo);
void fifo_reset_out(struct fifo *fifo);
int fifo_is_empty(const struct fifo *fifo);
int fifo_is_full(const struct fifo *fifo);
size_t fifo_in(struct fifo *fifo, const void *src, size_t len);
#define fifo_put(fifo, src) fifo_in(fifo, src, 1)
size_t fifo_out(struct fifo *fifo, void *dst, size_t len);
#define fifo_get(fifo, dst) fifo_out(fifo, dst, 1)
size_t fifo_out_peek(const struct fifo *fifo, void *dst, size_t len);
#define fifo_peek(fifo, dst) fifo_peek(fifo, dst, 1)
size_t fifo_out_skip(struct fifo *fifo, size_t len);
#define fifo_skip(fifo) fifo_out_skip(fifo, 1)

#ifdef FIFO_IMPL

struct fifo *
fifo_alloc(size_t esize, size_t size)
{
	struct fifo *fifo;

	size |= size >> 1;
	size |= size >> 2;
	size |= size >> 4;
	size |= size >> 8;
	size |= size >> 16;
	size |= size >> 32;
	size += 1;

	fifo = malloc(sizeof(struct fifo));
	if (!fifo) return 0;
	fifo->data = malloc(size * esize);
	if (!fifo->data) {
		free(fifo);
		return 0;
	}
	fifo->mask = size - 1;
	fifo->esize = esize;
	fifo->in = fifo->out = 0;
	return fifo;
}

void
fifo_free(struct fifo *fifo)
{
	free(fifo->data);
	free(fifo);
}

size_t
fifo_len(const struct fifo *fifo)
{
	return fifo->in - fifo->out;
}

size_t
fifo_size(const struct fifo *fifo)
{
	return fifo->mask + 1;
}

size_t
fifo_avail(const struct fifo *fifo)
{
	return fifo_size(fifo) - fifo_len(fifo);
}

size_t
fifo_esize(const struct fifo *fifo)
{
	return fifo->esize;
}

int
fifo_is_empty(const struct fifo *fifo)
{
	return fifo->in == fifo->out;
}

int
fifo_is_full(const struct fifo *fifo)
{
	return fifo_len(fifo) > fifo->mask;
}

void
fifo_reset(struct fifo *fifo)
{
	fifo->in = fifo->out = 0;
}

void
fifo_reset_out(struct fifo *fifo)
{
	fifo->out = fifo->in;
}

size_t
fifo_in(struct fifo *fifo, const void *src, size_t len)
{

	size_t size, esize, elen, off, n;

	size  = fifo_size(fifo);
	esize = fifo->esize;
	off   = fifo->in & fifo->mask;

	n = fifo_avail(fifo);
	if (len > n) len = n;
	elen = len;

	if (esize != 1) {
		size *= esize;
		len *= esize;
		off *= esize;
	}

	n = min(len, size - off);

	memcpy(fifo->data + off, src, n);
	memcpy(fifo->data, (char *)src + n, len - n);

	fifo->in += elen;
	return elen;
}

size_t
fifo_out_peek(const struct fifo *fifo, void *dst, size_t len)
{
	
	size_t size, esize, elen, off, n;

	size  = fifo_size(fifo);
	esize = fifo->esize;
	off   = fifo->out & fifo->mask;

	n = fifo_len(fifo);
	if (len > n) len = n;
	elen = len;

	if (esize != 1) {
		size *= esize;
		len *= esize;
		off *= esize;
	}

	n = min(len, size - off);

	memcpy(dst, fifo->data + off, n);
	memcpy((char *)dst + n, fifo->data, len - n);
	return elen;
}

size_t
fifo_out_skip(struct fifo *fifo, size_t len)
{
	size_t n;
	
	n = fifo_len(fifo);
	if (len > n) len = n;
	fifo->out += len;
	return len;
}

size_t
fifo_out(struct fifo *fifo, void *dst, size_t len)
{
	fifo_out_peek(fifo, dst, len);
	return fifo_out_skip(fifo, len);
}

#endif
