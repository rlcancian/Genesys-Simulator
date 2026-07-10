FROM ubuntu:24.04

ARG DEBIAN_FRONTEND=noninteractive
ARG GENESYS_REPOSITORY_URL
ARG GENESYS_RUNTIME_BRANCH=master
ARG GENESYS_BUILD_TYPE=Release

ENV LANG=C.UTF-8 LC_ALL=C.UTF-8

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential cmake git ninja-build pkg-config \
    python3 ca-certificates \
    libsbml5-dev ngspice r-base \
    qt6-base-dev qt6-base-dev-tools qt6-tools-dev qt6-tools-dev-tools \
    libgl1-mesa-dev libxkbcommon-x11-0 libxcb-cursor0 libxcb-icccm4 libxcb-image0 libxcb-keysyms1 libxcb-render-util0 libxcb-shape0 libxcb-xinerama0 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src
RUN test -n "${GENESYS_REPOSITORY_URL}"
RUN mkdir -p /src/genesys
WORKDIR /src/genesys
RUN git clone --branch "${GENESYS_RUNTIME_BRANCH}" --single-branch "${GENESYS_REPOSITORY_URL}" .

RUN cmake --preset terminal-app && \
    cmake --build --preset terminal-app --parallel "$(nproc)" && \
    cmake --preset gui-app && \
    cmake --build --preset gui-app --parallel "$(nproc)"

RUN bash -lc 'set -Eeuo pipefail; \
    for preset in gui-httpworker gui-dataanalyser gui-optimizer gui-ai-assistant; do \
      if cmake --list-presets=all | grep -q "${preset}"; then \
        cmake --preset "${preset}"; \
        cmake --build --preset "${preset}" --parallel "$(nproc)"; \
      fi; \
    done'

COPY scripts/runtime-entrypoint.sh /usr/local/bin/genesys-runtime
RUN chmod +x /usr/local/bin/genesys-runtime

ENTRYPOINT ["/usr/local/bin/genesys-runtime"]
