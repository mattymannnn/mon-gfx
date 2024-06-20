CC = gcc

CFLAGS = -Wall -Wextra -Werror -Wno-sign-compare -std=c11 -O2 -DPNG_SKIP_SETJMP_CHECK
CFLAGS += $(shell pkg-config --cflags libpng)

LIBS = -lpng -lz
LDFLAGS += $(shell pkg-config --libs-only-L libpng)

SPRITE_SRCS = src/sprite_main.c src/sprite_gfx.c src/convert_png.c src/lz.c
ICON_SRCS = src/icon_main.c src/icon_gfx.c src/convert_png.c

ifeq ($(OS),Windows_NT)
EXE := .exe
else
EXE :=
endif

.PHONY: all clean

all: monsprite$(EXE) monicon$(EXE)
	@:

monsprite$(EXE): $(SPRITE_SRCS) src/convert_png.h src/gfx.h src/global.h src/lz.h
	$(CC) $(CFLAGS) $(SPRITE_SRCS) -o $@ $(LDFLAGS) $(LIBS)

monicon$(EXE): $(ICON_SRCS) src/convert_png.h src/gfx.h src/global.h
	$(CC) $(CFLAGS) $(ICON_SRCS) -o $@ $(LDFLAGS) $(LIBS)

clean:
	$(RM) monsprite monsprite.exe monicon monicon.exe
