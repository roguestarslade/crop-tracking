#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <sys/stat.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

//JSON stuff
#include <cjson/cJSON.h>

#define WIDTH 1000
#define HEIGHT 1000
#define CHANNELS 3
#define BORDER_WIDHT 2

static unsigned char image[WIDTH * HEIGHT * CHANNELS];

void clear_image() {
    memset(image, 0, sizeof(image)); // Fully transparent
}

void draw_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT)
        return;

    int index = (y * WIDTH + x) * CHANNELS;
    image[index + 0] = r;
    image[index + 1] = g;
    image[index + 2] = b;
    image[index + 3] = a;
}

void draw_test_crosshairs() {
    for (int x = 0; x < WIDTH; x++) draw_pixel(x, HEIGHT / 2, 255, 0, 0, 255);
    for (int y = 0; y < HEIGHT; y++) draw_pixel(WIDTH / 2, y, 0, 0, 255, 255);
}

void print_usage() {
    fprintf(stderr, "Usage: tracking-solution --input <input.json> --output <output.json> --vis-dir <dir>\n");
}

void draw_box(float x, float y, float w, float h, uint8_t r, uint8_t g, uint8_t b) {
    int x0 = (int)(x * WIDTH);
    int y0 = (int)(y * HEIGHT);
    int x1 = (int)((x + w) * WIDTH);
    int y1 = (int)((y + h) * HEIGHT);

    for (int py = y0; py < y1; py++) {
        for (int px = x0; px < x1; px++) {
            draw_pixel(px, py, r, g, b);
        }
    }
}

void draw_border_box(float fx, float fy, float fw, float fh, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    int x0 = (int)(fx * WIDTH);
    int y0 = (int)(fy * HEIGHT);
    int x1 = (int)((fx + fw) * WIDTH);
    int y1 = (int)((fy + fh) * HEIGHT);

    for (int i = 0; i < BORDER_WIDTH; i++) {
        // Top border
        for (int x = x0; x < x1; x++) draw_pixel(x, y0 + i, r, g, b, a);
        // Bottom border
        for (int x = x0; x < x1; x++) draw_pixel(x, y1 - i - 1, r, g, b, a);
        // Left border
        for (int y = y0; y < y1; y++) draw_pixel(x0 + i, y, r, g, b, a);
        // Right border
        for (int y = y0; y < y1; y++) draw_pixel(x1 - i - 1, y, r, g, b, a);
    }
}

void generate_images_from_json(const char *input_path, const char *vis_dir) {
    FILE *fp = fopen(input_path, "rb");
    if (!fp) {
        fprintf(stderr, "❌ Failed to open %s\n", input_path);
        return;
    }

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    rewind(fp);
    char *buffer = malloc(size + 1);
    fread(buffer, 1, size, fp);
    buffer[size] = '\0';
    fclose(fp);

    cJSON *root = cJSON_Parse(buffer);
    if (!root) {
        fprintf(stderr, "❌ JSON parse error in file: %s\n", input_path);
        free(buffer);
        return;
    }

    int frame_count = cJSON_GetArraySize(root);
    printf("🧪 Found %d frame(s) in input JSON\n", frame_count);

    for (int i = 0; i < frame_count; i++) {
        cJSON *frame = cJSON_GetArrayItem(root, i);
        cJSON *frame_id = cJSON_GetObjectItem(frame, "frame_id");
        cJSON *detections = cJSON_GetObjectItem(frame, "detections");

        int det_count = cJSON_GetArraySize(detections);
        printf("🎬 Frame %d: %d detection(s)\n", frame_id->valueint, det_count);

        for (int j = 0; j < det_count; j++) {
            cJSON *det = cJSON_GetArrayItem(detections, j);
            float x = cJSON_GetObjectItem(det, "x")->valuedouble;
            float y = cJSON_GetObjectItem(det, "y")->valuedouble;
            float w = cJSON_GetObjectItem(det, "width")->valuedouble;
            float h = cJSON_GetObjectItem(det, "height")->valuedouble;

            clear_image();
            //draw_box(x, y, w, h, 0, 0, 0); // draw black box
            draw_border_box(x, y, w, h, 255, 0, 0, 255); // red border box

            char outpath[1024];
            snprintf(outpath, sizeof(outpath), "%s/frame%03d_obj%02d.png", vis_dir, frame_id->valueint, j);

            if (stbi_write_png(outpath, WIDTH, HEIGHT, CHANNELS, image, WIDTH * CHANNELS)) {
                printf("✅ Wrote: %s  (x=%.3f, y=%.3f, w=%.3f, h=%.3f)\n",
                    outpath, x, y, w, h);
            } else {
                fprintf(stderr, "❌ Failed to write image: %s\n", outpath);
            }
        }
    }

    cJSON_Delete(root);
    free(buffer);
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

    generate_images_from_json(input_path, vis_dir);

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
