#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include <bson/bson.h>

#include "../../llist.h"

static void destroy_bson(void* v) { if (v) bson_destroy((bson_t*)v); }

static bson_t* make_doc(int num, const char* name) {
    bson_t* d = bson_new();
    BSON_APPEND_INT32(d, "number", num);
    BSON_APPEND_UTF8(d, "name", name);
    return d;
}

static bool match_number(const bson_t* d, void* ctx) {
    int target = *(int*)ctx;
    bson_iter_t it;
    return bson_iter_init_find(&it, d, "number") && BSON_ITER_HOLDS_INT32(&it) && bson_iter_int32(&it) == target;
}

static int update_name(bson_t* d, void* ctx) {
    const char* new_name = (const char*)ctx;
    bson_iter_t it;
    int num = -1;
    if (bson_iter_init_find(&it, d, "number") && BSON_ITER_HOLDS_INT32(&it)) num = bson_iter_int32(&it);
    bson_t tmp;
    bson_init(&tmp);
    BSON_APPEND_INT32(&tmp, "number", num);
    BSON_APPEND_UTF8(&tmp, "name", new_name);
    bson_destroy(d);
    *d = tmp;
    return 0;
}

static void test_basic_crud() {
    BsonLinkedList* l = bson_ll_create(destroy_bson);
    assert(l);
    assert(bson_ll_size(l) == 0);
    assert(bson_ll_push_back(l, make_doc(1, "a")) == 0);
    assert(bson_ll_push_front(l, make_doc(2, "b")) == 0);
    assert(bson_ll_size(l) == 2);
    int k = 1;
    const bson_t* f = bson_ll_find_first(l, match_number, &k);
    assert(f);
    const char* newn = "alpha";
    assert(bson_ll_update_first(l, match_number, &k, update_name, (void*)newn) == 0);
    assert(bson_ll_delete_first(l, match_number, &k) == 0);
    assert(bson_ll_size(l) == 1);
    bson_ll_destroy(l);
}

typedef struct { BsonLinkedList* l; int start; int end; } ThreadArgs;

static void* writer_thread(void* arg) {
    ThreadArgs* a = (ThreadArgs*)arg;
    for (int i = a->start; i < a->end; i++) {
        char name[32]; sprintf(name, "w-%d", i);
        bson_ll_push_back(a->l, make_doc(i, name));
    }
    return NULL;
}

static void* reader_thread(void* arg) {
    ThreadArgs* a = (ThreadArgs*)arg;
    for (int i = a->start; i < a->end; i++) { int k = i; bson_ll_find_first(a->l, match_number, &k); }
    return NULL;
}

static void test_race_conditions() {
    BsonLinkedList* l = bson_ll_create(destroy_bson);
    assert(l);
    const int N = 1000;
    ThreadArgs wa = { l, 0, N }, ra = { l, 0, N };
    pthread_t tw, tr;
    pthread_create(&tw, NULL, writer_thread, &wa);
    pthread_create(&tr, NULL, reader_thread, &ra);
    pthread_join(tw, NULL);
    pthread_join(tr, NULL);
    assert(bson_ll_size(l) == (size_t)N);
    bson_ll_destroy(l);
}

static void test_memory_leak_sanity() {
    for (int round = 0; round < 50; round++) {
        BsonLinkedList* l = bson_ll_create(destroy_bson);
        for (int i = 0; i < 100; i++) bson_ll_push_back(l, make_doc(i, "x"));
        for (int i = 0; i < 100; i += 2) bson_ll_delete_first(l, match_number, &i);
        bson_ll_destroy(l);
    }
}

static void test_edge_cases() {
    BsonLinkedList* l = bson_ll_create(destroy_bson);
    assert(l);
    int miss = 9999; const char* n = "n";
    assert(bson_ll_delete_first(l, match_number, &miss) == -1);
    assert(bson_ll_update_first(l, match_number, &miss, update_name, (void*)n) == -1);
    assert(bson_ll_find_first(l, match_number, &miss) == NULL);
    bson_ll_destroy(l);
}

static void test_ordering_and_integrity() {
    BsonLinkedList* l = bson_ll_create(destroy_bson);
    assert(l);
    for (int i = 0; i < 10; i++) assert(bson_ll_push_back(l, make_doc(i, "z")) == 0);
    assert(bson_ll_size(l) == 10);
    int k = 0; assert(bson_ll_delete_first(l, match_number, &k) == 0);
    k = 9; assert(bson_ll_delete_first(l, match_number, &k) == 0);
    assert(bson_ll_size(l) == 8);
    bson_ll_destroy(l);
}

int main() {
    printf("=== BSON Linked List tests ===\n");
    test_basic_crud();
    test_race_conditions();
    test_memory_leak_sanity();
    test_edge_cases();
    test_ordering_and_integrity();
    printf("All Linked List tests passed.\n");
    return 0;
}


