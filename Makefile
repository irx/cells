# TUI spreadsheet
# 2021 Maksymilian Mruszczak <u at one u x dot o r g>

.PHONY: clean all

PREFIX = /usr/local
MANPREFIX = ${PREFIX}/man

CC = cc
CXX = c++
CFLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE \
	 -Wno-deprecated-declarations
CXXFLAGS = -pedantic -Wall -I./include
LDFLAGS = # no deps ;P

BIN = cells
HDR = \
      include/Display.h \
      include/Value.h
SRC = \
      src/Display.cc \
      src/main.cc \
      src/Value.cc
OBJ = ${SRC:.cc=.o}

all: ${BIN}

cells: ${OBJ}
	@echo LD $@
	${CXX} -o $@ ${OBJ} ${LDFLAGS}

.c.o: ${HDR}
	@echo CC $<
	@${CC} -c ${CFLAGS} $<

.cc.o: ${HDR}
	@echo CXX $<
	@${CXX} -c ${CXXFLAGS} $< -o $@

clean:
	rm ${BIN} ${OBJ}
