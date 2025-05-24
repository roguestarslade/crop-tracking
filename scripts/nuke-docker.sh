#!/bin/bash

set -e

echo "🧨 Shutting down all running containers..."
docker ps -q | xargs -r docker stop

echo "💀 Removing all containers..."
docker ps -aq | xargs -r docker rm

echo "🔥 Deleting all Docker images..."
docker images -aq | xargs -r docker rmi -f

echo "🧹 Removing all Docker volumes..."
docker volume ls -q | xargs -r docker volume rm

echo "🌐 Removing all user-defined Docker networks..."
docker network ls --filter "type=custom" -q | xargs -r docker network rm

echo "🧱 Pruning all build cache (buildkit + legacy)..."
docker builder prune --all --force
docker system prune --all --volumes --force

echo "✅ Docker ecosystem absolutely obliterated. Clean slate."
