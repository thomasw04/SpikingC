AM_CFLAGS = -Wall -Wextra -Werror -pedantic -std=c11 -Iinclude

if DEBUG
AM_CFLAGS += -g -O0 -fsanitize=address -fsanitize=undefined -fsanitize=leak -fno-omit-frame-pointer -DDEBUG
else
AM_CFLAGS += -O3
endif

bin_PROGRAMS = Patcher SpikingC

SpikingC_SOURCES = \
src/main.c \
src/lif.c \
src/linear.c \
src/model.c \
src/config.c \
src/utility.c \
include/incbin.h \
include/lif.h \
include/linear.h \
include/model.h \
include/config.h \
include/utility.h

SpikingC_LDADD = -lm

Patcher_SOURCES = \
patcher/main.c

Patcher_LDADD = -lm