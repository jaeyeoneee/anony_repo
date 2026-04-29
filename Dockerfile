# Dockerfile — reproducible EpochESS artifact environment.
#
# Build:   docker build -t epochess:ccs2026 .
# Run:     docker run --rm -v $PWD/artifact_output:/out epochess:ccs2026
# Contents: OS + build deps + source; `./reproduce.sh full` on entry.

FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=UTC

RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential \
        cmake \
        g++ \
        libboost-all-dev \
        libgmp-dev \
        libntl-dev \
        libsodium-dev \
        python3 \
        python3-matplotlib \
        python3-numpy \
        python3-pandas \
        ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /epochess
COPY . /epochess

RUN chmod +x reproduce.sh

# Default entrypoint runs the full artifact reproduction and copies output
# to /out (which should be bind-mounted by the user).
ENTRYPOINT ["/bin/bash", "-c", "\
    ./reproduce.sh full && \
    mkdir -p /out && \
    cp -r build/results /out/csv_results && \
    cp -r figures /out/figures && \
    echo '---' && \
    echo 'Artifact output at /out: csv_results/ and figures/'"]
