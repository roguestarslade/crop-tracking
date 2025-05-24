# Base image
FROM ubuntu:22.04

# Set non-interactive mode for apt
ENV DEBIAN_FRONTEND=noninteractive
ENV PATH="/root/.cargo/bin:${PATH}"

# Set directory environment variables
ENV PROJECT_ROOT=/crop-tracking
ENV C_DIR=${PROJECT_ROOT}/c
ENV RUST_DIR=${PROJECT_ROOT}/rust
ENV PROTO_DIR=${PROJECT_ROOT}/proto
ENV SCRIPTS_DIR=${PROJECT_ROOT}/scripts

# Set working directory to project root
WORKDIR ${PROJECT_ROOT}

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    curl \
    git \
    pkg-config \
    protobuf-compiler \
    gdb \
    make \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# Install Rust via rustup
RUN curl https://sh.rustup.rs -sSf | bash -s -- -y

# Copy source code into the container
COPY proto/ ${PROTO_DIR}
COPY rust/ ${RUST_DIR}
COPY c/ ${C_DIR}
COPY scripts/ ${SCRIPTS_DIR}
COPY entrypoint.sh ${PROJECT_ROOT}/entrypoint.sh

# Download stb_image_write.h into the C directory
RUN curl -sSfL -o ${C_DIR}/stb_image_write.h https://raw.githubusercontent.com/nothings/stb/master/stb_image_write.h

# Build Rust tools
WORKDIR ${RUST_DIR}
RUN echo $PROTO_DIR && cargo build

# Build C tracker
WORKDIR ${C_DIR}
RUN make

# Finalize build output in /crop-tracking/bin
WORKDIR ${PROJECT_ROOT}
RUN mkdir -p bin \
    && cp ${RUST_DIR}/target/release/build-simple-input-data bin/ \
    && cp ${RUST_DIR}/target/release/build-noisy-as-fuck-input-data bin/ \
    && cp ${C_DIR}/tracking-solution bin/

# Set entrypoint script executable
RUN chmod +x ${PROJECT_ROOT}/entrypoint.sh

# Restore working directory and set entrypoint
WORKDIR ${PROJECT_ROOT}
ENTRYPOINT ["./entrypoint.sh"]
