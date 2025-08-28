# 🚀 High-Performance B+ Tree (pthread) Implementation

[![CI](https://github.com/lukehassel/btree/actions/workflows/ci.yml/badge.svg?branch=stable)](https://github.com/lukehassel/btree/actions/workflows/ci.yml)

A robust, thread-safe B+ Tree implementation in C using pthread read-write locks for high concurrency.

## ✨ Features

- **🧵 Thread-Safe**: Pthread read-write locks for concurrent access
- **🔧 Generic Design**: Void pointer interface for flexible key/value types
- **📊 Comprehensive Testing**: Extensive test suite with performance benchmarks
- **🔄 Range Queries**: Efficient range scan operations
- **💾 Memory Efficient**: Optimized memory layout and management

## 🏗️ Architecture

### Core Components

- **`btree.c`** - Pthread B+ Tree implementation
- **`btree.h`** - Public API and shared structures

### Concurrency

- Reader/writer locks at node level for high read throughput
- Lock coupling during descent to leaves
- Safe concurrent finds with concurrent inserts/deletes

## 🚀 Performance

### Benchmark Results (1000 items)

| Operation | Pthread |
|-----------|---------|
| Insertion | ~2.9M items/sec |
| Find | ~10.4M items/sec |
| Range Query | Working |

### Large Dataset Performance

- **100K items**: ~3.8M items/sec insertion
- **500K items**: ~3.1M items/sec insertion  
- **1M items**: ~2.7M items/sec insertion

## 🛠️ Building

### Prerequisites

- **Compiler**: Clang or GCC with C11 support
- **Platform**: macOS (ARM64/Apple Silicon or x86_64) or Linux
- **Build System**: Make

### Quick Start

```bash
# Clone the repository
git clone <repository-url>
cd ai-btree

# Build
make all

# Run tests
make test

# Run specific tests
make test-pthread
```

### Build Targets

```bash
make all                    # Build
make test                   # Run pthread tests
make test-pthread          # Run pthread tests
make test-safe-performance # Safe performance tests
make clean                 # Clean build artifacts
make help                  # Show available targets
```

## 💾 File Format & Serialization

### Magic Number System

Both B-tree and linked list implementations use a **magic number system** to identify file types and ensure data integrity. This allows you to distinguish between different data structure files and validate their format.

#### Magic Numbers

- **B-tree files**: `0x42545245` ("BTRE" in ASCII)
- **Linked list files**: `0x4C4C4953` ("LLIS" in ASCII)

#### File Format Structure

**B-tree files** contain:
```
[BTreeHeader] [NodeHeader] [NodeData] [NodeHeader] [NodeData] ...
```

**Linked list files** contain:
```
[LListHeader] [LListNodeHeader] [NodeData] [LListNodeHeader] [NodeData] ...
```

#### Header Information

Each file includes metadata such as:
- **Magic number** for file type identification
- **Version number** for format compatibility
- **Record counts** and **checksums** for data integrity
- **Node information** for reconstruction

#### Usage Example

```c
#include "btree.h"
#include "llist.h"

// Save B-tree to file
BPlusTree *tree = bplus_tree_create_with_serializers(16, compare_ints, NULL,
                                                    serialize_int_key, deserialize_int_key,
                                                    serialize_string_value, deserialize_string_value);
// ... populate tree ...
bplus_tree_save_to_file(tree, "data.btree");

// Save linked list to file
LinkedList *list = llist_create_with_serializer(NULL, serialize_int_data, deserialize_int_data);
// ... populate list ...
llist_save_to_file(list, "data.llist");

// Load and automatically detect file type
BPlusTree *loaded_tree = bplus_tree_load_from_file("data.btree", compare_ints, NULL,
                                                  deserialize_int_key, deserialize_string_value);
LinkedList *loaded_list = llist_load_from_file("data.llist", NULL, deserialize_int_data);
```

#### File Type Detection

The system automatically detects file types when loading:
- **B-tree files** are loaded with `bplus_tree_load_from_file()`
- **Linked list files** are loaded with `llist_load_from_file()`
- **Invalid files** (wrong magic number) will fail to load with an error

#### Data Integrity

- **Checksums** verify data hasn't been corrupted
- **Version checking** ensures compatibility with future format changes
- **Magic number validation** prevents loading wrong file types

## 📚 Usage

### Basic Operations

```c
#include "btree.h"

// Create a pthread-based B+ Tree
BPlusTree *tree = bplus_tree_create(16, compare_ints, NULL);

// Insert key-value pairs
int key = 42;
char *value = "hello";
bplus_tree_insert(tree, &key, value);

// Find values
void *found = bplus_tree_find(tree, &key);

// Range queries
void* results[100];
int count = bplus_tree_find_range(tree, &start_key, &end_key, results, 100);

// Cleanup
bplus_tree_destroy(tree);
```

### Quick Start (full)

```c
#include "btree.h"

int main() {
    BPlusTree *tree = bplus_tree_create(8, compare_ints, NULL);
    int key = 123; char *val = strdup("example");
    bplus_tree_insert(tree, &key, val);
    char *found = bplus_tree_find(tree, &key);
    if (found) printf("found: %s\n", found);
    bplus_tree_delete(tree, &key);
    bplus_tree_destroy(tree);
    return 0;
}
```

### Range Query Example

```c
// Assumes tree populated with integer keys
int start = 100, end = 200;
void *results[64];
int n = bplus_tree_find_range(tree, &start, &end, results, 64);
for (int i = 0; i < n; i++) {
    printf("val[%d]=%s\n", i, (char*)results[i]);
}
```

### Concurrency Pattern (Reads/Writes)

- Reads use shared (RD) locks internally and can proceed concurrently.
- Writes (insert/delete) take WR locks per node.
- Multiple threads can call `find` safely alongside writers.


### Memory Management Notes

- If your values are heap-allocated and owned by the tree, pass a `destroy_value` function at create time (e.g., `free`).
- If you manage values externally, pass `NULL` to avoid double frees.
- Keys are not copied; store pointers with lifetimes that outlive tree operations.

### Using BSON values (libbson)

You can store `bson_t*` documents as values. Pass a destroyer that calls `bson_destroy` so the tree cleans up documents.

Important: A B+ tree indexes by a single key. When you insert a `bson_t*` as the value, you must choose exactly one field to use as the B+ tree key. Queries by other BSON fields are not automatic; to "search by another field" you either:

- Build a second B+ tree keyed by that other field (secondary index), or
- Scan over a key range and filter each BSON document in user code.

Build requirements:

- Install mongo-c-driver (provides libbson). On macOS: `brew install mongo-c-driver`.
- Run the dedicated test target to verify setup: `make test-bson`.

Minimal example:

```c
#include <bson/bson.h>
#include "btree.h"

static void destroy_bson_value(void* value) {
    if (value) bson_destroy((bson_t*)value);
}

static bson_t* make_doc(int num, const char* name) {
    bson_t* d = bson_new();
    BSON_APPEND_INT32(d, "number", num);
    BSON_APPEND_UTF8(d, "name", name);
    return d;
}

int main() {
    // Choose which BSON field is the index (key). Here we index by an integer field "number".
    // Keys are stored separately from BSON docs. Values are full BSON docs.
    BPlusTree* tree = bplus_tree_create(DEFAULT_ORDER, compare_ints, destroy_bson_value);

    int *k1 = malloc(sizeof(int)), *k2 = malloc(sizeof(int));
    *k1 = 10; *k2 = 20; // keys derived from BSON field "number"
    bson_t* d1 = make_doc(*k1, "alpha");
    bson_t* d2 = make_doc(*k2, "bravo");

    bplus_tree_insert(tree, k1, d1);
    bplus_tree_insert(tree, k2, d2);

    // Find by the indexed field only (here: number == 20)
    const bson_t* found = (const bson_t*)bplus_tree_find(tree, k2);
    // read a field
    bson_iter_t it; const char* name = NULL;
    if (bson_iter_init_find(&it, found, "name") && BSON_ITER_HOLDS_UTF8(&it)) {
        name = bson_iter_utf8(&it, NULL);
    }

    // delete by key (again, only the indexed field)
    bplus_tree_delete(tree, k1);

    // scan a range and "search by other field" by filtering results in user code
    int start = 0, end = 10; void* results[16];
    int n = bplus_tree_find_range(tree, &start, &end, results, 16);
    for (int i = 0; i < n; i++) {
        const bson_t* doc = (const bson_t*)results[i];
        if (bson_iter_init_find(&it, doc, "name") && BSON_ITER_HOLDS_UTF8(&it)) {
            if (strcmp(bson_iter_utf8(&it, NULL), "bravo") == 0) {
                // match
            }
        }
    }

    bplus_tree_destroy(tree);
    free(k1); free(k2);
    return 0;
}
```

Indexing by a different BSON field (e.g., string field `name`) – full example:

```c
#include <bson/bson.h>
#include <string.h>
#include "btree.h"

static void destroy_bson_value(void* value) {
    if (value) bson_destroy((bson_t*)value);
}

static int compare_strings(const void* a, const void* b) {
    return strcmp((const char*)a, (const char*)b);
}

static bson_t* make_doc(int number, const char* name) {
    bson_t* d = bson_new();
    BSON_APPEND_INT32(d, "number", number);
    BSON_APPEND_UTF8(d, "name", name);
    return d;
}

int main() {
    // This tree indexes by the BSON string field "name"
    BPlusTree* by_name = bplus_tree_create(DEFAULT_ORDER, compare_strings, destroy_bson_value);

    // Keys are C strings (name). Values are full BSON documents
    char* kAlpha = strdup("alpha");
    char* kBravo = strdup("bravo");
    bson_t* dAlpha = make_doc(1, kAlpha);
    bson_t* dBravo = make_doc(2, kBravo);

    bplus_tree_insert(by_name, kAlpha, dAlpha);
    bplus_tree_insert(by_name, kBravo, dBravo);

    // Find by the indexed field (name)
    const bson_t* found = (const bson_t*)bplus_tree_find(by_name, kBravo);
    // Optional: read another field from the found doc
    bson_iter_t it; int number = -1;
    if (found && bson_iter_init_find(&it, found, "number") && BSON_ITER_HOLDS_INT32(&it)) {
        number = bson_iter_int32(&it);
    }

    // Delete by the indexed field
    bplus_tree_delete(by_name, kAlpha);

    // Range query over string keys (e.g., ["a", "c"]) and manual filtering
    const char* start = "a"; const char* end = "c";
    void* results[16];
    int n = bplus_tree_find_range(by_name, (void*)start, (void*)end, results, 16);
    for (int i = 0; i < n; i++) {
        const bson_t* doc = (const bson_t*)results[i];
        // process doc...
    }

    // Cleanup: tree frees BSON docs via destroyer; free string keys if you own them
    bplus_tree_destroy(by_name);
    free(kAlpha);
    free(kBravo);
    return 0;
}
```

## 📘 API Reference

Header `btree.h`:

- `BPlusTree *bplus_tree_create(int order, key_comparator cmp, value_destroyer dtor)`
- `int bplus_tree_insert(BPlusTree *t, void *key, void *value)`
- `void *bplus_tree_find(BPlusTree *t, void *key)`
- `int bplus_tree_find_range(BPlusTree *t, void *start_key, void *end_key, void **results, int max_results)`
- `int bplus_tree_delete(BPlusTree *t, void *key)`
- `void bplus_tree_destroy(BPlusTree *t)`

## 🧪 Testing

### Test Suite

- **Unit Tests**: Basic functionality, edge cases, error handling
- **Performance Tests**: Benchmarking insertion, find, and range operations
- **Stress Tests**: Large datasets (100K to 1M items)
- **Concurrency Tests**: Thread safety and concurrent access
- **Memory Tests**: Memory leak detection and cleanup

### Running Tests

```bash
# Run all tests
make test
```

## 🔧 Configuration

### Compiler Flags

- **Optimization**: `-O2` for production, `-O0` for debugging
- **Threading**: `-lpthread` for pthread support



## 📊 Performance Tuning

### Tree Order

- **Small datasets**: Order 4-8 for better cache locality
- **Large datasets**: Order 16-32 for reduced tree height
- **Memory vs Speed**: Higher order = faster but more memory


### Debug Mode

```bash
# Build with debug symbols
make CFLAGS="-g -O0 -DDEBUG"

# Run with debug output
export DEBUG=1
make test
```

## 🤝 Contributing

### Development Setup

1. Fork the repository
2. Create a feature branch
3. Add tests for new functionality
4. Ensure all tests pass
5. Submit a pull request

### Code Style

- **C11 Standard**: Use modern C features
- **Naming**: Snake_case for functions, PascalCase for types
- **Documentation**: Comprehensive comments for public APIs







---

