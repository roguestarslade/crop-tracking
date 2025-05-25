#!/bin/bash
set -e

BIN_PATH="./bin/tracking-solution"
INPUT="/crop-tracking/data/input-moving-square.json"
OUTPUT="/crop-tracking/data/output-simple.json"
VIS_DIR="/crop-tracking/data/visualization"
LOG_FILE="/crop-tracking/data/tracking-run.log"

echo "🚀 Running tracking-solution..."
$BIN_PATH --input "$INPUT" --output "$OUTPUT" --vis-dir "$VIS_DIR" | tee "$LOG_FILE"
