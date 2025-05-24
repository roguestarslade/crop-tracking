# Base image
FROM ubuntu:22.04

# Set non-interactive mode for apt
ENV DEBIAN_FRONTEND=noninteractive
ENV PATH="/root/.cargo/bin:${PATH}"

# Set directory environment variables
ENV PROJECT_ROOT=/crop-tracking
ENV C_DIR=${PROJECT_ROOT}/c
ENV RUST_DIR=${PROJECT_ROOT}/rust
ENV SCRIPTS_DIR=${PROJECT_ROOT}/scripts

# Set working directory to project root
WORKDIR ${PROJECT_ROOT}

# Install build dependencies (NO protobuf, NO protobuf-c)
RUN apt-get update && apt-get install -y \
    build-essential \
    autoconf \
    automake \
    libtool \
    curl \
    git \
    cmake \
    g++ \
    pkg-config \
    gdb \
    make \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# Install Rust via rustup
RUN curl https://sh.rustup.rs -sSf | bash -s -- -y

# Copy source code into the container
COPY rust/ ${RUST_DIR}
COPY c/ ${C_DIR}
COPY scripts/ ${SCRIPTS_DIR}
COPY entrypoint.sh ${PROJECT_ROOT}/entrypoint.sh

# Download stb_image_write.h into the C directory
RUN curl -sSfL -o ${C_DIR}/stb_image_write.h https://raw.githubusercontent.com/nothings/stb/master/stb_image_write.h

# Build Rust tools (assumes you are NOT using .proto files anymore)
WORKDIR ${RUST_DIR}
RUN echo "🚧 Building Rust tools..." && \
    cargo build --release && \
    echo "✅ Rust build complete."

# Build C tracker (NO CMake dependency on protobuf-c)
WORKDIR ${C_DIR}
RUN mkdir -p build && cd build && \
    cmake .. && \
    cmake --build . && \
    echo "✅ CMake C tracker build complete."

# Finalize build output in /crop-tracking/bin
WORKDIR ${PROJECT_ROOT}
RUN mkdir -p bin \
    && cp ${RUST_DIR}/target/release/build-simple-input-data bin/ \
    && cp ${RUST_DIR}/target/release/build-noisy-as-fuck-input-data bin/ \
    && cp ${C_DIR}/build/tracking-solution bin/

# Set entrypoint script executable
RUN chmod +x ${PROJECT_ROOT}/entrypoint.sh

ARG USER_ID=1000
ARG GROUP_ID=1000
RUN useradd -u ${USER_ID} -m devuser && chown -R devuser:devuser /crop-tracking
USER devuser

# Restore working directory and set entrypoint
WORKDIR ${PROJECT_ROOT}
ENTRYPOINT ["./entrypoint.sh"]
