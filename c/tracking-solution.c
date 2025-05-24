#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <sys/stat.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define WIDTH 1000
#define HEIGHT 1000
#define CHANNELS 3

static unsigned char image[WIDTH * HEIGHT * CHANNELS];

void clear_image() {
    for (int i = 0; i < WIDTH * HEIGHT * CHANNELS; ++i)
        image[i] = 255;
}

void draw_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
    if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT)
        return;

    int index = (y * WIDTH + x) * CHANNELS;
    image[index + 0] = r;
    image[index + 1] = g;
    image[index + 2] = b;
}

void draw_test_crosshairs() {
    for (int x = 0; x < WIDTH; x++) draw_pixel(x, HEIGHT / 2, 255, 0, 0);
    for (int y = 0; y < HEIGHT; y++) draw_pixel(WIDTH / 2, y, 0, 0, 255);
}

void print_usage() {
    fprintf(stderr, "Usage: tracking-solution --input <input.json> --output <output.json> --vis-dir <dir>\n");
}

int main(int argc, char **argv) {
    const char *input_path = NULL;
    const char *output_path = NULL;
    const char *vis_dir = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--input") == 0 && i + 1 < argc)
            input_path = argv[++i];
        else if (strcmp(argv[i], "--output") == 0 && i + 1 < argc)
            output_path = argv[++i];
        else if (strcmp(argv[i], "--vis-dir") == 0 && i + 1 < argc)
            vis_dir = argv[++i];
    }

    if (!input_path || !output_path || !vis_dir) {
        print_usage();
        return 1;
    }

    // For now: confirm paths
    printf("📥 Input: %s\n", input_path);
    printf("📤 Output: %s\n", output_path);
    printf("🖼️  Visualization Dir: %s\n", vis_dir);

    // Ensure output directory exists
    mkdir(vis_dir, 0777);

    // Build image output path
    char vis_path[1024];
    snprintf(vis_path, sizeof(vis_path), "%s/tracking_summary.png", vis_dir);

    // TODO: parse input_path JSON and do real work
    clear_image();
    draw_test_crosshairs();

    printf("💾 Writing visualization to %s...\n", vis_path);
    if (!stbi_write_png(vis_path, WIDTH, HEIGHT, CHANNELS, image, WIDTH * CHANNELS)) {
        fprintf(stderr, "❌ Failed to write image\n");
        return 1;
    }

    // Stub JSON output file
    FILE *fout = fopen(output_path, "w");
    if (fout) {
        fprintf(fout, "{ \"status\": \"ok\", \"frames_processed\": 1 }\n");
        fclose(fout);
    } else {
        fprintf(stderr, "❌ Failed to write output JSON to %s\n", output_path);
        return 1;
    }

    printf("✅ Done.\n");
    return 0;
}
