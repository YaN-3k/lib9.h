#if 0
#
# Plan 9 like libc.h
#

set -xe

VERSION=1.0
SRC=main.c
BIN=main

if [ "$1" = clean ]; then
	for i in $SRC; do
		rm -f "${i%.c}.o"
	done
	rm -f "$BIN"
	exit
fi

CPPFLAGS="-D_DEFAULT_SOURCE -DVERSION=\"$VERSION\""
CFLAGS="-ggdb -ansi -pedantic -Wextra -Wall $CPPFLAGS"
LDFLAGS=
CC=cc

for i in $SRC; do
	out=${i%.c}.o
	OBJ="$OBJ $out"
	$CC -c $CFLAGS $i -o $out
done

$CC $LDFLAGS $OBJ -o "$BIN"

exit
#endif
#ifndef LIBC_H__
#ifdef INCLUDE_GUARD
#define LIBC_H__
#endif

#include <assert.h>
#include <errno.h>
#include <math.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

typedef unsigned char  uchar;
typedef unsigned short ushort;
typedef unsigned int   uint;
typedef unsigned long  ulong;

extern void die(const char *fmt, ...);
extern void *emalloc(size_t size);
extern void *erealloc(void *ptr, size_t size);
extern void *ecalloc(size_t nmemb, size_t size);
extern char *estrdup(const char *s);

#ifdef MEMCHECK
extern void *mcmalloc(size_t size, const char *file, int line);
extern void *mcrealloc(void *ptr, size_t size, const char *file, int line);
extern void *mccalloc(size_t nmemb, size_t size, const char *file, int line);
extern char *mcstrdup(const char *s, const char *file, int line);
extern void mcfree(void *ptr);
extern void memdump(void);
#else
#define memdump()
#endif

#ifdef IMPLEMENTATION

void
die(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	if(fmt[0] && fmt[strlen(fmt) - 1] == ':') {
		fputc(' ', stderr);
		perror(NULL);
	} else fputc('\n', stderr);
	exit(1);
}

void *
emalloc(size_t size)
{
	void *p;

	if(!(p = malloc(size))) die("malloc:");
	return p;
}

void *
erealloc(void *ptr, size_t size)
{
	void *p;

	if(!(p = realloc(ptr, size))) die("realloc:");
	return p;
}

void *
ecalloc(size_t nmemb, size_t size)
{
	void *p;

	if(!(p = calloc(nmemb, size))) die("calloc:");
	return p;
}

char *
estrdup(const char *str)
{
	if(!(str = strdup(str))) die("strdup:");
	return (char *)str;
}

#define malloc  emalloc
#define calloc  ecalloc
#define realloc erealloc
#define strdup  estrdup

#ifdef  MEMCHECK

#undef malloc
#undef calloc
#undef realloc
#undef strdup

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

void *
mcmalloc(size_t size, const char *file, int line)
{
	alloc_info *alloc;

	alloc = malloc(size + sizeof *alloc_head);
	if (!alloc) {
#ifdef MEMCHECK_RETURN
		return alloc;
#else
		fprintf(stderr, "malloc: %s\n", strerror(errno));
		memdump();
		exit(1);
#endif
	}
	alloc->file = file;
	alloc->line = line;
	alloc->size = size;
	alloc->next = alloc_head;
	alloc->prev = NULL;
	if (alloc_head) alloc->next->prev = alloc;
	alloc_head = alloc;
	return alloc + 1;
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

	alloc = (alloc_info *)ptr - 1;
	if (size <= alloc->size) return ptr;
#ifdef MEMCHECK_PRESERVE_FILELINE
	new = mcmalloc(size, alloc->file, alloc->line);
#else
	new = mcmalloc(size, file, line);
#endif
#ifndef MEMCHECK_RETURN
	if (!new) return new;
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
	alloc = (alloc_info *)ptr - 1;
	alloc->size = ~alloc->size;
#ifndef MEMCHECK_SHOWALL
	if (alloc->prev == NULL) {
		assert(alloc_head == alloc);
		alloc_head = alloc->next;
	} else alloc->prev->next = alloc->next;
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
		    MEMCHECK_PIPE, "[%s] %s:%4d: %lu bytes at %p\n", reason,
		    alloc->file, alloc->line, alloc->size, (void *)(alloc + 1)
		);
	}
}

#define malloc(size)         mcmalloc(size,        __FILE__, __LINE__)
#define calloc(nmemb, size)  mccalloc(nmemb, size, __FILE__, __LINE__)
#define realloc(ptr, size)   mcrealloc(ptr, size,  __FILE__, __LINE__)
#define strdup(str)          mcstrdup(str,         __FILE__, __LINE__)
#define free(ptr)            mcfree(ptr)

#endif /* MEMCHECK */
#endif /* IMPLEMENTATION */

extern char *argv0;

/* use main(int argc, char *argv[]) */
#define ARGBEGIN                                                              \
        char argc_;                                                           \
        char **argv_;                                                         \
        int brk_;                                                             \
        int i_;                                                               \
        for (argv0 = *argv, argv++, argc--;                                   \
             *argv && (*argv)[0] == '-' && (*argv)[1];                        \
             argc--, argv++) {                                                \
                if ((*argv)[1] == '-' && (*argv)[2] == '\0') {                \
                        argv++; argc--; break;                                \
                }                                                             \
                for (i_ = 1, brk_ = 0, argv_ = argv;                          \
                     (*argv)[i_] && !brk_; i_++) {                            \
                        if (argv != argv_) break;                             \
                        argc_ = (*argv)[i_];                                  \
                        switch (argc_)
                        /*
                        case 'a':
                                opt_a = ARGF();
                                break;
                        case 'b':
                                opt_b = EARGF(usage());
                                break;
                         */
#define ARGC() argc_

#define ARGF()                                                                \
        (((*argv)[i_+1] == '\0' && argv[1] == NULL) ?                         \
                (char *)0 :                                                   \
                (brk_ = 1, ((*argv)[i_+1] != '\0') ?                          \
                        (&(*argv)[i_+1]) :                                    \
                        (argc--, *++argv)))

#define EARGF(x)                                                              \
        (((*argv)[i_+1] == '\0' && argv[1] == NULL) ?                         \
                ((x), abort(), (char *)0) :                                   \
                (brk_ = 1, ((*argv)[i_+1] != '\0') ?                          \
                        (&(*argv)[i_+1]) :                                    \
                        (argc--, *++argv)))


#define ARGEND }}

#endif /* LIBC_H__ */
