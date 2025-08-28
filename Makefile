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
BTREE_BSON_TEST = $(TESTDIR)/btree_bson_test

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

# Link BSON value test (requires libbson)
# Try pkg-config first; if not found, attempt common Homebrew paths
PKGCONF_BSON_CFLAGS := $(shell pkg-config --cflags libbson-1.0 2>/dev/null)
PKGCONF_BSON_LIBS := $(shell pkg-config --libs libbson-1.0 2>/dev/null)

ifeq ($(PKGCONF_BSON_CFLAGS),)
  # Fallback includes/libs (Homebrew/Mac)
  BREW_BSON_PREFIX := $(shell brew --prefix mongo-c-driver 2>/dev/null)
  ifneq ($(BREW_BSON_PREFIX),)
    BSON_INC_FLAGS = -I$(BREW_BSON_PREFIX)/include/bson-2.1.0 -I$(BREW_BSON_PREFIX)/include
    BSON_LIB_FLAGS = -L$(BREW_BSON_PREFIX)/lib -lbson2
  else
    BSON_INC_FLAGS = -I/opt/homebrew/include -I/usr/local/include -I/opt/homebrew/include/libbson-1.0 -I/usr/local/include/libbson-1.0
    BSON_LIB_FLAGS = -L/opt/homebrew/lib -L/usr/local/lib -lbson-1.0
  endif
else
  BSON_INC_FLAGS = $(PKGCONF_BSON_CFLAGS)
  BSON_LIB_FLAGS = $(PKGCONF_BSON_LIBS)
endif

$(BTREE_BSON_TEST): $(BTREE_OBJ)
	$(CC) $(CFLAGS) $(BSON_INC_FLAGS) -o $@ $(TESTDIR)/btree_bson_test.c $^ $(LDFLAGS) $(BSON_LIB_FLAGS)

 





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

test-bson: $(BTREE_BSON_TEST)
	@echo "🧪 Running BSON value tests..."
	@$(BTREE_BSON_TEST)



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
	rm -f $(BTREE_BSON_TEST)
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

.PHONY: all test test-pthread test-bson clean help
