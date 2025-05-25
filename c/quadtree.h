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
    int id;
    float x;
    float y;
    float width;
    float height;
    bool matched_this_frame;
} TrackedObject;

typedef struct QuadTreeNode {
    float x, y;           // center of this node
    float half_size;      // half-width of node bounds
    int depth;

    struct QuadTreeNode* children[QT_CHILDREN];

    TrackedObject** objects;
    int object_count;
    int object_capacity;
} QuadTreeNode;

//
// 🧱 Utility: AABB overlap
//
static inline bool qt_overlap(const TrackedObject* a, const TrackedObject* b) {
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

bool already_exists_in_node(QuadTreeNode *node, TrackedObject *obj) {
    for (int i = 0; i < node->object_count; i++) {
        TrackedObject *existing = node->objects[i];
        if (existing && existing->id == obj->id) {
            return true;
        }
    }
    return false;
}

void insert_object(QuadTreeNode *node, TrackedObject *obj) {
    if (already_exists_in_node(node, obj)) {
        return;
    }

    if (node->object_count < node->object_capacity) {
        node->objects[node->object_count++] = obj;
    } else {
        // Optional: you can subdivide or warn depending on how your tree handles overflow
        fprintf(stderr, "⚠️ Node full at depth %d — consider subdividing.\n", node->depth);
    }
}

bool qt_contains(QuadTreeNode *node, TrackedObject *obj) {
    float minX = node->x - node->half_size;
    float maxX = node->x + node->half_size;
    float minY = node->y - node->half_size;
    float maxY = node->y + node->half_size;

    float objX = obj->x;
    float objY = obj->y;

    return objX >= minX && objX <= maxX && objY >= minY && objY <= maxY;
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
    node->objects = (TrackedObject**)malloc(sizeof(TrackedObject*) * QT_INIT_CAPACITY);
    return node;
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
// 🌲 Build full tree to specified depth
//
static QuadTreeNode* qt_create(int max_depth) {
    QuadTreeNode* root = qt_create_node(0.5f, 0.5f, 0.5f, 0);
    qt_subdivide(root, max_depth);
    return root;
}

//
// ➕ Insert object into leaf
//
static void qt_insert(QuadTreeNode* node, TrackedObject* obj) {
    printf("➕ Inserted object ID %d at (%.3f, %.3f)\n", obj->id, obj->x, obj->y);
    if (node->depth == QT_MAX_DEPTH || node->children[0] == NULL) {
        if (node->object_count >= node->object_capacity) {
            node->object_capacity *= 2;
            node->objects = (TrackedObject**)realloc(node->objects, sizeof(TrackedObject*) * node->object_capacity);
        }
        //node->objects[node->object_count++] = obj;
        if (!already_exists_in_node(node, obj)) {
            insert_object(node, obj);
        }  
        return;
    }

    for (int i = 0; i < QT_CHILDREN; i++) {
        QuadTreeNode* child = node->children[i];
        TrackedObject box = {
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
static void qt_query(const QuadTreeNode* node, const TrackedObject* region, TrackedObject** out_results, int* count, int max_results) {
    printf("🔍 Query region: (%.3f, %.3f, %.3f x %.3f)\n", region->x, region->y, region->width, region->height);
    TrackedObject box = {
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

static void qt_clear_objects(QuadTreeNode* node) {
    if (!node) return;

    node->object_count = 0;

    if (node->children[0] != NULL) {
        for (int i = 0; i < QT_CHILDREN; i++) {
            qt_clear_objects(node->children[i]);
        }
    }
}

static void qt_prune_unmatched_objects(QuadTreeNode* tree, TrackedObject* all, int* count_ptr) {
    int new_count = 0;
    int original_count = *count_ptr;

    for (int i = 0; i < original_count; i++) {
        TrackedObject* obj = &all[i];
        if (obj->matched_this_frame) {
            obj->matched_this_frame = false;
            all[new_count++] = *obj;
        }
    }

    *count_ptr = new_count;
    qt_clear_objects(tree);

    for (int i = 0; i < new_count; i++) {
        qt_insert(tree, &all[i]);
    }

    printf("🧹 Pruned %d stale object(s); %d remain\n", original_count - new_count, new_count);
}

static void qt_move_object(QuadTreeNode* tree, TrackedObject* obj, float new_x, float new_y, float new_w, float new_h) {
    obj->x = new_x;
    obj->y = new_y;
    obj->width = new_w;
    obj->height = new_h;

    qt_insert(tree, obj); // reinserts it — may appear in multiple overlapping nodes
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
