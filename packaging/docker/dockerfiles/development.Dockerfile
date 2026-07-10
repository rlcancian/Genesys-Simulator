FROM ubuntu:24.04

ARG DEBIAN_FRONTEND=noninteractive

ENV LANG=C.UTF-8 LC_ALL=C.UTF-8

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential cmake git gdb ninja-build pkg-config \
    libsbml5-dev ngspice r-base \
    qt6-base-dev qt6-base-dev-tools qt6-tools-dev qt6-tools-dev-tools qtcreator \
    libgl1-mesa-dev libxkbcommon-x11-0 libxcb-cursor0 libxcb-icccm4 libxcb-image0 libxcb-keysyms1 libxcb-render-util0 libxcb-shape0 libxcb-xinerama0 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /workspace
CMD ["qtcreator", "/workspace/CMakeLists.txt"]
