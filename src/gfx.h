#ifndef GFX_H
#define GFX_H

#include <stdint.h>
#include <stdbool.h>

#define GET_GBA_PAL_RED(x)   (((x) >>  0) & 0x1F)
#define GET_GBA_PAL_GREEN(x) (((x) >>  5) & 0x1F)
#define GET_GBA_PAL_BLUE(x)  (((x) >> 10) & 0x1F)
#define SET_GBA_PAL(r, g, b) (((b) << 10) | ((g) << 5) | (r))
#define UPCONVERT_BIT_DEPTH(x) (((x) * 255) / 31)
#define DOWNCONVERT_BIT_DEPTH(x) ((x) / 8)

struct Color {
	unsigned char red;
	unsigned char green;
	unsigned char blue;
};

struct Palette {
	struct Color colors[256];
	int colorPos[16];
	int numColors;
};

struct Image {
	int width;
	int height;
	int bitDepth;
	unsigned char *pixels;
	bool hasPalette;
	struct Palette palette;
};

void FreeImage(struct Image *image);

#endif // GFX_H
