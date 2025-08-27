# 🚀 High-Performance B+ Tree (pthread) Implementation

[![CI](https://github.com/lukehassel/btree/actions/workflows/ci.yml/badge.svg?branch=stable)](https://github.com/lukehassel/btree/actions/workflows/ci.yml)

A robust, thread-safe B+ Tree implementation in C using pthread read-write locks for high concurrency on macOS and Linux.

## ✨ Features

- **🧵 Thread-Safe**: Pthread read-write locks for concurrent access
- **⚡ High Performance**: Optimized algorithms with compiler auto-vectorization
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

### Key Comparison Functions

```c
// Integer comparison
int compare_ints(const void* a, const void* b) {
    int int_a = *(int*)a;
    int int_b = *(int*)b;
    if (int_a < int_b) return -1;
    if (int_a > int_b) return 1;
    return 0;
}

// String comparison
int compare_strings(const void* a, const void* b) {
    return strcmp((char*)a, (char*)b);
}
```

### Memory Management Notes

- If your values are heap-allocated and owned by the tree, pass a `destroy_value` function at create time (e.g., `free`).
- If you manage values externally, pass `NULL` to avoid double frees.
- Keys are not copied; store pointers with lifetimes that outlive tree operations.

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

# Run specific test suites
make test-simd
make test-pthread
make test-simd-vs-pthread

# Performance testing
make test-safe-performance
```

## 🔧 Configuration

### Compiler Flags

- **Optimization**: `-O2` for production, `-O0` for debugging
- **SIMD Support**: Automatically detected and enabled
- **Threading**: `-lpthread` for pthread support

### Platform-Specific Optimizations

- **Apple Silicon (ARM64)**: ARM NEON SIMD instructions
- **Intel/AMD (x86_64)**: AVX2 SIMD instructions
- **Fallback**: Scalar operations for unsupported platforms

## 📊 Performance Tuning

### Tree Order

- **Small datasets**: Order 4-8 for better cache locality
- **Large datasets**: Order 16-32 for reduced tree height
- **Memory vs Speed**: Higher order = faster but more memory

### SIMD Thresholds

- **Minimum parallel size**: 1000 items for SIMD operations
- **Chunk size**: 100 items for parallel processing
- **Auto-vectorization**: Enabled for loops with 4+ iterations

## 🐛 Troubleshooting

### Common Issues

1. **Compilation Errors**: Ensure C11 support and proper compiler flags
2. **Performance Issues**: Check SIMD support and compiler optimizations
3. **Memory Leaks**: Use `destroy_value` function for custom value types
4. **Thread Safety**: Ensure proper locking when using concurrent access

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
- **Testing**: Maintain >90% test coverage

## 📄 License

This project is licensed under the MIT License - see the LICENSE file for details.

## 🙏 Acknowledgments

- **SIMD Optimizations**: Leveraging modern CPU instruction sets
- **Compiler Optimizations**: Clang auto-vectorization capabilities
- **B+ Tree Research**: Academic papers and performance studies
- **Open Source Community**: Tools and libraries that made this possible

## 📈 Roadmap

### Version 1.1
- [ ] Additional SIMD optimizations
- [ ] GPU acceleration support
- [ ] Persistent storage capabilities

### Version 1.2
- [ ] Advanced indexing strategies
- [ ] Compression algorithms
- [ ] Distributed tree support

### Version 2.0
- [ ] Multi-language bindings
- [ ] Cloud-native features
- [ ] Advanced analytics integration

---

**Built with ❤️ for high-performance computing**
