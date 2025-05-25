#!/bin/bash

set -e
set -x  # ⬅️ Shell trace ON

echo "🐳 Building Docker image: tracking-solution"

start=$(date +%s)

docker build \
  --progress=plain \
  --tag tracking-solution \
  --file Dockerfile \
  .

end=$(date +%s)
duration=$(( end - start ))

docker build -t tracking-solution:latest .

echo "✅ Done building 'tracking-solution' container in ${duration}s"
