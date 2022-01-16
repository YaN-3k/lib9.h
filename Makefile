VERSION = 1.0

CPPFLAGS = -D_DEFAULT_SOURCE -DVERSION=\"${VERSION}\"
CFLAGS   = -ggdb -ansi -pedantic -Wextra -Wall ${CPPFLAGS}
LDFLAGS  =

SRC = main.c
OBJ = ${SRC:.c=.o}

all: options main

options:
	@echo main build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

.c.o:
	${CC} -c ${CFLAGS} $<

main: ${OBJ}
	${CC} -o $@ ${OBJ} ${LDFLAGS}

clean:
	rm -f main ${OBJ}

.PHONY: all options clean
