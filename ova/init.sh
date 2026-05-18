#!/bin/bash

set -euo pipefail
set -x

LOGFILE=/tmp/genesys_install.log
exec > >(tee -a "$LOGFILE") 2>&1

DESKTOP_APP_DIR="$HOME/.local/share/applications/"
REPO_URL="https://github.com/rlcancian/Genesys-Simulator.git"
REPO_DIR="$HOME/Documents"

# releases do usuario final
USER_VERSION_FILE="$HOME/.genesys_user_version"
LATEST_RELEASE_API="https://api.github.com/repos/rlcancian/Genesys-Simulator/releases/latest"
USER_RELEASE_DOWNLOAD_URL="https://github.com/rlcancian/Genesys-Simulator/releases/latest/download/genesys-linux.tar.gz"

# configuracao dev
DEV_BRANCH_FILE="$HOME/.genesys_dev_branch"
DEFAULT_DEV_BRANCH="currentStable"

# escolha da branch dev
if [ ! -f "$DEV_BRANCH_FILE" ]; then

    CHOICE=$(gxmessage -center -buttons \
        "currentStable:0,WorkInProgress:1" \
        -print \
        $'Escolha qual branch deseja seguir para desenvolvimento.\n\nEssa configuração ficará salva em:\n'"$DEV_BRANCH_FILE")

    if [ "$CHOICE" = "WorkInProgress" ]; then
        echo "WorkInProgress" > "$DEV_BRANCH_FILE"
    else
        echo "$DEFAULT_DEV_BRANCH" > "$DEV_BRANCH_FILE"
    fi

    gxmessage $'Branch salva em:\n'"$DEV_BRANCH_FILE"
fi

DEV_BRANCH=$(cat "$DEV_BRANCH_FILE")

# repositorio dev
DEV_REPO_PATH="$REPO_DIR/Genesys-Dev"

# executaveis
GENESYS_GUI_APP_DISPLAY_NAME="GenESySQt"
GENESYS_GUI_APP_EXEC="genesys_qt_gui_application"
GENESYS_WEB_APP_EXEC="genesys_web_app"

ICON_NAME="genesysico.gif"

INSTALL_DIR="$HOME/.local/bin/"
ICON_DIR="$HOME/.local/share/icons/"

mkdir -p "$INSTALL_DIR"
mkdir -p "$ICON_DIR"
mkdir -p "$REPO_DIR"

# espera internet
until getent hosts github.com >/dev/null 2>&1; do
    sleep 1
done

# =========================
# REPOSITÓRIO DEV
# =========================

if [ ! -d "$DEV_REPO_PATH" ]; then

    gxmessage -buttons "" -timeout 9999 \
        "Clonando versão de desenvolvimento ($DEV_BRANCH)..." &
    PID=$!

    git clone -b "$DEV_BRANCH" "$REPO_URL" "$DEV_REPO_PATH"

    kill $PID 2>/dev/null || true
fi

# verifica atualizações dev sem atualizar automaticamente
cd "$DEV_REPO_PATH"

git fetch origin || true
git checkout "$DEV_BRANCH" || true

# =========================
# USER VIA GITHUB RELEASES
# =========================

INSTALLED_VERSION=$(cat "$USER_VERSION_FILE" 2>/dev/null || echo "none")

LATEST_VERSION=$(curl -s "$LATEST_RELEASE_API" \
    | grep '"tag_name"' \
    | cut -d '"' -f4)

if [[ "$INSTALLED_VERSION" != "$LATEST_VERSION" ]]; then

    gxmessage -buttons "" -timeout 9999 \
        $'Baixando nova versão do GenESyS...\n\nVersão: '"$LATEST_VERSION" &
    PID=$!

    TMP_DIR=$(mktemp -d)

    curl -L "$USER_RELEASE_DOWNLOAD_URL" \
        -o "$TMP_DIR/genesys-linux.tar.gz"

    tar -xzf "$TMP_DIR/genesys-linux.tar.gz" -C "$TMP_DIR"

    # para o serviço antes de atualizar
    systemctl --user stop genesys-web.service || true

    cp -af "$TMP_DIR/$GENESYS_GUI_APP_EXEC" "$INSTALL_DIR"
    cp -af "$TMP_DIR/$GENESYS_WEB_APP_EXEC" "$INSTALL_DIR"
    cp -af "$TMP_DIR/$ICON_NAME" "$ICON_DIR"

    chmod +x "$INSTALL_DIR/$GENESYS_GUI_APP_EXEC"
    chmod +x "$INSTALL_DIR/$GENESYS_WEB_APP_EXEC"

    mkdir -p "$DESKTOP_APP_DIR"

    printf '%s\n' \
        "[Desktop Entry]" \
        "Name=$GENESYS_GUI_APP_DISPLAY_NAME" \
        "Exec=$INSTALL_DIR/$GENESYS_GUI_APP_EXEC" \
        "Icon=$ICON_DIR/$ICON_NAME" \
        "Type=Application" \
        "Terminal=false" \
        "Categories=Development;" \
        > "${DESKTOP_APP_DIR}${GENESYS_GUI_APP_DISPLAY_NAME}.desktop"

    USER_SERVICE_DIR="$HOME/.config/systemd/user"
    mkdir -p "$USER_SERVICE_DIR"

    printf '[Unit]\nDescription=GenESyS Web Server\nAfter=network.target\n\n[Service]\nExecStart=%s/%s\nWorkingDirectory=%s\nRestart=always\n\n[Install]\nWantedBy=default.target\n' \
        "$INSTALL_DIR" \
        "$GENESYS_WEB_APP_EXEC" \
        "$INSTALL_DIR" \
        > "$USER_SERVICE_DIR/genesys-web.service"

    systemctl --user daemon-reload
    systemctl --user enable genesys-web.service
    systemctl --user restart genesys-web.service

    echo "$LATEST_VERSION" > "$USER_VERSION_FILE"

    rm -rf "$TMP_DIR"

    kill $PID 2>/dev/null || true
fi

# =========================
# NOTIFICAÇÃO PARA DEV
# =========================

cd "$DEV_REPO_PATH"

git fetch origin

LOCAL_DEV=$(git rev-parse HEAD)
REMOTE_DEV=$(git rev-parse origin/"$DEV_BRANCH")

if [[ "$LOCAL_DEV" != "$REMOTE_DEV" ]]; then

    if gxmessage \
        -buttons "Sim:0,Não:1" \
        -default Sim \
        $'Há uma nova versão disponível para desenvolvedores.\n\nAtualizar agora?'; then

        gxmessage -buttons "" -timeout 9999 \
            "Atualizando ambiente de desenvolvimento..." &
        PID=$!

        git pull origin "$DEV_BRANCH"

        kill $PID 2>/dev/null || true
    fi
fi