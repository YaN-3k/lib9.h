#if 0
# Please don't ask me why this exists

set -xe

SRC=main.c
BIN=main

CPPFLAGS="-D_DEFAULT_SOURCE"
CFLAGS="-std=c99 -ggdb -pedantic -Wextra -Wall $CPPFLAGS"
LDFLAGS="-lm -pthread"
CC=cc

if [ "$1" = clean ]; then
	for f in $SRC; do
		rm -f "${f%.c}.o"
	done
	rm -f "$BIN"
	[ $# -eq 1 ] && exit 0
fi

for f in $SRC; do
	out=${f%.c}.o
	OBJ="$OBJ $out"
	if [ ! -e "$out" ] || [ "$(find -L "$f" -prune -newer "$out")" ]; then
		$CC -c $CFLAGS $f -o "$out"
	fi
done

[ "$OBJ" ] && $CC $LDFLAGS $OBJ -o "$BIN"

exit 0
#endif
#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

typedef unsigned char      uchar;
typedef signed char        schar;
typedef unsigned short     ushort;
typedef unsigned int       uint;
typedef unsigned long      ulong;
typedef	long long          vlong;
typedef	unsigned long long uvlong;

#define MAX(x, y)         ((x) > (y) ? (x) : (y))
#define MIN(x, y)         ((x) < (y) ? (x) : (y))
#define LIMIT(a, x, b)    MAX(MIN(x, b), x, a)
#define SIZE(x)           (sizeof(x) / sizeof(*x))
#define ENTRY_OF(p, t, m) (t *)((char *)(p) - offsetof(t, m))
#define MAX_ALIGN         0x10
#define ALIGN(x, align)   (((x) + (align) - 1) & ~((align) - 1))
#define SET(x)            (x) = 0
#define USED(x)           (void)(x)
#define TRY(fn, ...)      if (fn(__VA_ARGS__)) eprintf(#fn ":")

#ifdef NDEBUG
#define TRACE(fmt, ...)
#else
#define TRACE(fmt, ...) \
        printf("%s:%d " fmt "\n", __FILE__, __LINE__, __VA_ARGS__)
#endif

char *argv0;

/* use main(int argc, char *argv[]) */
#define ARGBEGIN                                                              \
        char argc_;                                                           \
        char **argv_;                                                         \
        int brk_;                                                             \
        for (argv0 = *argv, argv++, argc--;                                   \
             *argv && (*argv)[0] == '-' && (*argv)[1];                        \
             argc--, argv++) {                                                \
                if ((*argv)[1] == '-' && (*argv)[2] == '\0') {                \
                        argv++; argc--; break;                                \
                }                                                             \
                for (brk_ = 0, ++*argv, argv_ = argv;                         \
                     **argv && !brk_; ++*argv) {                              \
                        if (argv != argv_) break;                             \
                        argc_ = **argv;                                       \
                        switch (argc_)
                        /*
                        case 'a':
                                opt_a = EARGF(usage());
                                break;
                         */

#define ARGC() argc_

/* handles obsolete -NUM syntax */
#define ARGNUM                                                                \
                        case '0':                                             \
                        case '1':                                             \
                        case '2':                                             \
                        case '3':                                             \
                        case '4':                                             \
                        case '5':                                             \
                        case '6':                                             \
                        case '7':                                             \
                        case '8':                                             \
                        case '9'

#define ARGNUMF() (brk_ = 1, (int)estrtonum(*argv, 0, INT_MAX))

#define GETARG_(err)                                                          \
        (((*argv)[1] == '\0' && argv[1] == NULL) ?                            \
                (err) :                                                       \
                (brk_ = 1, ((*argv)[1] != '\0') ?                             \
                        (*argv + 1) :                                         \
                        (argc--, *++argv)))


#define ARGF()     GETARG_((char *)0)
#define EARGF(efn) GETARG_((efn, abort(), (char *)0))

#define EARGF2NUM(efn, min, max) (estrtonum(EARGF(efn), min, max))
#define EARGF2UINT(efn)          (int)EARGF2NUM(efn, 0, INT_MAX)
#define EARGF2INT(efn)           (int)EARGF2NUM(efn, INT_MIN, INT_MAX)

#define ARGEND }}

#ifdef MEMCHECK_MUTEX
#define MEMCHECK
#endif

#ifdef MEMCHECK
extern void *mcmalloc(size_t size, const char *file, int line);
extern void *mcrealloc(void *ptr, size_t size, const char *file, int line);
extern void *mccalloc(size_t nmemb, size_t size, const char *file, int line);
extern char *mcstrdup(const char *s, const char *file, int line);
extern void mcfree(void *ptr);
extern void memdump(void);
#else
extern void eprintf(const char *fmt, ...);
extern void enprintf(int status, const char *fmt, ...);
extern void weprintf(const char *fmt, ...);
extern long long estrtonum(const char *numstr, long long minval, long long maxval);
extern void *emalloc(size_t size);
extern void *erealloc(void *ptr, size_t size);
extern void *ecalloc(size_t nmemb, size_t size);
extern char *estrdup(const char *s);
#define memdump()
#endif

#ifdef LIBC_IMPL

static void xvprintf_(const char *fmt, va_list ap);

void
eprintf(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	xvprintf_(fmt, ap);
	va_end(ap);

	exit(1);
}

void
enprintf(int status, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	xvprintf_(fmt, ap);
	va_end(ap);

	exit(status);
}

void
weprintf(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	xvprintf_(fmt, ap);
	va_end(ap);
}

void
xvprintf_(const char *fmt, va_list ap)
{
	if (argv0 && strncmp(fmt, "usage", strlen("usage")))
		fprintf(stderr, "%s: ", argv0);

	vfprintf(stderr, fmt, ap);

	if (fmt[0] && fmt[strlen(fmt) - 1] == ':') {
		fputc(' ', stderr);
		perror(NULL);
	} else fputc('\n', stderr);
}

long long
estrtonum(const char *numstr, long long minval, long long maxval)
{
	long long ll;
	char *ep;
	const char *error;

	assert(numstr != NULL);
	assert(minval <= maxval);

	error = NULL;
	ll = strtoll(numstr, &ep, 10);
	if (numstr == ep || *ep != '\0')
		error = "invalid";
	else if ((ll == LLONG_MIN && errno == ERANGE) || ll < minval)
		error = "too small";
	else if ((ll == LLONG_MAX && errno == ERANGE) || ll > maxval)
		error = "too large";
	if (error) eprintf("%s: %s number", numstr, error);
	return ll;
}

void *
emalloc(size_t size)
{
	void *p;

	if (!(p = malloc(size))) eprintf("malloc:");
	return p;
}

void *
erealloc(void *ptr, size_t size)
{
	void *p;

	if (!(p = realloc(ptr, size))) eprintf("realloc:");
	return p;
}

void *
ecalloc(size_t nmemb, size_t size)
{
	void *p;

	if (!(p = calloc(nmemb, size))) eprintf("calloc:");
	return p;
}

char *
estrdup(const char *str)
{
	if (!(str = strdup(str))) eprintf("strdup:");
	return (char *)str;
}

#ifdef  MEMCHECK

#ifdef MEMCHECK_MUTEX
#define MEMCHECK_MUTEX_LOCK()   pthread_mutex_lock(&alloc_mutex)
#define MEMCHECK_MUTEX_UNLOCK() pthread_mutex_unlock(&alloc_mutex)
#else
#define MEMCHECK_MUTEX_LOCK()
#define MEMCHECK_MUTEX_UNLOCK()
#endif

#ifndef MEMCHECK_PIPE
#define MEMCHECK_PIPE stdout
#endif

typedef struct alloc_info {
	const char *file;
	int line;
	size_t size;
	struct alloc_info *next, *prev;
} alloc_info;

static alloc_info *alloc_head;
#ifdef MEMCHECK_MUTEX
static pthread_mutex_t alloc_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

void *
mcmalloc(size_t size, const char *file, int line)
{
	alloc_info *alloc;
	size_t alloc_align;

	alloc_align = ALIGN(sizeof(*alloc), MAX_ALIGN);
	alloc = malloc(size + alloc_align);
	if (!alloc) {
		fprintf(stderr, "malloc: %s\n", strerror(errno));
		memdump();
		exit(1);
	}
	alloc->file = file;
	alloc->line = line;
	alloc->size = size;
	MEMCHECK_MUTEX_LOCK();
	alloc->next = alloc_head;
	alloc->prev = NULL;
	if (alloc_head) alloc->next->prev = alloc;
	alloc_head = alloc;
	MEMCHECK_MUTEX_UNLOCK();
	return (char *)alloc + alloc_align;
}

void *
mcrealloc(void *ptr, size_t size, const char *file, int line)
{
	alloc_info *alloc;
	void *new;

	if (!ptr) return mcmalloc(size, file, line);
	else if (size == 0) {
		mcfree(ptr);
		return NULL;
	}

	alloc = (alloc_info *)((char *)ptr - ALIGN(sizeof(*alloc), MAX_ALIGN));
	if (size <= alloc->size) return ptr;
#ifdef MEMCHECK_PRESERVE_FILELINE
	new = mcmalloc(size, alloc->file, alloc->line);
#else
	new = mcmalloc(size, file, line);
#endif
	memcpy(new, ptr, alloc->size);
	mcfree(ptr);
	return new;
}

void *
mccalloc(size_t nmemb, size_t size, const char *file, int line)
{
	void *ptr;

	ptr = mcmalloc(nmemb * size, file, line);
	memset(ptr, 0, nmemb * size);
	return ptr;
}

char *
mcstrdup(const char *str, const char *file, int line)
{
	char *new;

	new = mcmalloc(strlen(str) + 1, file, line);
	strcpy(new, str);
	return new;
}

void
mcfree(void *ptr)
{
	alloc_info *alloc;

	if (ptr == NULL) return;
	alloc = (alloc_info *)((char *)ptr - ALIGN(sizeof(*alloc), MAX_ALIGN));
	alloc->size = ~alloc->size;
#ifndef MEMCHECK_SHOWALL
	MEMCHECK_MUTEX_LOCK();
	if (alloc->prev == NULL) {
		assert(alloc_head == alloc);
		alloc_head = alloc->next;
	} else alloc->prev->next = alloc->next;
	MEMCHECK_MUTEX_UNLOCK();
	if (alloc->next) alloc->next->prev = alloc->prev;
	free(alloc);
#endif
}

void
memdump(void)
{
	alloc_info *alloc;
	const char *reason;

	for (alloc = alloc_head; alloc; alloc = alloc->next) {
		if ((ptrdiff_t)alloc->size >= 0) {
			reason = "LEAKED";
		} else if ((ptrdiff_t)alloc->size < 0) {
			alloc->size = ~alloc->size;
			reason = "FREED ";
		}
		fprintf(
		    MEMCHECK_PIPE, "[%s] %s:%d: %lu bytes at %p\n", reason,
		    alloc->file, alloc->line, alloc->size, (void *)(alloc + 1)
		);
	}
}

#endif /* MEMCHECK */
#endif /* LIBC_IMPLEMENTATION */

#ifdef MEMCHECK
#define emalloc(size)         mcmalloc(size,        __FILE__, __LINE__)
#define ecalloc(nmemb, size)  mccalloc(nmemb, size, __FILE__, __LINE__)
#define erealloc(ptr, size)   mcrealloc(ptr, size,  __FILE__, __LINE__)
#define estrdup(str)          mcstrdup(str,         __FILE__, __LINE__)
#define free    mcfree
#define malloc  emalloc
#define calloc  ecalloc
#define realloc erealloc
#define strdup  estrdup
#endif

/* vim: set noet ts=8: */
