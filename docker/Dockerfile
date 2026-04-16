FROM ubuntu:24.04

ARG USER_ID=1000
ARG GROUP_ID=1000

ENV DEBIAN_FRONTEND=noninteractive
ENV LANG=C.UTF-8
ENV LC_ALL=C.UTF-8
ENV QT_QPA_PLATFORM=xcb
ENV QT_X11_NO_MITSHM=1

RUN apt-get update && \
    apt-get install --no-install-recommends --assume-yes \
        build-essential \
        ca-certificates \
        ccache \
        clang \
        clangd \
        cmake \
        curl \
        dbus-x11 \
        gdb \
        gh \
        git \
        graphviz \
        libgl1 \
        libgl1-mesa-dri \
        libglx-mesa0 \
        libxcb-cursor0 \
        libxcb-icccm4 \
        libxcb-image0 \
        libxcb-keysyms1 \
        libxcb-render-util0 \
        libxcb-shape0 \
        libxcb-xinerama0 \
        libxcb-xkb1 \
        libxkbcommon-x11-0 \
        ninja-build \
        novnc \
        openbox \
        openssh-client \
        pkg-config \
        qt6-base-dev \
        qt6-base-dev-tools \
        qt6-l10n-tools \
        qt6-tools-dev \
        qt6-tools-dev-tools \
        qtcreator \
        valgrind \
        websockify \
        x11vnc \
        xterm \
        xvfb && \
    rm -rf /var/lib/apt/lists/*

RUN set -eux; \
    if ! getent group "${GROUP_ID}" >/dev/null; then \
        groupadd --gid "${GROUP_ID}" genesys; \
    fi; \
    if getent passwd "${USER_ID}" >/dev/null; then \
        existing_user="$(getent passwd "${USER_ID}" | cut -d: -f1)"; \
        if [ "${existing_user}" != "genesys" ]; then \
            usermod --login genesys "${existing_user}"; \
        fi; \
        usermod --gid "${GROUP_ID}" genesys; \
        usermod --home /home/genesys --move-home genesys || usermod --home /home/genesys genesys; \
        chsh --shell /bin/bash genesys; \
    else \
        useradd --uid "${USER_ID}" --gid "${GROUP_ID}" --create-home --shell /bin/bash genesys; \
    fi; \
    mkdir -p /home/genesys; \
    chown -R "${USER_ID}:${GROUP_ID}" /home/genesys

COPY scripts/container-entrypoint.sh /usr/local/bin/genesys-container
RUN chmod +x /usr/local/bin/genesys-container

USER genesys
WORKDIR /workspace

ENTRYPOINT ["/usr/local/bin/genesys-container"]
