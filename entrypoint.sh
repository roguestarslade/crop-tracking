#!/bin/bash
set -e

CMD="$(basename "$0")"

case "$CMD" in
  tracking-solution)
    exec /app/bin/tracking-solution "$@"
    ;;

  build-simple-input-data)
    exec /app/bin/build-simple-input-data "$@"
    ;;

  build-noisy-as-fuck-input-data)
    exec /app/bin/build-noisy-as-fuck-input-data "$@"
    ;;

  *)
    echo "Unknown command: $CMD :("
    echo "Expected one of: tracking-solution, build-simple-input-data, build-noisy-as-fuck-input-data"
    exit 1
    ;;
esac
