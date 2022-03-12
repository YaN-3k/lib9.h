#if 0
set -xe

SRC="prog.c"
BIN="prog"

CPPFLAGS="-D_DEFAULT_SOURCE"
CFLAGS="-ggdb -std=c99 -pedantic -Wextra -Wall $CPPFLAGS"
LDFLAGS="-lm -pthread"
CC="cc"

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
typedef long long          vlong;
typedef unsigned long      ulong;
typedef unsigned long long uvlong;

#define nil               (void *)0
#define nelem(x)          (sizeof(x) / sizeof(*x))
#define entry_of(p, t, m) (t *)((char *)(p) - offsetof(t, m))
#define max(x, y)         ((x) > (y) ? (x) : (y))
#define min(x, y)         ((x) < (y) ? (x) : (y))
#define limit(a, x, b)    max(min(x, b), x, a)
#define align(x, align)   (((x) + (align) - 1) & ~((align) - 1))
#define USED(x)           if(x){}else{}
#define TRACE(...)        lib9_trace(__FILE__, __LINE__, __VA_ARGS__)

extern __thread ssize_t lib9_tmp;
extern __thread void *lib9_tmp_p;

#define LIB9_ASSERT(tmp, fn, test, str, ...) \
        (((tmp = fn(__VA_ARGS__)) test) ? tmp : (exits(#fn ": %s", str), tmp))

#define LIB9_ASSERT0(tmp, fn, test, str) \
        (((tmp = fn()) test) ? tmp : (exits(#fn ": %s", str), tmp))

#define ASSERT(fn, ...) \
	LIB9_ASSERT( lib9_tmp, fn, >= 0, strerror(errno), __VA_ARGS__)

#define ASSERT0(fn) \
	LIB9_ASSERT0(lib9_tmp, fn, >= 0, strerror(errno))

#define EASSERT(err, fn) \
	LIB9_ASSERT( lib9_tmp, fn, >= 0, err(lib9_tmp))

#define EASSERT0(err, fn) \
	LIB9_ASSERT0(lib9_tmp, fn, >= 0, err(lib9_tmp))

#define ASSERT_PTR(fn, ...) \
	LIB9_ASSERT( lib9_tmp_p, fn, != nil, strerror(errno), __VA_ARGS__)

#define ASSERT_PTR0(fn) \
	LIB9_ASSERT0(lib9_tmp_p, fn, != nil, strerror(errno))

extern char *argv0;

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

#define LIB9_GETARG(err)                                                      \
        (((*argv)[1] == '\0' && argv[1] == NULL) ?                            \
                (err) :                                                       \
                (brk_ = 1, ((*argv)[1] != '\0') ?                             \
                        (*argv + 1) :                                         \
                        (argc--, *++argv)))


#define ARGF()     LIB9_GETARG((char *)0)
#define EARGF(efn) LIB9_GETARG((efn, abort(), (char *)0))

#define EARGF2NUM(efn, min, max) (estrtonum(EARGF(efn), min, max))
#define EARGF2UINT(efn)          (int)EARGF2NUM(efn, 0, INT_MAX)
#define EARGF2INT(efn)           (int)EARGF2NUM(efn, INT_MIN, INT_MAX)

#define ARGEND }}

void exits(const char *fmt, ...);
void exits2(int status, const char *fmt, ...);
void eprintf(const char *fmt, ...);
long estrtonum(const char *numstr, long minval, long maxval);
void lib9_trace(const char *file, int line, char *fmt, ...);
#ifdef LIB9_MEMC
void *lib9_malloc(size_t size, const char *file, int line);
void *lib9_realloc(void *ptr, size_t size, const char *file, int line);
void *lib9_calloc(size_t nmemb, size_t size, const char *file, int line);
char *lib9_strdup(const char *s, const char *file, int line);
void lib9_free(void *ptr);
void memdump(void);
#else
#define emalloc(...)  ASSERT_PTR(malloc,  __VA_ARGS__)
#define ecalloc(...)  ASSERT_PTR(calloc,  __VA_ARGS__)
#define erealloc(...) ASSERT_PTR(realloc, __VA_ARGS__)
#define estrdup(...)  ASSERT_PTR(strdup,  __VA_ARGS__)
#define memdump()
#endif

#ifdef LIB9_IMPL

__thread ssize_t lib9_tmp;
__thread void *lib9_tmp_p;
char *argv0;

static void lib9_veprintf(const char *fmt, va_list ap);

void
exits(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	lib9_veprintf(fmt, ap);
	va_end(ap);

	memdump();
	exit(fmt && *fmt ? 1 : 0);
}

void
exits2(int status, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	lib9_veprintf(fmt, ap);
	va_end(ap);

	memdump();
	exit(status);
}

void
eprintf(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	lib9_veprintf(fmt, ap);
	va_end(ap);
}

void
lib9_veprintf(const char *fmt, va_list ap)
{
	if (!fmt || !*fmt) return;
	if (argv0 && strncmp(fmt, "usage", strlen("usage")))
		fprintf(stderr, "%s: ", argv0);

	vfprintf(stderr, fmt, ap);

	if (fmt[0] && fmt[strlen(fmt) - 1] == ':') {
		fputc(' ', stderr);
		perror(NULL);
	} else fputc('\n', stderr);
}

void
lib9_trace(const char *file, int line, char *fmt, ...)
{
	va_list ap;

	fprintf(stderr, "%s:%d ", file, line);
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	putchar('\n');
}

long
estrtonum(const char *numstr, long minval, long maxval)
{
	const char *error;
	long num;
	char *ep;

	assert(numstr != NULL);
	assert(minval <= maxval);

	error = NULL;
	num = strtol(numstr, &ep, 10);
	if (numstr == ep || *ep != '\0')
		error = "invalid";
	else if ((num == LLONG_MIN && errno == ERANGE) || num < minval)
		error = "too small";
	else if ((num == LLONG_MAX && errno == ERANGE) || num > maxval)
		error = "too large";
	if (error) exits("%s: %s number", numstr, error);
	return num;
}

#ifdef  LIB9_MEMC

#ifndef LIB9_MEMC_PIPE
#define LIB9_MEMC_PIPE stdout
#endif

#ifdef LIB9_MEMC_MUTEX
static pthread_mutex_t lib9_alloc_mutex = PTHREAD_MUTEX_INITIALIZER;
#define LIB9_MEMC_MUTEX_LOCK()   pthread_mutex_lock(&lib9_alloc_mutex)
#define LIB9_MEMC_MUTEX_UNLOCK() pthread_mutex_unlock(&lib9_alloc_mutex)
#else
#define LIB9_MEMC_MUTEX_LOCK()
#define LIB9_MEMC_MUTEX_UNLOCK()
#endif

typedef struct lib9_alloc {
	const char *file;
	int line;
	size_t size;
	struct lib9_alloc *next, *prev;
} lib9_alloc;

static lib9_alloc *lib9_alloc_head;
static const size_t lib9_alloc_align = align(sizeof(lib9_alloc), 0x10);

void *
lib9_malloc(size_t size, const char *file, int line)
{
	lib9_alloc *alloc;

	alloc = malloc(size + lib9_alloc_align);
	if (!alloc) exits("malloc:");
	alloc->file = file;
	alloc->line = line;
	alloc->size = size;
	LIB9_MEMC_MUTEX_LOCK();
	alloc->next = lib9_alloc_head;
	alloc->prev = NULL;
	if (lib9_alloc_head) alloc->next->prev = alloc;
	lib9_alloc_head = alloc;
	LIB9_MEMC_MUTEX_UNLOCK();
	return (char *)alloc + lib9_alloc_align;
}

void *
lib9_realloc(void *ptr, size_t size, const char *file, int line)
{
	lib9_alloc *alloc;
	void *new;

	if (!ptr) return lib9_malloc(size, file, line);
	else if (size == 0) {
		lib9_free(ptr);
		return NULL;
	}

	alloc = (lib9_alloc *)((char *)ptr - lib9_alloc_align);
	if (size <= alloc->size) return ptr;
#ifdef LIB9_MEMC_PRESERVE
	new = lib9_malloc(size, alloc->file, alloc->line);
#else
	new = lib9_malloc(size, file, line);
#endif
	memcpy(new, ptr, alloc->size);
	lib9_free(ptr);
	return new;
}

void *
lib9_calloc(size_t nmemb, size_t size, const char *file, int line)
{
	void *ptr;

	ptr = lib9_malloc(nmemb * size, file, line);
	memset(ptr, 0, nmemb * size);
	return ptr;
}

char *
lib9_strdup(const char *str, const char *file, int line)
{
	char *new;

	new = lib9_malloc(strlen(str) + 1, file, line);
	strcpy(new, str);
	return new;
}

void
lib9_free(void *ptr)
{
	lib9_alloc *alloc;

	if (ptr == NULL) return;
	alloc = (lib9_alloc *)((char *)ptr - lib9_alloc_align);
	alloc->size = ~alloc->size;
#ifndef LIB9_MEMC_ALL
	LIB9_MEMC_MUTEX_LOCK();
	if (alloc->prev == NULL) {
		assert(lib9_alloc_head == alloc);
		lib9_alloc_head = alloc->next;
	} else alloc->prev->next = alloc->next;
	if (alloc->next) alloc->next->prev = alloc->prev;
	LIB9_MEMC_MUTEX_UNLOCK();
	free(alloc);
#endif
}

void
memdump(void)
{
	lib9_alloc *alloc;
	const char *reason;

	for (alloc = lib9_alloc_head; alloc; alloc = alloc->next) {
		if ((ptrdiff_t)alloc->size >= 0) {
			reason = "LEAKED";
		} else if ((ptrdiff_t)alloc->size < 0) {
			alloc->size = ~alloc->size;
			reason = "FREED ";
		}
		fprintf(
		    LIB9_MEMC_PIPE, "[%s] %s:%d: %lu bytes at %p\n", reason,
		    alloc->file, alloc->line, alloc->size, (void *)(alloc + 1)
		);
	}
}

#endif /* LIB9_MEMC */
#endif /* LIB9_IMPL */

#ifdef LIB9_MEMC
#define malloc(size)          lib9_malloc(size,        __FILE__, __LINE__)
#define calloc(nmemb, size)   lib9_calloc(nmemb, size, __FILE__, __LINE__)
#define realloc(ptr, size)    lib9_realloc(ptr, size,  __FILE__, __LINE__)
#define strdup(str)           lib9_strdup(str,         __FILE__, __LINE__)
#define free                  lib9_free
#define emalloc               malloc
#define ecalloc               calloc
#define erealloc              realloc
#define estrdup               strdup
#endif

#define exit(status) exits2(status, 0);
