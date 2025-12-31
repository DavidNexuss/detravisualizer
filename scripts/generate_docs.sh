#!/bin/bash

DOXYGEN_CMD=$(which doxygen)

if [ -z "$DOXYGEN_CMD" ]; then
    echo "❌ Doxygen is not installed. Please install it first."
    exit 1
fi

echo "🛠️ Generating Doxygen documentation..."

# Generate the Doxyfile if missing
if [ ! -f Doxyfile ]; then
    echo "📄 Creating default Doxyfile..."
    doxygen -g
fi

# Overwrite settings in Doxyfile
sed -i 's|^PROJECT_NAME.*|PROJECT_NAME = "nexcorp"|' Doxyfile
sed -i 's|^OUTPUT_DIRECTORY.*|OUTPUT_DIRECTORY = docs|' Doxyfile
sed -i 's|^RECURSIVE.*|RECURSIVE = YES|' Doxyfile
sed -i 's|^EXTRACT_ALL.*|EXTRACT_ALL = YES|' Doxyfile
sed -i 's|^GENERATE_LATEX.*|GENERATE_LATEX = NO|' Doxyfile
sed -i 's|^GENERATE_TREEVIEW.*|GENERATE_TREEVIEW = YES|' Doxyfile
sed -i 's|^INPUT.*|INPUT = src' Doxyfile
sed -i 's|^FILE_PATTERNS.*|FILE_PATTERNS = *.cpp *.h *.hpp|' Doxyfile
sed -i 's|^MARKDOWN_SUPPORT.*|MARKDOWN_SUPPORT = YES|' Doxyfile

# Optional: clear old docs
rm -rf docs/html

# Run doxygen
doxygen Doxyfile

echo "✅ Documentation generated at docs/html/index.html"
