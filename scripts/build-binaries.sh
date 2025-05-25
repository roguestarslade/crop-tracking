#!/bin/bash
set -e

# Load environment
set -a
. /crop-tracking/.env
set +a

export RUST_DIR="$PROJECT_ROOT/rust"
export SCRIPTS_DIR="$PROJECT_ROOT/scripts"
export DATA_DIR="$PROJECT_ROOT/data"
export BIN_DIR="$PROJECT_ROOT/bin"

echo "🚧 Building Rust tools..."
cd "$RUST_DIR"
cargo build --release
echo "✅ Rust build complete."

mkdir -p "${BIN_DIR}"

cp "${RUST_DIR}/target/release/build-simple-input-data" "${BIN_DIR}/"
cp "${RUST_DIR}/target/release/build-noisy-as-fuck-input-data" "${BIN_DIR}/"
cp "${RUST_DIR}/target/release/build-moving-square" "${BIN_DIR}/"
cp "${RUST_DIR}/target/release/crop-tracking" "${BIN_DIR}/"

echo "✅ Build complete.  "
