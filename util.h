/*
 * #define:
 * UTIL_IMPL -- include implementation (https://github.com/nothings/stb)
 * UTIL_INCLUDE -- self contain headers (by default use plan9 style)
 * UTIL_MEMCHECK -- simple leak checking
 * UTIL_MEMCHECK_RETURN -- return NULL instead calling exit
 * UTIL_MEMCHECK_XPREFIX -- do not shadow malloc, realloc, calloc, etc
 * UTIL_MEMCHECK_PRESERVE_FILELINE -- do not override file/line when realloc
 * TODO:
 * memcheck thread safety
 * mccalloc call calloc instead mcmalloc
 */

#ifndef UTIL_H__
#ifdef UTIL_INCLUDE
#define UTIL_H__
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef UTIL_MEMCHECK
#include <assert.h>
#include <stddef.h>
#endif
#endif

extern void die(const char *fmt, ...);
extern void *xmalloc(size_t size);
extern void *xrealloc(void *ptr, size_t size);
extern void *xcalloc(size_t nmemb, size_t size);
extern char *xstrdup(const char *s);

#ifdef UTIL_MEMCHECK
extern void *mcmalloc(size_t size, const char *file, int line);
extern void *mcrealloc(void *ptr, size_t size, const char *file, int line);
extern void *mccalloc(size_t nmemb, size_t size, const char *file, int line);
extern char *mcstrdup(const char *s, const char *file, int line);
extern void mcfree(void *ptr);
extern void dumpmem(void);
#endif

#ifdef UTIL_IMPL

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
xmalloc(size_t size)
{
	void *p;

	if(!(p = malloc(size))) die("malloc:");
	return p;
}

void *
xrealloc(void *ptr, size_t size)
{
	void *p;

	if(!(p = realloc(ptr, size))) die("realloc:");
	return p;
}

void *
xcalloc(size_t nmemb, size_t size)
{
	void *p;

	if(!(p = calloc(nmemb, size))) die("calloc:");
	return p;
}

char *
xstrdup(const char *str)
{
	if(!(str = strdup(str))) die("strdup:");
	return (char *)str;
}

#ifdef  UTIL_MEMCHECK

#ifndef UTIL_MEMCHECK_PIPE
#define UTIL_MEMCHECK_PIPE stdout
#endif

typedef struct alloc_info {
	const char *file;
	int line;
	size_t size;
	struct alloc_info *next, *prev;
} alloc_info;

static void alloc_info_print(const char *reason, alloc_info *alloc);

static alloc_info *alloc_head;

void *
mcmalloc(size_t size, const char *file, int line)
{
	alloc_info *alloc;

	alloc = malloc(size + sizeof *alloc_head);
	if (!alloc) {
#ifdef UTIL_MEMCHECK_RETURN
		return alloc;
#else
		fprintf(stderr, "malloc: %s\n", strerror(errno));
		dumpmem();
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
#ifdef UTIL_MEMCHECK_PRESERVE_FILELINE
	new = mcmalloc(size, alloc->file, alloc->line);
#else
	new = mcmalloc(size, file, line);
#endif
#ifndef UTIL_MEMCHECK_RETURN
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
#ifndef UTIL_MEMCHECK_SHOWALL
	if (alloc->prev == NULL) {
		assert(alloc_head == alloc);
		alloc_head = alloc->next;
	} else alloc->prev->next = alloc->next;
	if (alloc->next) alloc->next->prev = alloc->prev;
	free(alloc);
#endif
}

void
alloc_info_print(const char *reason, alloc_info *alloc)
{
	fprintf(UTIL_MEMCHECK_PIPE, "[%s] %s:%4d: %lu bytes at %p\n", reason,
	        alloc->file, alloc->line, alloc->size, (void *)(alloc + 1));
}

void
dumpmem(void)
{
	alloc_info *alloc;

	for (alloc = alloc_head; alloc; alloc = alloc->next) {
		if ((ptrdiff_t)alloc->size >= 0) {
			alloc_info_print("LEAKED", alloc);
		} else if ((ptrdiff_t)alloc->size < 0) {
			alloc->size = ~alloc->size;
			alloc_info_print("FREED ", alloc);
		}
	}
}

#endif /* UTIL_MEMCHECK */
#endif /* UTIL_IMPL */

#if defined(UTIL_MEMCHECK)
#define malloc(size)         mcmalloc(size,        __FILE__, __LINE__)
#define calloc(nmemb, size)  mccalloc(nmemb, size, __FILE__, __LINE__)
#define realloc(ptr, size)   mcrealloc(ptr, size,  __FILE__, __LINE__)
#define strdup(str)          mcstrdup(str,         __FILE__, __LINE__)
#define free(ptr)            mcfree(ptr)
#elif !defined(UTIL_XPREFIX)
#define malloc  xmalloc
#define calloc  xcalloc
#define realloc xrealloc
#define strdup  xstrdup
#endif

#ifndef UTIL_MEMCHECK
#define dumpmem()
#endif

#endif
