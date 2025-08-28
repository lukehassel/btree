// Minimal BSON value tests for B+ tree
// Requires libbson (e.g., brew install mongo-c-driver)

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <bson/bson.h>

#include "../btree.h"

static int compare_int_keys(const void* a, const void* b) {
    return *((const int*)a) - *((const int*)b);
}

static void destroy_bson_value(void* value) {
    if (value != NULL) {
        bson_destroy((bson_t*)value);
    }
}

static bson_t* make_doc_int_str(int number, const char* name) {
    bson_t* doc = bson_new();
    BSON_APPEND_INT32(doc, "number", number);
    BSON_APPEND_UTF8(doc, "name", name);
    return doc;
}

static const char* get_name_field(const bson_t* doc) {
    bson_iter_t it;
    if (bson_iter_init_find(&it, doc, "name") && BSON_ITER_HOLDS_UTF8(&it)) {
        return bson_iter_utf8(&it, NULL);
    }
    return NULL;
}

static int get_number_field(const bson_t* doc) {
    bson_iter_t it;
    if (bson_iter_init_find(&it, doc, "number") && BSON_ITER_HOLDS_INT32(&it)) {
        return bson_iter_int32(&it);
    }
    return INT_MIN;
}

static void test_basic_bson_store_and_find() {
    BPlusTree* tree = bplus_tree_create(DEFAULT_ORDER, compare_ints, destroy_bson_value);
    assert(tree != NULL);

    int* k1 = malloc(sizeof(int)); *k1 = 1;
    int* k2 = malloc(sizeof(int)); *k2 = 2;
    int* k3 = malloc(sizeof(int)); *k3 = 3;

    bson_t* d1 = make_doc_int_str(10, "alpha");
    bson_t* d2 = make_doc_int_str(20, "bravo");
    bson_t* d3 = make_doc_int_str(30, "charlie");

    assert(bplus_tree_insert(tree, k1, d1) == 0);
    assert(bplus_tree_insert(tree, k2, d2) == 0);
    assert(bplus_tree_insert(tree, k3, d3) == 0);

    const bson_t* f2 = (const bson_t*)bplus_tree_find(tree, k2);
    assert(f2 != NULL);
    assert(strcmp(get_name_field(f2), "bravo") == 0);
    assert(get_number_field(f2) == 20);

    int miss = 9;
    assert(bplus_tree_find(tree, &miss) == NULL);

    bplus_tree_destroy(tree);
    free(k1); free(k2); free(k3);
}

static void test_find_range_with_bson_values() {
    BPlusTree* tree = bplus_tree_create(DEFAULT_ORDER, compare_ints, destroy_bson_value);
    assert(tree != NULL);

    const int n = 10;
    int* keys[n];
    for (int i = 0; i < n; i++) {
        keys[i] = malloc(sizeof(int));
        *keys[i] = i;
        char name[32];
        sprintf(name, "name-%d", i);
        bson_t* doc = make_doc_int_str(i * 100, name);
        assert(bplus_tree_insert(tree, keys[i], doc) == 0);
    }

    int start = 3, end = 7;
    void* results[16];
    int count = bplus_tree_find_range(tree, &start, &end, results, 16);
    assert(count == (end - start + 1));
    const bson_t* first = (const bson_t*)results[0];
    assert(strcmp(get_name_field(first), "name-3") == 0);
    const bson_t* last = (const bson_t*)results[count - 1];
    assert(strcmp(get_name_field(last), "name-7") == 0);

    bplus_tree_destroy(tree);
    for (int i = 0; i < n; i++) free(keys[i]);
}

int main() {
    printf("=== BSON value tests ===\n");
    test_basic_bson_store_and_find();
    test_find_range_with_bson_values();
    // Search by embedded BSON field by scanning a key range
    {
        BPlusTree* tree = bplus_tree_create(DEFAULT_ORDER, compare_ints, destroy_bson_value);
        assert(tree != NULL);
        const int n = 10;
        int* keys[n];
        for (int i = 0; i < n; i++) {
            keys[i] = malloc(sizeof(int));
            *keys[i] = i;
            char name[32];
            sprintf(name, "item-%d", i);
            if (i == 6) strcpy(name, "target");
            bson_t* doc = make_doc_int_str(i, name);
            assert(bplus_tree_insert(tree, keys[i], doc) == 0);
        }

        int start = 0, end = 9;
        void* results[16];
        int count = bplus_tree_find_range(tree, &start, &end, results, 16);
        assert(count == 10);
        int found_idx = -1;
        for (int i = 0; i < count; i++) {
            const bson_t* d = (const bson_t*)results[i];
            const char* nm = get_name_field(d);
            if (nm && strcmp(nm, "target") == 0) { found_idx = i; break; }
        }
        assert(found_idx >= 0);

        bplus_tree_destroy(tree);
        for (int i = 0; i < n; i++) free(keys[i]);
    }
    printf("All BSON tests passed.\n");
    return 0;
}


