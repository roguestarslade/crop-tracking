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

WORKDIR ${PROJECT_ROOT}
COPY .env ${PROJECT_ROOT}/.env

# Set init test data script
WORKDIR ${PROJECT_ROOT}
COPY entrypoint.sh ${PROJECT_ROOT}/build-test-data.sh
RUN chmod +x ${PROJECT_ROOT}/build-test-data.sh
RUN ${PROJECT_ROOT}/build-test-data.sh

# Set entrypoint script executable
COPY entrypoint.sh ${PROJECT_ROOT}/entrypoint.sh
RUN chmod +x ${PROJECT_ROOT}/entrypoint.sh

# Restore working directory and set entrypoint
WORKDIR ${PROJECT_ROOT}
ENTRYPOINT ["./entrypoint.sh"]
