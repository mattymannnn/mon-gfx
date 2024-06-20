#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <libgen.h>
#include "global.h"
#include "gfx.h"
#include "convert_png.h"

void WriteImage(char *imgFrontOutputPath, char *imgBackOutputPath, char *palFrontOutputPath, char *palBackOutputPath, char *gfxDataOutputPath, char *speciesName, struct Image *image, struct Palette *palette);

static void HandleMonGfx(char *inputPath, char *imgFrontOutputPath, char *imgBackOutputPath, char *palFrontOutputPath, char *palBackOutputPath, char *gfxDataOutputPath, char *speciesName)
{
    struct Image image;
    struct Palette palette = {0};

    image.bitDepth = 8;
    ReadPng(inputPath, &image);
    ReadPngPalette(inputPath, &image.palette);
    WriteImage(imgFrontOutputPath, imgBackOutputPath, palFrontOutputPath, palBackOutputPath, gfxDataOutputPath, speciesName, &image, &palette);
    FreeImage(&image);
}

int main(int argc, char **argv)
{
    if (argc < 3)
        FATAL_ERROR("Usage: mongfx input output\n");

    const char *imgFrontFileExtension = ".front.4bpp";
    const char *imgBackFileExtension = ".back.4bpp";
    const char *palFrontFileExtension = ".front.pal";
    const char *palBackFileExtension = ".back.pal";
    const char *gfxDataFileExtension = ".h";
    char *imgFrontOutputPath;
    char *imgBackOutputPath;
    char *palFrontOutputPath;
    char *palBackOutputPath;
    char *gfxDataOutputPath;
    char *inputPath = argv[1];
    char *outputPath = argv[2];

    size_t imgFrontOutputPathSize = strlen(outputPath) + strlen(imgFrontFileExtension) + 1;
    size_t imgBackOutputPathSize = strlen(outputPath) + strlen(imgBackFileExtension) + 1;
    size_t palFrontOutputPathSize = strlen(outputPath) + strlen(palFrontFileExtension) + 1;
    size_t palBackOutputPathSize = strlen(outputPath) + strlen(palBackFileExtension) + 1;
    size_t gfxDataOutputPathSize = strlen(outputPath) + strlen(gfxDataFileExtension) + 1;

// alloc front sprite
    imgFrontOutputPath = malloc(imgFrontOutputPathSize);
    if (imgFrontOutputPath == NULL)
        FATAL_ERROR("Failed to allocate memory for front tiles output path.\n");
    strcpy(imgFrontOutputPath, outputPath);
    strncat(imgFrontOutputPath, imgFrontFileExtension, strlen(imgFrontFileExtension));

    palFrontOutputPath = malloc(palFrontOutputPathSize);
    if (palFrontOutputPath == NULL)
        FATAL_ERROR("Failed to allocate memory for front pal output path.\n");
    strcpy(palFrontOutputPath, outputPath);
    strncat(palFrontOutputPath, palFrontFileExtension, strlen(palFrontFileExtension));

// alloc back sprite
    imgBackOutputPath = malloc(imgBackOutputPathSize);
    if (imgBackOutputPath == NULL)
        FATAL_ERROR("Failed to allocate memory for back tiles output path.\n");
    strcpy(imgBackOutputPath, outputPath);
    strncat(imgBackOutputPath, imgBackFileExtension, strlen(imgBackFileExtension));

    palBackOutputPath = malloc(palBackOutputPathSize);
    if (palBackOutputPath == NULL)
        FATAL_ERROR("Failed to allocate memory for back pal output path.\n");
    strcpy(palBackOutputPath, outputPath);
    strncat(palBackOutputPath, palBackFileExtension, strlen(palBackFileExtension));

// alloc gfx data
    gfxDataOutputPath = malloc(gfxDataOutputPathSize);
    if (gfxDataOutputPath == NULL)
        FATAL_ERROR("Failed to allocate memory for back pal output path.\n");
    strcpy(gfxDataOutputPath, outputPath);
    strncat(gfxDataOutputPath, gfxDataFileExtension, strlen(gfxDataFileExtension));

    HandleMonGfx(inputPath, imgFrontOutputPath, imgBackOutputPath, palFrontOutputPath, palBackOutputPath, gfxDataOutputPath, basename(outputPath));
    free(imgFrontOutputPath);
    free(imgBackOutputPath);
    free(palFrontOutputPath);
    free(palBackOutputPath);
    free(gfxDataOutputPath);

    return 0;
}
