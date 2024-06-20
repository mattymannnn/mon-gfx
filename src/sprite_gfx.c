#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <libgen.h>
#include "global.h"
#include "gfx.h"
#include "lz.h"

static unsigned char *DoLZCompress(unsigned char *src, int srcSize, int *compressedSize)
{
	unsigned char *compressedData = LZCompress(src, srcSize, compressedSize, 2);
    compressedData[1] = (unsigned char)srcSize;
    compressedData[2] = (unsigned char)(srcSize >> 8);
    compressedData[3] = (unsigned char)(srcSize >> 16);
	return compressedData;
}

static void WriteGbaPalette(unsigned char *dest, struct Palette *palette)
{
	for (int i = 0; i < 16; i++)
	{
		unsigned char red = 0;
		unsigned char green = 0;
		unsigned char blue = 0;

		if (i < palette->numColors)
		{
			red = DOWNCONVERT_BIT_DEPTH(palette->colors[i].red);
			green = DOWNCONVERT_BIT_DEPTH(palette->colors[i].green);
			blue = DOWNCONVERT_BIT_DEPTH(palette->colors[i].blue);
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
		if (pixelColor->blue == palette->colors[i].blue
		&&	pixelColor->green == palette->colors[i].green
		&&	pixelColor->red == palette->colors[i].red)
			return i;
	}

	return 16;
}

static void BuildSpriteBackPal(struct Image *image, struct Palette *palette)
{
	unsigned char *src = image->pixels;

	for (int i = 0; i < 16; i++)
	{
		unsigned char srcPixel = src[palette->colorPos[i] + 64];
		struct Color pixelColor = image->palette.colors[srcPixel];

		palette->colors[i].blue = pixelColor.blue;
		palette->colors[i].green = pixelColor.green;
		palette->colors[i].red = pixelColor.red;
	}
}

static void BuildSpriteTiles(unsigned char *dest, struct Image *image, struct Palette *palette, int outMetatilesWide, int outMetatilesHigh, int offsetX, int offsetY, bool clearPal, int *width, int *height)
{
	#define metatileWidth 8
	#define metatileHeight 8
	#define inMetatilesWide (256 / metatileWidth)
	#define inMmetatilesHigh (64 / metatileHeight)
	unsigned char *src = image->pixels;
	int top, bottom, left, right;

	top = 64;
	bottom = 0;
	left = 64;
	right = 0;

	if (clearPal == true) // reset before building pal
		palette->numColors = 0;

	for (int i = 0; i < outMetatilesHigh; i++)
	{ // Y BLOCK
		int metatileY = (i + offsetY) * (inMetatilesWide * (metatileHeight * metatileWidth));

		for (int j = 0; j < outMetatilesWide; j++)
		{ // X BLOCK
			int metatileX = (j + offsetX) * metatileWidth;

			for (int k = 0; k < 8; k++)
			{ // CURRENT BLOCK Y
				int pixelRow = k * (inMetatilesWide * metatileWidth);

				for (int l = 0; l < 8; l += 2)
				{ // CURRENT BLOCK X
					int pixel = metatileY + metatileX + pixelRow + l;
					unsigned char pixelPair[2];

					for (int m = 0; m < 2; m++)
					{ // CURRENT PIXEL FROM PAIR
						unsigned char srcPixel = src[pixel + m];
						struct Color pixelColor = image->palette.colors[srcPixel];
						int palNum = CheckPalForColor(palette->numColors, &pixelColor, palette);

						if (palNum < 16)
						{ // color already in pal
							pixelPair[m] = palNum;

							if (palNum != 0)
							{ // get WxH of mon inside sprite
								int x = j * 8 + l + m;
								int y = i * 8 + k;
								if (left > x) left = x;
								if (right < x) right = x;
								if (top > y) top = y;
								if (bottom < y) bottom = y;
							}
						}
						else
						{ // color not in pal yet
							palette->colors[palette->numColors].blue = pixelColor.blue;
							palette->colors[palette->numColors].green = pixelColor.green;
							palette->colors[palette->numColors].red = pixelColor.red;
							palette->colorPos[palette->numColors] = pixel + m;
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

	*width = (right - left) + 1;
	*height = (bottom - top) + 1;
}

static void WriteGfxData(char *path, char *speciesName, int widthFont, int heightFront, int widthBack, int heightBack)
{
    // open the file for writing
    FILE *fp = fopen(path, "w");
    if (fp == NULL)
    {
        printf("Error opening the file %s", path);
    }

    // write gfx declarations
	fprintf(fp, "extern const u32 front_tiles_%s[];\n", speciesName);
	fprintf(fp, "extern const u16 front_pal_%s[];\n", speciesName);
	fprintf(fp, "extern const u32 back_tiles_%s[];\n", speciesName);
	fprintf(fp, "extern const u16 back_pal_%s[];\n", speciesName);
	fprintf(fp, "extern const u8 icon_tiles_%s[];\n", speciesName);
	fprintf(fp, "extern const u16 icon_pal_%s[];\n", speciesName);
	// write gfx defines
	fprintf(fp, "#define %s_f_w ", speciesName);
	fprintf(fp, "%d\n", widthFont);
	fprintf(fp, "#define %s_f_h ", speciesName);
	fprintf(fp, "%d\n", heightFront);
	fprintf(fp, "#define %s_b_w ", speciesName);
	fprintf(fp, "%d\n", widthBack);
	fprintf(fp, "#define %s_b_h ", speciesName);
	fprintf(fp, "%d\n", heightBack);

    // close the file
    fclose(fp);
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

void WriteImage(char *imgFrontOutputPath, char *imgBackOutputPath, char *palFrontOutputPath, char *palBackOutputPath, char *gfxDataOutputPath, char *speciesName, struct Image *image, struct Palette *palette)
{
	#define NUM_TILES (8 * 8) // 64px * 64px
	#define TILE_SIZE (4 * 8) // bitdepth * 8
	#define BUFFER_SIZE (NUM_TILES * TILE_SIZE)

	int compressedSize, widthFont, heightFront, widthBack, heightBack;
	unsigned char *compressedData;

	if (image->width != 256)
		FATAL_ERROR("The width in pixels (%d) isn't 256.\n", image->width);

	if (image->height != 64)
		FATAL_ERROR("The height in pixels (%d) isn't 64.\n", image->height);

	unsigned char *buffer = malloc(BUFFER_SIZE * 2);// double size for 2 frames

	if (buffer == NULL)
		FATAL_ERROR("Failed to allocate memory for pixels.\n");

	// get front sprite tiles & pal
	BuildSpriteTiles(buffer, image, palette, 8, 8, 0, 0, true, &widthFont, &heightFront);

	// write front sprite tiles
	compressedData = DoLZCompress(buffer, BUFFER_SIZE, &compressedSize);
	WriteImageBinary(imgFrontOutputPath, compressedData, compressedSize);

	// get back sprite tiles
	BuildSpriteTiles(buffer, image, palette, 8, 8, 16, 0, false,  &widthBack, &heightBack);

	// write back sprite tiles
	compressedData = DoLZCompress(buffer, BUFFER_SIZE, &compressedSize);
	WriteImageBinary(imgBackOutputPath, compressedData, compressedSize);

	// write front sprite pal
	WriteGbaPalette(buffer, palette);
	WriteImageBinary(palFrontOutputPath, buffer, 32);

	// get back sprite pal
	BuildSpriteBackPal(image, palette);

	// write back sprite pal
	WriteGbaPalette(buffer, palette);
	WriteImageBinary(palBackOutputPath, buffer, 32);

	// write gfx data
	WriteGfxData(gfxDataOutputPath, speciesName, widthFont, heightFront, widthBack, heightBack);

	free(buffer);
}

void FreeImage(struct Image *image)
{
	free(image->pixels);
	image->pixels = NULL;
}
