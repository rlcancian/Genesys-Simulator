#!/usr/bin/env bash
set -euo pipefail

# -------- CONFIGURÁVEIS --------
USER_NAME="${SUDO_USER:-$(logname 2>/dev/null || echo vboxuser)}"
USER_HOME="/home/$USER_NAME"
KEYBOARD_CONF="/etc/default/keyboard"
ZERO_FILL="${ZERO_FILL:-1}"   # 1 = habilita dd zero-fill
# --------------------------------

require_root() {
  if [ "$(id -u)" -ne 0 ]; then
    echo "Execute como root: use 'su -' ou 'sudo -i'"
    exit 1
  fi
}

install_clion() {
  echo "[+] Instalando CLion"

  sudo -u "$USER_NAME" bash <<EOF
set -e

cd "$USER_HOME"

wget -O - https://download.jetbrains.com/cpp/CLion-2026.1.tar.gz | tar -xz
mv clion-* clion

# Criar atalho no menu (Development)
mkdir -p ~/.local/share/applications

cat > ~/.local/share/applications/clion.desktop <<EOL
[Desktop Entry]
Name=CLion
Exec=$USER_HOME/clion/bin/clion.sh
Icon=$USER_HOME/clion/bin/clion.png
Type=Application
Categories=Development;IDE;
Terminal=false
EOL

EOF

  chown -R "$USER_NAME:$USER_NAME" "$USER_HOME/clion"
  chown -R "$USER_NAME:$USER_NAME" "$USER_HOME/.local"
}

install_sudo_and_user() {
  echo "[+] Instalando sudo e configurando usuário (${USER_NAME})"
  apt update
  apt install -y sudo
  usermod -aG sudo "${USER_NAME}" || true
}

install_gui() {
  echo "[+] Instalando Xorg + LXDE + LightDM"
  apt update
  DEBIAN_FRONTEND=noninteractive apt install -y --no-install-recommends \
    xorg lxde-core lightdm
}

install_prereqs() {
  echo "[+] Instalando pré-requisitos (git, g++, Qt6, Graphviz)"
  apt install -y \
    git g++ vim cmake ninja-build gxmessage \
    qt6-base-dev qt6-base-dev-tools \
    qt6-tools-dev qt6-tools-dev-tools \
    qt6-charts-dev \
    libsbml5-dev r-base ngspice \
    graphviz octave
}

install_firefox() {
  echo "[+] Instalando Firefox ESR"
  apt install -y firefox-esr
}

set_firefox_default() {
  echo "[+] Definindo Firefox ESR como navegador padrão"

  sudo -u "$USER_NAME" bash <<EOF
mkdir -p ~/.config

cat > ~/.config/mimeapps.list <<EOL
[Default Applications]
text/html=firefox-esr.desktop
x-scheme-handler/http=firefox-esr.desktop
x-scheme-handler/https=firefox-esr.desktop
x-scheme-handler/about=firefox-esr.desktop
x-scheme-handler/unknown=firefox-esr.desktop
EOL
EOF
}

set_keyboard() {
  echo "[+] Configurando teclado ABNT2 (br)"
  if [ -f "${KEYBOARD_CONF}" ]; then
    sed -i \
      -e 's/^XKBMODEL=.*/XKBMODEL="abnt2"/' \
      -e 's/^XKBLAYOUT=.*/XKBLAYOUT="br"/' \
      "${KEYBOARD_CONF}"
  else
    echo "Arquivo ${KEYBOARD_CONF} não encontrado; criando..."
    cat > "${KEYBOARD_CONF}" <<EOF
XKBMODEL="abnt2"
XKBLAYOUT="br"
EOF
  fi

  # aplicar imediatamente (quando possível)
  setupcon || true
  localectl set-x11-keymap br abnt2 || true
}

cleanup_system() {
  echo "[+] Limpeza de pacotes e arquivos"
  apt clean
  apt autoremove --purge -y
  rm -rf /usr/share/doc/* /usr/share/man/* /usr/share/locale/* || true
}

trim_and_zerofill() {
  echo "[+] fstrim"
  fstrim -av || true

  if [ "${ZERO_FILL}" -eq 1 ]; then
    echo "[+] Preenchendo espaço livre com zeros (para melhorar compressão da OVA)"
    dd if=/dev/zero of=/zero.fill bs=1M status=progress || true
    rm -f /zero.fill
  fi
}

configure_shortcuts() {
  echo "[+] Configurando atalho Ctrl+Alt+T para abrir terminal"

  OPENBOX_CONF="/etc/xdg/openbox/rc.xml"

  if [ -f "$OPENBOX_CONF" ]; then
    # Evita duplicação
    if ! grep -q '<keybind key="C-A-T">' "$OPENBOX_CONF"; then
      sed -i '/<\/keyboard>/i \
    <keybind key="C-A-T">\
      <action name="Execute">\
        <command>xterm</command>\
      </action>\
    </keybind>' "$OPENBOX_CONF"
  
    # Propaga para usuário
    USER_CONF="/home/${USER_NAME}/.config/openbox"
    mkdir -p "$USER_CONF"
    cp "$OPENBOX_CONF" "$USER_CONF/lxde-rc.xml"
    chown -R ${USER_NAME}:${USER_NAME} /home/${USER_NAME}/.config
  fi
  else
    echo "Arquivo $OPENBOX_CONF não encontrado"
  fi
}

setup_startup_script() {
  echo "[+] Configurando script remoto para executar via Autostart (XDG)"

  USER_NAME="vboxuser"
  USER_HOME="/home/$USER_NAME"

  # Caminhos atualizados para Autostart
  STARTUP_SCRIPT="$USER_HOME/.local/bin/genesys_startup.sh"
  AUTOSTART_DIR="$USER_HOME/.config/autostart"
  AUTOSTART_FILE="$AUTOSTART_DIR/genesys_init.desktop"

  SCRIPT_URL="https://raw.githubusercontent.com/rlcancian/Genesys-Simulator/refs/heads/currentStable/ova/init.sh"

  echo "[+] Criando diretórios..."
  mkdir -p "$USER_HOME/.local/bin"
  mkdir -p "$USER_HOME/Documents"
  mkdir -p "$AUTOSTART_DIR"

  echo "[+] Baixando script de inicialização..."
  if ! wget -qO "$STARTUP_SCRIPT" "$SCRIPT_URL"; then
    echo "[-] Erro ao baixar script"
    exit 1
  fi

  chmod +x "$STARTUP_SCRIPT"
  chown "$USER_NAME:$USER_NAME" "$STARTUP_SCRIPT"

  echo "[+] Criando arquivo de autostart..."
  cat > "$AUTOSTART_FILE" <<EOF
[Desktop Entry]
Type=Application
Name=Genesys Simulator Updater
Comment=Verifica atualizações do GenESyS ao iniciar a sessão
Exec=$STARTUP_SCRIPT
Icon=system-software-update
Terminal=false
Categories=Development;
X-GNOME-Autostart-enabled=true
EOF

  chown -R "$USER_NAME:$USER_NAME" "$USER_HOME/.local"
  chown -R "$USER_NAME:$USER_NAME" "$USER_HOME/.config"
  chown "$USER_NAME:$USER_NAME" "$USER_HOME/Documents"
  chmod +x "$AUTOSTART_FILE"
}


install_guest_add_util() {
  local URL="https://raw.githubusercontent.com/rlcancian/Genesys-Simulator/refs/heads/currentStable/ova/install_guest_add.sh"
  local TARGET="/usr/local/bin/install_guest_add"

  echo "[+] Instalando utilitário install_guest_add..."

  if ! wget -qO "$TARGET" "$URL"; then
    echo "[-] Falha no download"
    return 1
  fi

  # Permissão de execução
  chmod +x "$TARGET"

  echo "[+] Instalado em: $TARGET"
}

setup_ova_updater() {
  echo "[+] Configurando updater da OVA com retry via systemd"

  local RUNNER="/usr/local/bin/ova_update_runner"
  local SERVICE="/etc/systemd/system/ova-update.service"
  local UPDATE_URL="https://raw.githubusercontent.com/rlcancian/Genesys-Simulator/refs/heads/currentStable/ova/update.sh"

  # Runner (falha de propósito se não conseguir baixar)
  cat > "$RUNNER" <<EOF
#!/usr/bin/env bash
set -euo pipefail

TMP_SCRIPT="/tmp/update.sh"
UPDATE_URL="$UPDATE_URL"

echo "[+] OVA updater iniciado (root)"

# precisa de wget
command -v wget >/dev/null 2>&1

# tenta baixar (se falhar, script falha → systemd reinicia)
wget -qO "\$TMP_SCRIPT" "\$UPDATE_URL"

chmod +x "\$TMP_SCRIPT"

# executa (se falhar, também dispara retry do systemd)
"\$TMP_SCRIPT"

rm -f "\$TMP_SCRIPT"

echo "[+] Update finalizado com sucesso"
EOF

  chmod +x "$RUNNER"

  # Service com retry
  cat > "$SERVICE" <<EOF
[Unit]
Description=OVA Auto Update (root)
After=network-online.target
Wants=network-online.target

[Service]
Type=oneshot
ExecStart=$RUNNER

# Retry controlado pelo systemd
Restart=on-failure
RestartSec=10
StartLimitIntervalSec=300
StartLimitBurst=20

[Install]
WantedBy=multi-user.target
EOF

  echo "[+] Recarregando systemd..."
  systemctl daemon-reexec
  systemctl daemon-reload

  echo "[+] Habilitando serviço..."
  systemctl enable ova-update.service

  echo "[+] Updater configurado com retry via systemd"
}

main() {
  require_root

  install_sudo_and_user
  install_clion
  install_gui
  install_prereqs
  install_firefox
  set_firefox_default
  set_keyboard
  configure_shortcuts
  setup_startup_script
  install_guest_add_util
  setup_ova_updater
  cleanup_system
  trim_and_zerofill

  echo "[+] Concluído. Reinicie a VM para aplicar completamente."
}

main "$@"
