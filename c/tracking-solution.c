#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <sys/stat.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

//JSON stuff
#include <cjson/cJSON.h>

//quadtree
#include "quadtree.h"

#define WIDTH 1000
#define HEIGHT 1000
#define CHANNELS 4
#define BORDER_WIDTH 2

static unsigned char image[WIDTH * HEIGHT * CHANNELS];

static stbtt_fontinfo font;
static unsigned char *ttf_buffer = NULL;
static float font_scale = 0.0f;

// Max objects you expect per run (adjust as needed)
#define MAX_DETECTIONS 8192
#define MAX_INSERT_QUEUE 1024

static TrackedObject g_insert_queue[MAX_INSERT_QUEUE];
static int g_insert_queue_count = 0;

static TrackedObject g_all_detections[MAX_DETECTIONS];
static int g_detection_index = 0;

static int g_next_id = 1;  // start from 1
static QuadTreeNode *g_quadtree = NULL;

void init_quadtree() {
    g_quadtree = qt_create(QT_MAX_DEPTH);
    qt_subdivide(g_quadtree, QT_MAX_DEPTH);
    g_detection_index = 0;
}

void queue_unmatched_object(const TrackedObject* obj) {
    if (g_insert_queue_count >= MAX_INSERT_QUEUE) {
        fprintf(stderr, "❌ Insert queue overflow! Object dropped.\n");
        return;
    }

    TrackedObject* target = &g_insert_queue[g_insert_queue_count++];
    *target = *obj;
    target->id = g_next_id++;  // 🆔 Assign unique ID

    printf("     📥 Queued new object ID %d for insert\n", target->id);
}

void process_insert_queue() {
    for (int i = 0; i < g_insert_queue_count; i++) {
        qt_insert(g_quadtree, &g_insert_queue[i]);

        // Store inserted object into global detection list
        if (g_detection_index < MAX_DETECTIONS) {
            g_all_detections[g_detection_index++] = g_insert_queue[i];
        }
    }
    printf("🌱 Inserted %d new objects into QuadTree.\n", g_insert_queue_count);
    g_insert_queue_count = 0;
}

void insert_detection_from_json(cJSON *det) {
    if (g_detection_index >= MAX_DETECTIONS) return;

    TrackedObject *obj = &g_all_detections[g_detection_index++];
    obj->x = (float)cJSON_GetObjectItem(det, "x")->valuedouble;
    obj->y = (float)cJSON_GetObjectItem(det, "y")->valuedouble;
    obj->width = (float)cJSON_GetObjectItem(det, "width")->valuedouble;
    obj->height = (float)cJSON_GetObjectItem(det, "height")->valuedouble;

    qt_insert(g_quadtree, obj);
}

TrackedObject* find_closest_object(const TrackedObject* query, TrackedObject** candidates, int count) {
    if (count == 0) return NULL;

    TrackedObject* best = NULL;
    float best_dist2 = 9999.0f;

    for (int i = 0; i < count; i++) {
        TrackedObject* target = candidates[i];

        if (target->matched_this_frame) continue;  // 🛡️ Skip already-matched

        float dx = query->x - target->x;
        float dy = query->y - target->y;
        float dist2 = dx * dx + dy * dy;

        if (dist2 < best_dist2) {
            best = target;
            best_dist2 = dist2;
        }
    }

    return best;
}

void process_frame_from_json(cJSON *frame) {
    if (!frame) return;

    int frame_id = cJSON_GetObjectItem(frame, "frame_id")->valueint;
    cJSON *detections = cJSON_GetObjectItem(frame, "detections");
    int det_count = cJSON_GetArraySize(detections);

    printf("🔍 Processing Frame %d with %d detection(s)\n", frame_id, det_count);

    for (int j = 0; j < det_count; j++) {
        cJSON *det = cJSON_GetArrayItem(detections, j);

        float x = (float)cJSON_GetObjectItem(det, "x")->valuedouble;
        float y = (float)cJSON_GetObjectItem(det, "y")->valuedouble;
        float w = (float)cJSON_GetObjectItem(det, "width")->valuedouble;
        float h = (float)cJSON_GetObjectItem(det, "height")->valuedouble;

        // Create a temporary DetectedObject
        TrackedObject query = {
            .x = x,
            .y = y,
            .width = w,
            .height = h
        };

        // Query QuadTree for potential overlaps
        TrackedObject *hits[32];
        int hit_count = 0;
        qt_query(g_quadtree, &query, hits, &hit_count, 32);

        printf("   🧠 Detection %d: (x=%.3f, y=%.3f) — %d hit(s)\n", j, x, y, hit_count);

        if (hit_count == 0) {
            // ➕ No matches — handle later
            printf("     ➕ NEW OBJECT: will be inserted later.\n");
            queue_unmatched_object(&query);
        } else {
            // 📍 Matches found — handle closest resolution later
            printf("     🔄 MATCH CANDIDATES: resolve identity next.\n");
            TrackedObject* match = find_closest_object(&query, hits, hit_count);
            if (match) {
                printf("     🎯 Closest match at (x=%.3f, y=%.3f)\n", match->x, match->y);
                match->x = query.x;
                match->y = query.y;
                match->width = query.width;
                match->height = query.height;
                match->matched_this_frame = true;
            } else {
                printf("     ❌ No viable match — queuing insert\n");
                queue_unmatched_object(&query);
            }
        }
    }
}

void reset_match_flags(TrackedObject *objects, int count) {
    for (int i = 0; i < count; i++) {
        objects[i].matched_this_frame = false;
    }
}

void clear_image() {
    memset(image, 0, sizeof(image)); // Fully transparent
}

void blit_glyph(int x, int y, int w, int h, unsigned char *bitmap) {
    for (int dy = 0; dy < h; dy++) {
        for (int dx = 0; dx < w; dx++) {
            int gx = x + dx;
            int gy = y + dy;
            if (gx < 0 || gx >= WIDTH || gy < 0 || gy >= HEIGHT)
                continue;
            int alpha = bitmap[dy * w + dx];
            int idx = (gy * WIDTH + gx) * CHANNELS;
            image[idx + 0] = 255;   // white text
            image[idx + 1] = 255;
            image[idx + 2] = 255;
            image[idx + 3] = alpha;
        }
    }
}

void draw_text(int x, int y, const char *text) {
    int px = x;

    for (const char *p = text; *p; p++) {
        int w, h, xoff, yoff;
        unsigned char *bitmap = stbtt_GetCodepointBitmap(&font, 0, font_scale, *p, &w, &h, &xoff, &yoff);
        blit_glyph(px + xoff, y + yoff, w, h, bitmap);

        int advance, lsb;
        stbtt_GetCodepointHMetrics(&font, *p, &advance, &lsb);
        px += (int)(advance * font_scale);

        stbtt_FreeBitmap(bitmap, NULL);
    }
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
            draw_pixel(px, py, r, g, b, 255);
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

            char label[64];
            snprintf(label, sizeof(label), "F%03d O%02d", frame_id->valueint, j);
            draw_text(10, 30, label);  // draw top-left, can reposition            

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

    FILE *font_file = fopen("/crop-tracking/fonts/DejaVuSansMono.ttf", "rb");
    if (!font_file) {
        fprintf(stderr, "❌ Could not load DejaVuSansMono.ttf\n");
        return 1;
    }
    ttf_buffer = malloc(1 << 20); // 1 MB
    fread(ttf_buffer, 1, 1 << 20, font_file);
    fclose(font_file);

    if (!stbtt_InitFont(&font, ttf_buffer, stbtt_GetFontOffsetForIndex(ttf_buffer, 0))) {
        fprintf(stderr, "❌ Font init failed\n");
        return 1;
    }
    font_scale = stbtt_ScaleForPixelHeight(&font, 28);  // ~28px tall    

    // Build image output path
    char vis_path[1024];
    snprintf(vis_path, sizeof(vis_path), "%s/tracking_summary.png", vis_dir);

    // TODO: parse input_path JSON and do real work
    clear_image();
    //draw_test_crosshairs();

    /*
    printf("💾 Writing visualization to %s...\n", vis_path);
    if (!stbi_write_png(vis_path, WIDTH, HEIGHT, CHANNELS, image, WIDTH * CHANNELS)) {
        fprintf(stderr, "❌ Failed to write image\n");
        return 1;
    }
    */

    //generate_images_from_json(input_path, vis_dir);

    init_quadtree();
    // Load and parse the input JSON for tracking logic
    FILE *fp = fopen(input_path, "rb");
    if (!fp) {
        fprintf(stderr, "❌ Failed to open %s\n", input_path);
        return 1;
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
        fprintf(stderr, "❌ JSON parse error in %s\n", input_path);
        free(buffer);
        return 1;
    }

    int frame_count = cJSON_GetArraySize(root);
    for (int i = 0; i < frame_count; i++) {
        cJSON *frame = cJSON_GetArrayItem(root, i);
        reset_match_flags(g_all_detections, g_detection_index);
        process_frame_from_json(frame);
        process_insert_queue();
        qt_prune_unmatched_objects(g_quadtree, g_all_detections, &g_detection_index);
    }

    // Stub JSON output file
    /*
    FILE *fout = fopen(output_path, "w");
    if (fout) {
        fprintf(fout, "{ \"status\": \"ok\", \"frames_processed\": 1 }\n");
        fclose(fout);
    } else {
        fprintf(stderr, "❌ Failed to write output JSON to %s\n", output_path);
        return 1;
    }
    */

    clear_image();
    for (int i = 0; i < g_detection_index; i++) {
        TrackedObject *obj = &g_all_detections[i];
        draw_border_box(obj->x, obj->y, obj->width, obj->height, 0, 255, 0, 255);

        char label[32];
        snprintf(label, sizeof(label), "ID %d", obj->id);
        draw_text((int)(obj->x * WIDTH), (int)(obj->y * HEIGHT) - 10, label);
    }

    printf("💾 Writing visualization to %s...\n", vis_path);
    stbi_write_png(vis_path, WIDTH, HEIGHT, CHANNELS, image, WIDTH * CHANNELS);    

    printf("✅ Done.\n");
    return 0;
}
