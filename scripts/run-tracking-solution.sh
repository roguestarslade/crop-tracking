#!/bin/bash
set -e

BIN_PATH="./bin/tracking-solution"
INPUT="/crop-tracking/data/input-simple.json"
OUTPUT="/crop-tracking/data/output-simple.json"
VIS_DIR="/crop-tracking/data/visualization"

echo "🚀 Running tracking-solution..."
$BIN_PATH --input "$INPUT" --output "$OUTPUT" --vis-dir "$VIS_DIR"
