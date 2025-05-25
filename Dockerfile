# Base image
FROM ubuntu:22.04

# Set non-interactive mode for apt
ENV DEBIAN_FRONTEND=noninteractive
ENV PATH="/root/.cargo/bin:${PATH}"

# Set directory environment variables
ENV PROJECT_ROOT=/crop-tracking
ENV RUST_DIR=${PROJECT_ROOT}/rust
ENV SCRIPTS_DIR=${PROJECT_ROOT}/scripts
ENV FONTS_DIR=${PROJECT_ROOT}/fonts

# Set working directory to project root
WORKDIR ${PROJECT_ROOT}

# Install build dependencies (NO protobuf, NO protobuf-c)
RUN apt-get update && apt-get install -y \
    build-essential \
    bash \
    autoconf \
    automake \
    libtool \
    curl \
    git \
    pkg-config \
    cmake \
    g++ \
    gdb \
    make \
    rsync \ 
    libcjson-dev \ 
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# Install Rust via rustup
RUN curl https://sh.rustup.rs -sSf | bash -s -- -y

WORKDIR ${PROJECT_ROOT}
RUN git clone https://github.com/roguestarslade/crop-tracking.git /tmp/clone && \
    rsync -a /tmp/clone/ ${PROJECT_ROOT}/ && \
    rm -rf /tmp/clone

# Copy source code into the container
COPY rust/ ${RUST_DIR}
COPY scripts/ ${SCRIPTS_DIR}
COPY entrypoint.sh ${PROJECT_ROOT}/entrypoint.sh
COPY fonts/ ${FONTS_DIR}

# Build Rust tools (assumes you are NOT using .proto files anymore)
WORKDIR ${RUST_DIR}
RUN echo "🚧 Building Rust tools..." && \
    cargo build --release && \
    echo "✅ Rust build complete."

# Finalize build output in /crop-tracking/bin
# Collect final binaries
WORKDIR ${PROJECT_ROOT}
RUN mkdir -p bin && \
    cp ${RUST_DIR}/target/release/build-simple-input-data bin/ && \
    cp ${RUST_DIR}/target/release/build-noisy-as-fuck-input-data bin/ && \
    cp ${RUST_DIR}/target/release/build-moving-square bin/ && \
    cp ${RUST_DIR}/target/release/crop-tracking bin/

WORKDIR ${PROJECT_ROOT}
COPY .env ${PROJECT_ROOT}/.env

# Restore working directory and set entrypoint
WORKDIR ${PROJECT_ROOT}
ENTRYPOINT ["${PROJECT_ROOT}/bin/crop-tracking"]
