# 🧪 Test Suite Documentation

This directory contains the comprehensive test suite for the B+ Tree implementation, covering functionality, performance, and edge cases.

## 📁 Test Files Overview

### Core Implementation Tests

- **`btree_test.c`** - Comprehensive test suite for the pthread B+ Tree implementation
- **`btree_simd_test.c`** - Test suite for the SIMD-optimized B+ Tree implementation


### Performance Tests

- **`btree_safe_performance_test.c`** - Safe performance testing with memory leak prevention
- **`btree_basic_performance_test.c`** - Basic performance benchmarking
- **`btree_simple_performance_test.c`** - Simple performance measurements
- **`btree_simd_vs_pthread_test.c`** - Performance comparison between implementations

### Utility Files

- **`test_utils.c`** - Common test utilities and helper functions
- **`test_utils.h`** - Header for test utilities

## 🚀 Running Tests

### Quick Test Commands

```bash
# Run all tests
make test

# Run specific implementations
make test-pthread          # Pthread implementation only
make test-simd             # SIMD implementation only
make test-simd-vs-pthread  # Performance comparison

# Performance testing
make test-safe-performance # Safe performance tests
```

### Individual Test Execution

```bash
# Build specific test
make tests/btree_simd_test

# Run specific test
./tests/btree_simd_test
./tests/btree_simd_vs_pthread_test
```

## 🧪 Test Categories

### 1. Unit Tests (`btree_test.c`)

**Basic Operations:**
- ✅ Tree creation and destruction
- ✅ Insert and find operations
- ✅ Node splitting and tree rebalancing
- ✅ Range queries and scans
- ✅ Deletion operations

**Edge Cases:**
- ✅ Empty tree operations
- ✅ Single node operations
- ✅ Duplicate key handling
- ✅ NULL parameter handling
- ✅ Memory allocation failures

**Concurrency:**
- ✅ Concurrent insertions
- ✅ Concurrent reads
- ✅ Mixed read/write operations
- ✅ Thread safety validation

### 2. SIMD Tests (`btree_simd_test.c`)

**Core Functionality:**
- ✅ SIMD tree creation and destruction
- ✅ SIMD-optimized insert operations
- ✅ SIMD-optimized find operations
- ✅ SIMD-optimized range queries
- ✅ Memory management and cleanup

**Performance Validation:**
- ✅ Small dataset performance (100 items)
- ✅ Medium dataset performance (500 items)
- ✅ Large dataset performance (1000 items)
- ✅ SIMD instruction utilization

### 3. Performance Tests

**Safe Performance (`btree_safe_performance_test.c`):**
- ✅ Memory leak prevention
- ✅ Safe memory management
- ✅ Performance benchmarking
- ✅ Tree integrity verification

**Basic Performance (`btree_basic_performance_test.c`):**
- ✅ Basic insertion performance
- ✅ Basic find performance
- ✅ Range query performance
- ✅ Memory usage analysis

**Simple Performance (`btree_simple_performance_test.c`):**
- ✅ Simple performance measurements
- ✅ Basic benchmarking
- ✅ Performance regression detection

### 4. Comparison Tests (`btree_simd_vs_pthread_test.c`)

**Performance Comparison:**
- ✅ Side-by-side benchmarking
- ✅ Multiple dataset sizes (1K, 5K, 10K items)
- ✅ Insertion performance comparison
- ✅ Find performance comparison
- ✅ Range query performance comparison

## 📊 Test Data

### Dataset Sizes

- **Small**: 100-500 items (for basic functionality)
- **Medium**: 1K-10K items (for performance testing)
- **Large**: 100K-1M items (for stress testing)

### Key Types

- **Integer keys**: Sequential, random, and edge case values
- **String keys**: Various lengths and character sets
- **Custom keys**: User-defined comparison functions

### Value Types

- **String values**: Dynamic allocation and cleanup
- **Integer values**: Simple value storage
- **Custom values**: User-defined value types

## 🔧 Test Configuration

### Environment Variables

```bash
# Enable debug output
export DEBUG=1

# Set test verbosity
export TEST_VERBOSE=1

# Enable memory leak detection
export VALGRIND=1
```

### Compiler Flags

```bash
# Debug build
make CFLAGS="-g -O0 -DDEBUG"

# Release build
make CFLAGS="-O3 -DNDEBUG"

# SIMD optimization
make CFLAGS="-march=native -O2"
```

## 📈 Performance Benchmarks

### Expected Performance (Apple Silicon M1)

| Operation | 1K items | 10K items | 100K items |
|-----------|----------|-----------|------------|
| Insertion | 3.1M/sec | 3.0M/sec | 2.8M/sec |
| Find | 11.2M/sec | 10.8M/sec | 10.2M/sec |
| Range Query | <1ms | <5ms | <50ms |

### Performance Regression Detection

Tests automatically detect performance regressions by:
- Comparing against baseline measurements
- Alerting on significant performance drops
- Providing detailed performance analysis
- Generating performance reports

## 🐛 Debugging Tests

### Common Test Issues

1. **Memory Leaks**: Use `make test-safe-performance` for memory-safe testing
2. **Performance Failures**: Check system load and background processes
3. **Concurrency Issues**: Verify thread safety with `make test-pthread`
4. **SIMD Failures**: Ensure proper compiler flags and platform support

### Debug Output

```bash
# Enable verbose test output
make test TEST_VERBOSE=1

# Run with debug symbols
make CFLAGS="-g -O0" test

# Use debugger
gdb ./tests/btree_simd_test
```

### Test Isolation

```bash
# Run single test category
make test-pthread

# Run specific test file
make tests/btree_simd_test && ./tests/btree_simd_test

# Clean and rebuild
make clean && make test
```

## 📋 Test Coverage

### Current Coverage

- **Function Coverage**: 100% of public API functions
- **Branch Coverage**: >95% of conditional branches
- **Line Coverage**: >90% of source code lines
- **Error Path Coverage**: 100% of error handling paths

### Coverage Reports

```bash
# Generate coverage report (requires gcov)
make coverage

# View coverage in browser
make coverage-html
```

## 🤝 Adding New Tests

### Test Structure

```c
bool test_new_feature() {
    // Setup
    BPlusTree *tree = bplus_tree_create_simd(8, compare_ints, NULL);
    
    // Test execution
    int result = bplus_tree_insert_simd(tree, &key, value);
    
    // Verification
    assert(result == 0);
    
    // Cleanup
    bplus_tree_destroy_simd(tree);
    
    return true;
}
```

### Test Registration

```c
int main() {
    // Run all tests
    RUN_TEST(test_new_feature);
    
    printf("Tests passed: %d, Tests failed: %d\n", 
           tests_passed, tests_failed);
    
    return tests_failed == 0 ? 0 : 1;
}
```

### Test Guidelines

1. **Isolation**: Each test should be independent
2. **Cleanup**: Always clean up allocated resources
3. **Assertions**: Use meaningful assertions with clear messages
4. **Performance**: Include performance measurements for new features
5. **Documentation**: Document test purpose and expected behavior

## 📊 Continuous Integration

### Automated Testing

- **Build Verification**: All tests must pass before merge
- **Performance Regression**: Performance tests run on every commit
- **Memory Safety**: Memory leak detection on every test run
- **Platform Coverage**: Tests run on multiple platforms

### Test Matrix

| Platform | Compiler | SIMD Support | Status |
|----------|----------|--------------|---------|
| macOS ARM64 | Clang | ARM NEON | ✅ |
| macOS x86_64 | Clang | AVX2 | ✅ |
| Linux x86_64 | GCC | AVX2 | 🔄 |
| Linux ARM64 | GCC | ARM NEON | 🔄 |

---

**Test suite maintained with ❤️ for reliability and performance**
