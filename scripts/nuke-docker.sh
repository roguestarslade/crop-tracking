#!/bin/bash

set -e

echo "🧨 Shutting down all running containers..."
docker ps -q | xargs -r docker stop

echo "💀 Removing all containers..."
docker ps -aq | xargs -r docker rm

echo "🧹 Pruning all networks, volumes, and dangling images..."
docker system prune -a --volumes -f

echo "✅ Docker environment nuked. Clean slate :)"
