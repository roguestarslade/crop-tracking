# Base image
FROM ubuntu:22.04

# Set working directory
WORKDIR /app

# Environment
ENV DEBIAN_FRONTEND=noninteractive
ENV PATH="/root/.cargo/bin:${PATH}"

# Install system dependencies
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

# Copy and build Rust binaries
COPY rust/ /app/rust/
RUN cd /app/rust && cargo build --release

# Copy and build C binary
COPY c/ /app/c/
RUN make -C /app/c

# Copy final executables to /app/bin
RUN mkdir -p /app/bin \
    && cp /app/rust/target/release/build-simple-input-data /app/bin/ \
    && cp /app/rust/target/release/build-noisy-as-fuck-input-data /app/bin/ \
    && cp /app/c/tracking-solution /app/bin/

# Copy entrypoint
COPY entrypoint.sh /app/entrypoint.sh
RUN chmod +x /app/entrypoint.sh

# Set entrypoint
ENTRYPOINT ["/app/entrypoint.sh"]
