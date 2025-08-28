#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "llist.h"

struct BsonListNode {
    bson_t* doc;
    BsonListNode* next;
    BsonListNode* prev;
};

static BsonListNode* make_node(bson_t* doc) {
    BsonListNode* n = (BsonListNode*)malloc(sizeof(BsonListNode));
    if (!n) return NULL;
    n->doc = doc;
    n->next = NULL;
    n->prev = NULL;
    return n;
}

BsonLinkedList* bson_ll_create(bson_value_destroyer destroy_doc) {
    BsonLinkedList* list = (BsonLinkedList*)malloc(sizeof(BsonLinkedList));
    if (!list) return NULL;
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
    list->destroy_doc = destroy_doc;
    pthread_rwlock_init(&list->lock, NULL);
    return list;
}

void bson_ll_destroy(BsonLinkedList* list) {
    if (!list) return;
    pthread_rwlock_wrlock(&list->lock);
    BsonListNode* cur = list->head;
    while (cur) {
        BsonListNode* nxt = cur->next;
        if (list->destroy_doc && cur->doc) list->destroy_doc(cur->doc);
        free(cur);
        cur = nxt;
    }
    pthread_rwlock_unlock(&list->lock);
    pthread_rwlock_destroy(&list->lock);
    free(list);
}

int bson_ll_push_back(BsonLinkedList* list, bson_t* doc) {
    if (!list || !doc) return -1;
    BsonListNode* n = make_node(doc);
    if (!n) return -1;
    pthread_rwlock_wrlock(&list->lock);
    if (!list->tail) {
        list->head = list->tail = n;
    } else {
        n->prev = list->tail;
        list->tail->next = n;
        list->tail = n;
    }
    list->size++;
    pthread_rwlock_unlock(&list->lock);
    return 0;
}

int bson_ll_push_front(BsonLinkedList* list, bson_t* doc) {
    if (!list || !doc) return -1;
    BsonListNode* n = make_node(doc);
    if (!n) return -1;
    pthread_rwlock_wrlock(&list->lock);
    if (!list->head) {
        list->head = list->tail = n;
    } else {
        n->next = list->head;
        list->head->prev = n;
        list->head = n;
    }
    list->size++;
    pthread_rwlock_unlock(&list->lock);
    return 0;
}

const bson_t* bson_ll_find_first(BsonLinkedList* list, bson_matcher match, void* ctx) {
    if (!list || !match) return NULL;
    pthread_rwlock_rdlock(&list->lock);
    const bson_t* out = NULL;
    for (BsonListNode* cur = list->head; cur; cur = cur->next) {
        if (match(cur->doc, ctx)) { out = cur->doc; break; }
    }
    pthread_rwlock_unlock(&list->lock);
    return out;
}

static void unlink_and_free_node(BsonLinkedList* list, BsonListNode* node) {
    if (node->prev) node->prev->next = node->next;
    else list->head = node->next;
    if (node->next) node->next->prev = node->prev;
    else list->tail = node->prev;
    if (list->destroy_doc && node->doc) list->destroy_doc(node->doc);
    free(node);
    list->size--;
}

int bson_ll_delete_first(BsonLinkedList* list, bson_matcher match, void* ctx) {
    if (!list || !match) return -1;
    pthread_rwlock_wrlock(&list->lock);
    for (BsonListNode* cur = list->head; cur; cur = cur->next) {
        if (match(cur->doc, ctx)) {
            unlink_and_free_node(list, cur);
            pthread_rwlock_unlock(&list->lock);
            return 0;
        }
    }
    pthread_rwlock_unlock(&list->lock);
    return -1;
}

int bson_ll_update_first(BsonLinkedList* list, bson_matcher match, void* mctx, bson_updater update, void* uctx) {
    if (!list || !match || !update) return -1;
    pthread_rwlock_wrlock(&list->lock);
    for (BsonListNode* cur = list->head; cur; cur = cur->next) {
        if (match(cur->doc, mctx)) {
            int rc = update(cur->doc, uctx);
            pthread_rwlock_unlock(&list->lock);
            return rc;
        }
    }
    pthread_rwlock_unlock(&list->lock);
    return -1;
}

size_t bson_ll_size(BsonLinkedList* list) {
    if (!list) return 0;
    pthread_rwlock_rdlock(&list->lock);
    size_t s = list->size;
    pthread_rwlock_unlock(&list->lock);
    return s;
}


