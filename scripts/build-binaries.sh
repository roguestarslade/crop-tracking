#!/bin/bash
set -e

# Load environment
set -a
. /crop-tracking/.env
set +a

export C_DIR="$PROJECT_ROOT/c"
export RUST_DIR="$PROJECT_ROOT/rust"
export PROTO_DIR="$PROJECT_ROOT/proto"
export SCRIPTS_DIR="$PROJECT_ROOT/scripts"
export DATA_DIR="$PROJECT_ROOT/data"
export BIN_DIR="$PROJECT_ROOT/bin"

echo "🚧 Building Rust tools..."
cd "$RUST_DIR"
cargo build --release
echo "✅ Rust build complete."

echo "🔨 Building C tracker (no proto)..."
cd "$C_DIR"
mkdir -p build
cd build
cmake ..
cmake --build .
echo "✅ CMake C tracker build complete."

echo "📦 Collecting all built binaries into $BIN_DIR..."
mkdir -p "$BIN_DIR"
cp "$RUST_DIR/target/release/build-simple-input-data" "$BIN_DIR/"
cp "$RUST_DIR/target/release/build-noisy-as-fuck-input-data" "$BIN_DIR/"
cp "$C_DIR/build/tracking-solution" "$BIN_DIR/"
echo "✅ All binaries installed to $BIN_DIR"

echo "🚀 Running test data script: build-test-data.sh"
chmod +x "$PROJECT_ROOT/build-test-data.sh"
"$PROJECT_ROOT/build-test-data.sh"

echo "✅ Build complete.  "
