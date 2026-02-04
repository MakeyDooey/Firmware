FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    gcc-arm-none-eabi \
    libnewlib-arm-none-eabi \
    binutils-arm-none-eabi \
    ccache \
    cppcheck \
    clang-format \
    && apt-get clean && rm -rf /var/lib/apt/lists/*

# Professional ccache tuning for Firmware
# pch_defines: allows cache hits even when using Precompiled Headers
# time_macros: prevents cache misses if __DATE__ or __TIME__ are used
ENV CCACHE_SLOPPINESS=pch_defines,time_macros
ENV CCACHE_DIR=/workspace/.ccache
ENV PATH="/usr/lib/ccache:$PATH"

WORKDIR /workspace

CMD ["make", "all", "-j$(nproc)", "OPT=small"]
