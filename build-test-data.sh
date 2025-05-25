#!/bin/bash
set -e
set -a
. /crop-tracking/.env
set +a

export C_DIR="$PROJECT_ROOT/c"
export RUST_DIR="$PROJECT_ROOT/rust"
export PROTO_DIR="$PROJECT_ROOT/proto"
export SCRIPTS_DIR="$PROJECT_ROOT/scripts"
export DATA_DIR="$PROJECT_ROOT/data"
export BIN_DIR="$PROJECT_ROOT/bin"

REPO_URL="https://github.com/roguestarslade/crop-tracking.git"

echo "🚀 Running input data generators..."
echo "🚀 Running build-simple-input-data..."
"$BIN_DIR/build-simple-input-data"
echo "🚀 Running build-noisy-as-fuck-input-data..."
"$BIN_DIR/build-noisy-as-fuck-input-data"

echo "✅ All input data generated."
