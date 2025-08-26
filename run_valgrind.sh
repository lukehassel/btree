#!/bin/bash

# Script to run B+ tree tests with Valgrind for memory leak detection
# This requires Valgrind to be installed on the system

echo "=========================================="
echo "Running B+ Tree Tests with Valgrind"
echo "=========================================="

# Check if Valgrind is available
if ! command -v valgrind &> /dev/null; then
    echo "❌ Error: Valgrind is not installed or not in PATH"
    echo "Please install Valgrind first:"
    echo "  - macOS: brew install valgrind"
    echo "  - Ubuntu/Debian: sudo apt-get install valgrind"
    echo "  - CentOS/RHEL: sudo yum install valgrind"
    exit 1
fi

# Create output directory
mkdir -p output

# Compile the test program
echo "🔨 Compiling test program..."
gcc -std=c11 -Wall -Wextra -O2 -g btree_test.c btree.c btree_viz.c -o btree_test

if [ $? -ne 0 ]; then
    echo "❌ Compilation failed!"
    exit 1
fi

echo "✅ Compilation successful!"

# Run tests with Valgrind
echo ""
echo "🔍 Running tests with Valgrind memory leak detection..."
echo "This may take a while..."

valgrind \
    --tool=memcheck \
    --leak-check=full \
    --show-leak-kinds=all \
    --track-origins=yes \
    --verbose \
    --log-file=valgrind_output.log \
    ./btree_test

# Check Valgrind exit code
VALGRIND_EXIT_CODE=$?

echo ""
echo "=========================================="
echo "Valgrind Analysis Complete"
echo "=========================================="

if [ $VALGRIND_EXIT_CODE -eq 0 ]; then
    echo "✅ Valgrind completed successfully"
else
    echo "⚠️  Valgrind detected issues (exit code: $VALGRIND_EXIT_CODE)"
fi

# Show summary of findings
echo ""
echo "📋 Valgrind Summary:"
echo "Full log saved to: valgrind_output.log"

# Check for memory leaks in the log
if grep -q "definitely lost:" valgrind_output.log; then
    echo "❌ Memory leaks detected!"
    echo "Definitely lost memory:"
    grep "definitely lost:" valgrind_output.log
else
    echo "✅ No definitely lost memory detected"
fi

if grep -q "indirectly lost:" valgrind_output.log; then
    echo "⚠️  Indirectly lost memory detected:"
    grep "indirectly lost:" valgrind_output.log
else
    echo "✅ No indirectly lost memory detected"
fi

if grep -q "possibly lost:" valgrind_output.log; then
    echo "⚠️  Possibly lost memory detected:"
    grep "possibly lost:" valgrind_output.log
else
    echo "✅ No possibly lost memory detected"
fi

echo ""
echo "💡 Tips:"
echo "- Check valgrind_output.log for detailed information"
echo "- Look for 'definitely lost' entries as these are true memory leaks"
echo "- 'indirectly lost' and 'possibly lost' may indicate issues with cleanup"
echo "- Run with --track-origins=yes to see where uninitialized values come from"

exit $VALGRIND_EXIT_CODE
