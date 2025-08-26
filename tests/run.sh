#!/bin/bash

# Comprehensive B+ Tree Test Runner
# This script runs all available tests including basic, race condition, and memory leak analysis

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    local color=$1
    local message=$2
    echo -e "${color}${message}${NC}"
}

# Function to print section headers
print_section() {
    echo ""
    echo "=========================================="
    print_status $BLUE "$1"
    echo "=========================================="
}

# Function to check if a command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Function to compile a test program
compile_test() {
    local test_name=$1
    local source_files=$2
    local output_name=$3
    
    print_status $CYAN "🔨 Compiling $test_name..."
    
    gcc -std=c11 -Wall -Wextra -O2 -g $source_files -o $output_name
    
    if [ $? -eq 0 ]; then
        print_status $GREEN "✅ $test_name compilation successful!"
        return 0
    else
        print_status $RED "❌ $test_name compilation failed!"
        return 1
    fi
}

# Function to run tests and capture results
run_test_suite() {
    local test_name=$1
    local executable=$2
    local description=$3
    
    print_status $CYAN "🧪 Running $test_name..."
    print_status $YELLOW "$description"
    
    if [ -f "$executable" ]; then
        ./$executable
        local exit_code=$?
        
        if [ $exit_code -eq 0 ]; then
            print_status $GREEN "✅ $test_name PASSED"
            return 0
        else
            print_status $RED "❌ $test_name FAILED (exit code: $exit_code)"
            return 1
        fi
    else
        print_status $RED "❌ $test_name executable not found!"
        return 1
    fi
}

# Function to run Valgrind analysis
run_valgrind_analysis() {
    local test_name=$1
    local executable=$2
    local log_file=$3
    
    if ! command_exists valgrind; then
        print_status $YELLOW "⚠️  Valgrind not available, skipping memory leak analysis for $test_name"
        return 0
    fi
    
    print_status $PURPLE "🔍 Running Valgrind analysis for $test_name..."
    
    valgrind \
        --tool=memcheck \
        --leak-check=full \
        --show-leak-kinds=all \
        --track-origins=yes \
        --verbose \
        --log-file="$log_file" \
        --quiet \
        ./$executable
    
    local valgrind_exit_code=$?
    
    # Check for memory leaks
    if grep -q "definitely lost:" "$log_file"; then
        print_status $RED "❌ Memory leaks detected in $test_name!"
        grep "definitely lost:" "$log_file"
    else
        print_status $GREEN "✅ No definitely lost memory in $test_name"
    fi
    
    if grep -q "indirectly lost:" "$log_file"; then
        print_status $YELLOW "⚠️  Indirectly lost memory in $test_name:"
        grep "indirectly lost:" "$log_file"
    fi
    
    if grep -q "possibly lost:" "$log_file"; then
        print_status $YELLOW "⚠️  Possibly lost memory in $test_name:"
        grep "possibly lost:" "$log_file"
    fi
    
    return $valgrind_exit_code
}

# Main execution
main() {
    print_section "🚀 B+ Tree Comprehensive Test Suite"
    
    # Create output directory
    mkdir -p output
    
    local all_tests_passed=true
    local test_results=()
    
    # Test 1: Basic Test Suite
    print_section "📋 Basic Test Suite"
    
    if compile_test "Basic Tests" "btree_test.c test_utils.c ../btree.c ../btree_viz.c" "btree_test"; then
        if run_test_suite "Basic Tests" "btree_test" "Running core functionality tests..."; then
            test_results+=("Basic Tests: ✅ PASSED")
        else
            test_results+=("Basic Tests: ❌ FAILED")
            all_tests_passed=false
        fi
    else
        test_results+=("Basic Tests: ❌ COMPILATION FAILED")
        all_tests_passed=false
    fi
    
    # Test 2: Race Condition Tests
    print_section "🏃 Race Condition Tests"
    
    if compile_test "Race Condition Tests" "btree_race_condition_tests.c test_utils.c ../btree.c ../btree_viz.c -lpthread" "btree_race_condition_tests"; then
        if run_test_suite "Race Condition Tests" "btree_race_condition_tests" "Running race condition and concurrency tests..."; then
            test_results+=("Race Condition Tests: ✅ PASSED")
        else
            test_results+=("Race Condition Tests: ❌ FAILED")
            all_tests_passed=false
        fi
    else
        test_results+=("Race Condition Tests: ❌ COMPILATION FAILED")
        all_tests_passed=false
    fi
    
    # Test 3: Valgrind Analysis (if available)
    print_section "🔍 Memory Leak Analysis with Valgrind"
    
    if command_exists valgrind; then
        print_status $GREEN "✅ Valgrind is available"
        
        # Run Valgrind on basic tests
        if [ -f "btree_test" ]; then
            run_valgrind_analysis "Basic Tests" "btree_test" "valgrind_basic.log"
        fi
        
        # Run Valgrind on race condition tests
        if [ -f "btree_race_condition_tests" ]; then
            run_valgrind_analysis "Race Condition Tests" "btree_race_condition_tests" "valgrind_race.log"
        fi
        
        print_status $CYAN "📋 Valgrind logs saved to:"
        print_status $CYAN "  - valgrind_basic.log"
        print_status $CYAN "  - valgrind_race.log"
    else
        print_status $YELLOW "⚠️  Valgrind not available"
        print_status $YELLOW "   Install with: brew install valgrind (macOS) or apt-get install valgrind (Ubuntu)"
    fi
    
    # Summary
    print_section "📊 Test Results Summary"
    
    for result in "${test_results[@]}"; do
        echo "$result"
    done
    
    echo ""
    if [ "$all_tests_passed" = true ]; then
        print_status $GREEN "🎉 All tests PASSED!"
        print_status $GREEN "✅ B+ Tree implementation is working correctly"
        echo ""
        print_status $CYAN "💡 Next steps:"
        print_status $CYAN "  - Review Valgrind logs for memory leaks"
        print_status $CYAN "  - Run individual test suites if needed"
        exit 0
    else
        print_status $RED "💥 Some tests FAILED!"
        print_status $RED "❌ Please review the failed tests above"
        echo ""
        print_status $YELLOW "🔧 Troubleshooting:"
        print_status $YELLOW "  - Check compilation errors"
        print_status $YELLOW "  - Review test assertions"
        print_status $YELLOW "  - Check for memory management issues"
        exit 1
    fi
}

# Run main function
main "$@"
