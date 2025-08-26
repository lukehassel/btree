# 🚀 High-Performance B+ Tree Implementation

A modern, optimized B+ Tree implementation in C with SIMD optimizations, designed for high-performance applications on macOS and other Unix-like systems.

## ✨ Features

- **🚀 SIMD Optimizations**: Platform-specific SIMD instructions (ARM NEON for Apple Silicon, AVX2 for x86_64)
- **🧵 Thread-Safe**: Pthread read-write locks for concurrent access
- **⚡ High Performance**: Optimized algorithms with compiler auto-vectorization
- **🔧 Generic Design**: Void pointer interface for flexible key/value types
- **📊 Comprehensive Testing**: Extensive test suite with performance benchmarks
- **🔄 Range Queries**: Efficient range scan operations
- **💾 Memory Efficient**: Optimized memory layout and management

## 🏗️ Architecture

### Core Components

- **`btree.c`** - Base pthread B+ Tree implementation
- **`btree_simd.c`** - SIMD-optimized B+ Tree implementation
- **`btree.h`** - Common header with shared structures and interfaces
- **`btree_simd.h`** - SIMD-specific function declarations

### SIMD Optimizations

- **Compiler Auto-Vectorization**: Uses `#pragma clang loop vectorize(enable)`
- **Platform Detection**: Automatically detects and uses optimal SIMD instructions
- **Smart Algorithm Selection**: Binary search for large arrays, SIMD for small operations
- **Memory Access Patterns**: Optimized for cache locality

## 🚀 Performance

### Benchmark Results (1000 items)

| Operation | Pthread | SIMD | Improvement |
|-----------|---------|------|-------------|
| Insertion | 2.9M items/sec | 3.1M items/sec | +7% |
| Find | 10.4M items/sec | 11.2M items/sec | +8% |
| Range Query | Both working | Both working | Comparable |

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

# Build all implementations
make all

# Run tests
make test

# Run specific tests
make test-pthread      # Pthread implementation only
make test-simd         # SIMD implementation only
make test-simd-vs-pthread  # Performance comparison
```

### Build Targets

```bash
make all                    # Build all implementations
make test                   # Run all tests
make test-pthread          # Test pthread implementation
make test-simd             # Test SIMD implementation
make test-simd-vs-pthread  # Performance comparison
make test-safe-performance # Safe performance tests
make clean                 # Clean build artifacts
make help                  # Show available targets
```

## 📚 Usage

### Basic Operations

```c
#include "btree_simd.h"

// Create a SIMD-optimized B+ Tree
BPlusTree *tree = bplus_tree_create_simd(16, compare_ints, NULL);

// Insert key-value pairs
int key = 42;
char *value = "hello";
bplus_tree_insert_simd(tree, &key, value);

// Find values
void *found = bplus_tree_find_simd(tree, &key);

// Range queries
void* results[100];
int count = bplus_tree_find_range_simd(tree, &start_key, &end_key, results, 100);

// Cleanup
bplus_tree_destroy_simd(tree);
```

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
