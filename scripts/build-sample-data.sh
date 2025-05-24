#!/bin/bash
set -e

echo "🧹 Cleaning old build outputs..."
PROJECT_ROOT="/crop-tracking"
RUST_DIR="${PROJECT_ROOT}/rust"
C_DIR="${PROJECT_ROOT}/c"
BIN_DIR="${PROJECT_ROOT}/bin"

# Wipe Rust target to force full rebuild
rm -rf "${RUST_DIR}/target"

echo "🔨 Rebuilding Rust tools..."
cd "${RUST_DIR}"
cargo build --release

echo "🔨 Rebuilding C tracker..."
cd "${C_DIR}/build"
cmake --build .

echo "📦 Installing binaries to $BIN_DIR..."
mkdir -p "$BIN_DIR"
cp "${RUST_DIR}/target/release/build-simple-input-data" "$BIN_DIR/"
cp "${RUST_DIR}/target/release/build-noisy-as-fuck-input-data" "$BIN_DIR/"
cp "${C_DIR}/build/tracking-solution" "$BIN_DIR/"

echo "✅ All binaries rebuilt and copied to $BIN_DIR"
