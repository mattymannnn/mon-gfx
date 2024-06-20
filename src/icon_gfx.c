#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "global.h"
#include "gfx.h"

static void WriteGbaPalette(unsigned char *dest, struct Palette *palette)
{
	for (int i = 0; i < 16; i++)
	{
		unsigned char red = 0;
		unsigned char green = 0;
		unsigned char blue = 0;

		if (i < palette->numColors)
		{
			red = palette->colors[i].red;
			green = palette->colors[i].green;
			blue = palette->colors[i].blue;
		}

		uint16_t paletteEntry = SET_GBA_PAL(red, green, blue);

		*dest++ = (paletteEntry & 0xFF);
		*dest++ = (paletteEntry >> 8);
	}
}

static int CheckPalForColor(int palSize, struct Color *pixelColor, struct Palette *palette)
{
	for (int i = 0; i < palSize; i++)
	{
		if (DOWNCONVERT_BIT_DEPTH(pixelColor->blue) == palette->colors[i].blue
		&&	DOWNCONVERT_BIT_DEPTH(pixelColor->green) == palette->colors[i].green
		&&	DOWNCONVERT_BIT_DEPTH(pixelColor->red) == palette->colors[i].red)
			return i;
	}

	return 16;
}

static void BuildSpriteTiles(unsigned char *dest, struct Image *image, struct Palette *palette, int outMetatilesWide, int outMetatilesHigh)
{
	#define metatileWidth 8
	#define metatileHeight 8
	#define inMetatilesWide (32 / metatileWidth)
	#define inMmetatilesHigh (32 / metatileHeight)
	unsigned char *src = image->pixels;

	for (int i = 0; i < outMetatilesHigh; i++)
	{
		int metatileY = i * (inMetatilesWide * (metatileHeight * metatileWidth));

		for (int j = 0; j < outMetatilesWide; j++)
		{
			int metatileX = j * metatileWidth;

			for (int k = 0; k < 8; k++)
			{
				int pixelRow = k * (inMetatilesWide * metatileWidth);

				for (int l = 0; l < 8; l += 2)
				{
					int pixel = metatileY + metatileX + pixelRow + l;
					unsigned char pixelPair[2];

					for (int m = 0; m < 2; m++)
					{
						unsigned char srcPixel = src[pixel + m];
						struct Color pixelColor = image->palette.colors[srcPixel];
						int palNum = CheckPalForColor(palette->numColors, &pixelColor, palette);

						if (palNum < 16)
						{
							pixelPair[m] = palNum;
						}
						else
						{
							palette->colors[palette->numColors].blue = DOWNCONVERT_BIT_DEPTH(pixelColor.blue);
							palette->colors[palette->numColors].green = DOWNCONVERT_BIT_DEPTH(pixelColor.green);
							palette->colors[palette->numColors].red = DOWNCONVERT_BIT_DEPTH(pixelColor.red);
							pixelPair[m] = palette->numColors;
							if (++palette->numColors > 16)
								FATAL_ERROR("Pal is bigger than 16 colors\n");
						}
					}

					*dest++ = (pixelPair[1] << 4) | pixelPair[0]; // convert to 4bit - 2 pixels in a byte
				}
			}
		}
	}
}

static void WriteImageBinary(char *path, void *buffer, int bufferSize)
{
	FILE *fp = fopen(path, "wb");

	if (fp == NULL)
		FATAL_ERROR("Failed to open \"%s\" for writing.\n", path);

	if (fwrite(buffer, bufferSize, 1, fp) != 1)
		FATAL_ERROR("Failed to write to \"%s\".\n", path);

	fclose(fp);
}

void WriteImage(char *iconOutputPath, char *palOutputPath, struct Image *image, struct Palette *palette)
{
	#define NUM_TILES (4 * 4) // 32px * 32px
	#define TILE_SIZE (4 * 8) // bitdepth * 8
	#define BUFFER_SIZE (NUM_TILES * TILE_SIZE)

	if (image->width != 32)
		FATAL_ERROR("The width in pixels (%d) isn't 32.\n", image->width);

	if (image->height != 32)
		FATAL_ERROR("The height in pixels (%d) isn't 64.\n", image->height);

	unsigned char *buffer = malloc(BUFFER_SIZE);

	if (buffer == NULL)
		FATAL_ERROR("Failed to allocate memory for pixels.\n");

	BuildSpriteTiles(buffer, image, palette, 4, 4);
	WriteImageBinary(iconOutputPath, buffer, BUFFER_SIZE);
	WriteGbaPalette(buffer, palette);
	WriteImageBinary(palOutputPath, buffer, 32);
	free(buffer);
}

void FreeImage(struct Image *image)
{
	free(image->pixels);
	image->pixels = NULL;
}
