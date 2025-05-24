# Base image
FROM ubuntu:22.04

# Define build-time variable for the workspace path
ARG PROJECT_DIR=/crop-tracking

# Use it as the working directory
WORKDIR ${PROJECT_DIR}

# Environment setup
ENV DEBIAN_FRONTEND=noninteractive
ENV PATH="/root/.cargo/bin:${PATH}"

# Install build tools
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

# Install Rust
RUN curl https://sh.rustup.rs -sSf | bash -s -- -y

# Copy full repo in-place
COPY . ${PROJECT_DIR}

# Fetch stb_image_write.h into the C directory
RUN curl -sSfL -o ${PROJECT_DIR}/c/stb_image_write.h https://raw.githubusercontent.com/nothings/stb/master/stb_image_write.h

# Build Rust
RUN cd rust && cargo build --release

# Build C
RUN make -C c

# Move binaries
RUN mkdir -p bin \
    && cp rust/target/release/build-simple-input-data bin/ \
    && cp rust/target/release/build-noisy-as-fuck-input-data bin/ \
    && cp c/tracking-solution bin/

# Make entrypoint executable
RUN chmod +x entrypoint.sh

# Set entrypoint
ENTRYPOINT ["./entrypoint.sh"]
