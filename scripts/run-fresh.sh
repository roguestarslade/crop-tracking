#!/bin/bash
set -e

echo "💣 Killing any running 'crop-tracking' containers..."
docker ps -q --filter "ancestor=crop-tracking:latest" | xargs -r docker kill

echo "🚀 Running crop-tracking:latest"
docker run -it crop-tracking:latest
