#ifndef BSON_LINKED_LIST_H
#define BSON_LINKED_LIST_H

#include <stddef.h>
#include <stdbool.h>
#include <pthread.h>
#include <bson/bson.h>

// Function pointer for destroying stored bson documents (e.g., bson_destroy)
typedef void (*bson_value_destroyer)(void* value);

typedef struct BsonListNode BsonListNode;

typedef struct BsonLinkedList {
    BsonListNode* head;
    BsonListNode* tail;
    size_t size;
    pthread_rwlock_t lock; // List-level RW lock
    bson_value_destroyer destroy_doc; // called on each document when removed/destroyed
} BsonLinkedList;

// Predicate and updater types
typedef bool (*bson_matcher)(const bson_t* doc, void* ctx);
typedef int (*bson_updater)(bson_t* doc, void* ctx);

BsonLinkedList* bson_ll_create(bson_value_destroyer destroy_doc);
void bson_ll_destroy(BsonLinkedList* list);

int bson_ll_push_back(BsonLinkedList* list, bson_t* doc);
int bson_ll_push_front(BsonLinkedList* list, bson_t* doc);

// Returns the first matching document pointer (do not free); NULL if not found
const bson_t* bson_ll_find_first(BsonLinkedList* list, bson_matcher match, void* ctx);

// Deletes the first matching document; returns 0 if deleted, -1 if not found
int bson_ll_delete_first(BsonLinkedList* list, bson_matcher match, void* ctx);

// Updates the first matching document using updater; updater should modify in place; returns 0 on success, -1 if not found or updater fails
int bson_ll_update_first(BsonLinkedList* list, bson_matcher match, void* mctx, bson_updater update, void* uctx);

size_t bson_ll_size(BsonLinkedList* list);

#endif // BSON_LINKED_LIST_H


