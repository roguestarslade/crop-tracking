#!/bin/bash
set -a
. /crop-tracking/.env
set +a

export C_DIR="$PROJECT_ROOT/c"
export RUST_DIR="$PROJECT_ROOT/rust"
export PROTO_DIR="$PROJECT_ROOT/proto"
export SCRIPTS_DIR="$PROJECT_ROOT/scripts"
export DATA_DIR="$PROJECT_ROOT/data"

REPO_URL="https://github.com/roguestarslade/crop-tracking.git"


