#!/bin/bash

set -e
set -x  # ⬅️ Shell trace ON

echo "🐳 Building Docker image: crop-tracking"

start=$(date +%s)

docker build \
  --progress=plain \
  --tag crop-tracking \
  --file Dockerfile \
  .

end=$(date +%s)
duration=$(( end - start ))

echo "✅ Done building 'crop-tracking' container in ${duration}s"
