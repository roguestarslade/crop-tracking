#ifndef QUADTREE_H
#define QUADTREE_H

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#define QT_MAX_DEPTH 8
#define QT_CHILDREN 4
#define QT_INIT_CAPACITY 4

typedef struct {
    float x;      // center
    float y;
    float width;
    float height;
} DetectedObject;

typedef struct QuadTreeNode {
    float x, y;           // center of this node
    float half_size;      // half-width of node bounds
    int depth;

    struct QuadTreeNode* children[QT_CHILDREN];

    DetectedObject** objects;
    int object_count;
    int object_capacity;
} QuadTreeNode;

//
// 🧱 Utility: AABB overlap
//
static inline bool qt_overlap(const DetectedObject* a, const DetectedObject* b) {
    float ax0 = a->x - a->width * 0.5f;
    float ay0 = a->y - a->height * 0.5f;
    float ax1 = a->x + a->width * 0.5f;
    float ay1 = a->y + a->height * 0.5f;

    float bx0 = b->x - b->width * 0.5f;
    float by0 = b->y - b->height * 0.5f;
    float bx1 = b->x + b->width * 0.5f;
    float by1 = b->y + b->height * 0.5f;

    return !(ax1 < bx0 || ax0 > bx1 || ay1 < by0 || ay0 > by1);
}

//
// 🪓 Create a new QuadTree node
//
static QuadTreeNode* qt_create_node(float x, float y, float half_size, int depth) {
    QuadTreeNode* node = (QuadTreeNode*)calloc(1, sizeof(QuadTreeNode));
    node->x = x;
    node->y = y;
    node->half_size = half_size;
    node->depth = depth;
    node->object_capacity = QT_INIT_CAPACITY;
    node->objects = (DetectedObject**)malloc(sizeof(DetectedObject*) * QT_INIT_CAPACITY);
    return node;
}

//
// 🌲 Build full tree to specified depth
//
static QuadTreeNode* qt_create(int max_depth) {
    QuadTreeNode* root = qt_create_node(0.5f, 0.5f, 0.5f, 0);
    if (max_depth > 0) {
        for (int i = 0; i < QT_CHILDREN; i++) root->children[i] = NULL;
        for (int i = 0; i < QT_CHILDREN; i++) {
            float offset = root->half_size * 0.5f;
            float dx = (i & 1) ? offset : -offset;
            float dy = (i & 2) ? offset : -offset;
            root->children[i] = qt_create_node(root->x + dx, root->y + dy, offset, 1);
        }
    }
    return root;
}

//
// 🌱 Recursively subdivide to depth
//
static void qt_subdivide(QuadTreeNode* node, int max_depth) {
    if (node->depth >= max_depth) return;
    float offset = node->half_size * 0.5f;
    for (int i = 0; i < QT_CHILDREN; i++) {
        float dx = (i & 1) ? offset : -offset;
        float dy = (i & 2) ? offset : -offset;
        node->children[i] = qt_create_node(node->x + dx, node->y + dy, offset, node->depth + 1);
        qt_subdivide(node->children[i], max_depth);
    }
}

//
// ➕ Insert object into leaf
//
static void qt_insert(QuadTreeNode* node, DetectedObject* obj) {
    if (node->depth == QT_MAX_DEPTH || node->children[0] == NULL) {
        if (node->object_count >= node->object_capacity) {
            node->object_capacity *= 2;
            node->objects = (DetectedObject**)realloc(node->objects, sizeof(DetectedObject*) * node->object_capacity);
        }
        node->objects[node->object_count++] = obj;
        return;
    }

    for (int i = 0; i < QT_CHILDREN; i++) {
        QuadTreeNode* child = node->children[i];
        DetectedObject box = {
            .x = child->x,
            .y = child->y,
            .width = child->half_size * 2,
            .height = child->half_size * 2
        };
        if (qt_overlap(&box, obj)) {
            qt_insert(child, obj);
        }
    }
}

//
// 🔍 Query region — collect overlapping objects
//
static void qt_query(const QuadTreeNode* node, const DetectedObject* region, DetectedObject** out_results, int* count, int max_results) {
    DetectedObject box = {
        .x = node->x,
        .y = node->y,
        .width = node->half_size * 2,
        .height = node->half_size * 2
    };
    if (!qt_overlap(&box, region)) return;

    for (int i = 0; i < node->object_count && *count < max_results; i++) {
        if (qt_overlap(node->objects[i], region)) {
            out_results[(*count)++] = node->objects[i];
        }
    }

    if (node->children[0] != NULL) {
        for (int i = 0; i < QT_CHILDREN; i++) {
            qt_query(node->children[i], region, out_results, count, max_results);
        }
    }
}

//
// 🧼 Free quadtree memory
//
static void qt_free(QuadTreeNode* node) {
    if (!node) return;
    for (int i = 0; i < QT_CHILDREN; i++) {
        qt_free(node->children[i]);
    }
    free(node->objects);
    free(node);
}

#endif // QUADTREE_H
