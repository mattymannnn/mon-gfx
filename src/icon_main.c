#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "global.h"
#include "gfx.h"
#include "convert_png.h"

void WriteImage(char *iconOutputPath, char *palOutputPath, struct Image *image, struct Palette *palette);

static void HandleMonGfx(char *inputPath, char *iconOutputPath, char *palOutputPath)
{
    struct Image image;
    struct Palette palette = {0};

    image.bitDepth = 8;
    ReadPng(inputPath, &image);
    ReadPngPalette(inputPath, &image.palette);
    WriteImage(iconOutputPath, palOutputPath, &image, &palette);
    FreeImage(&image);
}

int main(int argc, char **argv)
{
    if (argc < 3)
        FATAL_ERROR("Usage: monicon input output\n");

    const char *iconFileExtension = ".icon.4bpp";
    const char *palFileExtension = ".icon.pal";
    char *iconOutputPath;
    char *palOutputPath;
    char *inputPath = argv[1];
    char *outputPath = argv[2];

    size_t iconOutputPathSize = strlen(outputPath) + strlen(iconFileExtension) + 1;
    size_t palOutputPathSize = strlen(outputPath) + strlen(palFileExtension) + 1;

// alloc icon
    iconOutputPath = malloc(iconOutputPathSize);
    if (iconOutputPath == NULL)
        FATAL_ERROR("Failed to allocate memory for front tiles output path.\n");
    strcpy(iconOutputPath, outputPath);
    strncat(iconOutputPath, iconFileExtension, strlen(iconFileExtension));

// alloc pal
    palOutputPath = malloc(palOutputPathSize);
    if (palOutputPath == NULL)
        FATAL_ERROR("Failed to allocate memory for front tiles output path.\n");
    strcpy(palOutputPath, outputPath);
    strncat(palOutputPath, palFileExtension, strlen(palFileExtension));

    HandleMonGfx(inputPath, iconOutputPath, palOutputPath);
    free(iconOutputPath);
    free(palOutputPath);

    return 0;
}
