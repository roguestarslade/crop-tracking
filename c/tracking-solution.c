#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define WIDTH 1000
#define HEIGHT 1000

#define CHANNELS 3  // RGB
#define WHITE 255
#define BLACK 0

// Raw pixel buffer: row-major RGB
static unsigned char image[WIDTH * HEIGHT * CHANNELS];

// Clears image to white
void clear_image() {
    for (int i = 0; i < WIDTH * HEIGHT * CHANNELS; ++i) {
        image[i] = WHITE;
    }
}

// Simple pixel plotter
void draw_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
    if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT)
        return;

    int index = (y * WIDTH + x) * CHANNELS;
    image[index + 0] = r;
    image[index + 1] = g;
    image[index + 2] = b;
}

// Simple placeholder test
void draw_test_crosshairs() {
    for (int x = 0; x < WIDTH; x++) {
        draw_pixel(x, HEIGHT / 2, 255, 0, 0);
    }
    for (int y = 0; y < HEIGHT; y++) {
        draw_pixel(WIDTH / 2, y, 0, 0, 255);
    }
}

int main(int argc, char **argv) {
    const char *project_root = getenv("PROJECT_ROOT");
    if (!project_root) {
        fprintf(stderr, "❌ PROJECT_ROOT not set in environment\n");
        return 1;
    }

    char output_dir[512];
    snprintf(output_dir, sizeof(output_dir), "%s/data/visualization", project_root);

    // Ensure output dir exists
    mkdir(output_dir, 0777);

    char output_path[1024];
    snprintf(output_path, sizeof(output_path), "%s/tracking_summary.png", output_dir);

    printf("📐 Initializing blank image canvas...\n");
    clear_image();

    printf("🎯 Drawing test crosshairs...\n");
    draw_test_crosshairs();

    printf("💾 Writing image to %s...\n", output_path);
    if (!stbi_write_png(output_path, WIDTH, HEIGHT, CHANNELS, image, WIDTH * CHANNELS)) {
        fprintf(stderr, "❌ Failed to write image\n");
        return 1;
    }

    printf("✅ Image saved successfully.\n");
    return 0;
}
