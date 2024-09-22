AM_CFLAGS = -Wall -Wextra -Werror -pedantic -std=gnu99 -Iinclude

if DEBUG
AM_CFLAGS += -g -O0 -fsanitize=address -fsanitize=undefined -fsanitize=leak -fno-omit-frame-pointer
else
AM_CFLAGS += -O3
endif

bin_PROGRAMS = SpikingC

SpikingC_SOURCES = \
src/main.c \
src/lif.c \
src/linear.c \
src/model.c \
src/snnconfig.c \
src/utility.c \
include/lif.h \
include/linear.h \
include/model.h \
include/snnconfig.h \
include/utility.h

SpikingC_LDADD = -lm