# Makefile for B+ Tree implementations
# Supports both pthread and OpenMP versions

CC = clang
CFLAGS = -std=c11 -Wall -Wextra -O2 -g
LDFLAGS = -lpthread

# Platform-specific flags
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
# Ensure rwlock APIs are exposed on glibc and enable pthread at compile time
CFLAGS += -D_XOPEN_SOURCE=700 -pthread
LDFLAGS += -pthread
endif

# OpenMP flags for macOS with clang
OPENMP_CFLAGS = -Xpreprocessor -fopenmp -I/opt/homebrew/opt/libomp/include
OPENMP_LDFLAGS = -L/opt/homebrew/opt/libomp/lib -lomp

# Directories
SRCDIR = .
TESTDIR = tests
OBJDIR = obj

# Source files
BTREE_SRC = $(SRCDIR)/btree.c
TEST_UTILS_SRC = $(TESTDIR)/test_utils.c

# Object files
BTREE_OBJ = $(OBJDIR)/btree.o
TEST_UTILS_OBJ = $(OBJDIR)/test_utils.o

# Executables
BTREE_TEST = $(TESTDIR)/btree_test

BTREE_SIMPLE_PERF_TEST = $(TESTDIR)/btree_simple_performance_test
BTREE_BASIC_PERF_TEST = $(TESTDIR)/btree_basic_performance_test
BTREE_SAFE_PERF_TEST = $(TESTDIR)/btree_safe_performance_test

# Default target
all: $(BTREE_TEST)

# Create object directory
$(OBJDIR):
	mkdir -p $(OBJDIR)

# Compile pthread B+ tree
$(BTREE_OBJ): $(BTREE_SRC) | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@





# Compile test utilities
$(TEST_UTILS_OBJ): $(TEST_UTILS_SRC) | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Link pthread test
$(BTREE_TEST): $(BTREE_OBJ) $(TEST_UTILS_OBJ)
	$(CC) $(CFLAGS) -o $@ $(TESTDIR)/btree_test.c $^ $(LDFLAGS)

 





# Link simple performance test
$(BTREE_SIMPLE_PERF_TEST): $(BTREE_OBJ) $(TEST_UTILS_OBJ)
	$(CC) $(CFLAGS) -o $@ $(TESTDIR)/btree_simple_performance_test.c $^ $(LDFLAGS)

# Link basic performance test
$(BTREE_BASIC_PERF_TEST): $(BTREE_OBJ) $(TEST_UTILS_OBJ)
	$(CC) $(CFLAGS) -o $@ $(TESTDIR)/btree_basic_performance_test.c $^ $(LDFLAGS)

# Link safe performance test
$(BTREE_SAFE_PERF_TEST): $(BTREE_OBJ) $(TEST_UTILS_OBJ)
	$(CC) $(CFLAGS) -o $@ $(TESTDIR)/btree_safe_performance_test.c $^ $(LDFLAGS)





# Test targets
test: $(BTREE_TEST)
	@echo "🧵 Testing pthread implementation..."
	@$(BTREE_TEST)

test-pthread: $(BTREE_TEST)
	@echo "🧵 Testing pthread implementation..."
	@$(BTREE_TEST)



test-both: test

test-performance: $(BTREE_PERF_COMPARISON)
	@echo "🚀 Running performance comparison..."
	@$(BTREE_PERF_COMPARISON)

test-simple-performance: $(BTREE_SIMPLE_PERF_TEST)
	@echo "🚀 Running simple performance test..."
	@$(BTREE_SIMPLE_PERF_TEST)

test-basic-performance: $(BTREE_BASIC_PERF_TEST)
	@echo "🚀 Running basic performance test..."
	@$(BTREE_BASIC_PERF_TEST)

test-safe-performance: $(BTREE_SAFE_PERF_TEST)
	@echo "🚀 Running safe performance test..."
	@$(BTREE_SAFE_PERF_TEST)

 

test-simd-vs-pthread: $(BTREE_SIMD_VS_PTHREAD_TEST)
	@echo "🚀 Running SIMD vs Pthread performance comparison..."
	@$(BTREE_SIMD_VS_PTHREAD_TEST)



# Clean target
clean:
	rm -rf $(OBJDIR)
	rm -f $(BTREE_TEST)
	rm -rf $(TESTDIR)/*.dSYM
	rm -f $(TESTDIR)/*.o



# Help target
help:
	@echo "Available targets:"
	@echo "  all              - Build"
	@echo "  test             - Run pthread tests"
	@echo "  test-pthread     - Run pthread tests"
	@echo "  clean            - Remove build artifacts"
	@echo "  help             - Show this help message"

.PHONY: all test test-pthread clean help
