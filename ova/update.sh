#!/usr/bin/env bash
set -euo pipefail

VERSION_FILE="/etc/genesys_ova_version"
CURRENT_VERSION="1.0" # Incremente ao fazer novo update

require_root() {
  if [ "$(id -u)" -ne 0 ]; then
    echo "Execute como root"
    exit 1
  fi
}

get_installed_version() {
  if [ -f "$VERSION_FILE" ]; then
    cat "$VERSION_FILE"
  else
    echo "0.0"
  fi
}

set_version() {
  echo "$1" > "$VERSION_FILE"
}

version_gt() {
  # retorna 0 (true) se $1 > $2
  dpkg --compare-versions "$1" gt "$2"
}

update_1_0() {
  echo "[+] MIGRAÇÃO COMPLETA: INIT ANTIGO → NOVO SISTEMA"

  REAL_USER="vboxuser"
  USER_HOME="/home/vboxuser"

  mkdir -p "$USER_HOME/.local/bin"
  mkdir -p "$USER_HOME/.config/autostart"

  echo "[+] Removendo bootstrap antigo..."
  rm -f "$USER_HOME/.local/bin/genesys_startup.sh" || true

  echo "[+] Garantindo dependências básicas"
  apt-get update -y || true
  apt-get install -y wget curl git gxmessage tar mousepad|| true

  echo "[+] Instalando novo init.sh"
  INIT_URL="https://raw.githubusercontent.com/rlcancian/Genesys-Simulator/refs/heads/currentStable/ova/init.sh"

  DOWNLOAD_OK=0
  for i in 1 2 3; do
    echo "[+] Tentando baixar init.sh (tentativa $i)..."

    if wget -qO /usr/local/bin/genesys_init.sh "$INIT_URL"; then
      DOWNLOAD_OK=1
      break
    fi

    echo "[!] Falha no download, aguardando rede..."
    sleep 5
  done

  if [ "$DOWNLOAD_OK" -ne 1 ]; then
    echo "[!] Falha ao baixar init.sh após 3 tentativas"
    echo "[!] Pulando update (tentará novamente no próximo boot)"
    return 0
  fi

  chmod 755 /usr/local/bin/genesys_init.sh

  echo "[+] Criando launcher limpo"
  cat > "$USER_HOME/.local/bin/genesys_startup.sh" <<EOF
#!/bin/bash
set -euo pipefail
exec /usr/local/bin/genesys_init.sh
EOF

  chmod +x "$USER_HOME/.local/bin/genesys_startup.sh"

  echo "[+] Atualizando autostart"
  cat > "$USER_HOME/.config/autostart/genesys_init.desktop" <<EOF
[Desktop Entry]
Type=Application
Name=Genesys Init System
Exec=$USER_HOME/.local/bin/genesys_startup.sh
Icon=system-software-update
Terminal=false
X-GNOME-Autostart-enabled=true
Categories=Development;
EOF

  echo "[+] Migrando repositório para novo padrão"

  OLD_REPO="$USER_HOME/Documents/Genesys-Simulator"
  NEW_REPO="$USER_HOME/Documents/Genesys-Dev"

  if [ -d "$OLD_REPO" ]; then
    if [ ! -e "$NEW_REPO" ]; then
      mv "$OLD_REPO" "$NEW_REPO"
      echo "[+] Repo renomeado"
    else
      echo "[!] Genesys-Dev já existe"
    fi
  fi

  DEV_REPO_PATH="$NEW_REPO"
  [ -d "$DEV_REPO_PATH" ] || DEV_REPO_PATH="$OLD_REPO"

  echo "[+] Forçando configuração padrão (currentStable)"
  
  DEV_BRANCH_FILE="$USER_HOME/.genesys_dev_branch"
  echo "currentStable" > "$DEV_BRANCH_FILE"
  
  if [ -d "$DEV_REPO_PATH/.git" ]; then
      echo "[+] Entrando no repo: $DEV_REPO_PATH"
  
      cd "$DEV_REPO_PATH"
      
      echo "[+] Limpando cache Qt Creator/CMake"
      find . \( -name 'CMakeLists.txt.user' -o -name 'CMakeCache.txt' \) -delete
      find . -type d \( -name 'build' -o -name 'cmake-build-*' \) -exec rm -rf -- {} +
  
      if sudo -u "$REAL_USER" git rev-parse --is-inside-work-tree >/dev/null 2>&1; then
          sudo -u "$REAL_USER" git config genesys.lastAppliedBranch "currentStable"
          echo "[+] Git config aplicada"
      else
          echo "[!] .git existe mas repo está inválido"
      fi
  
      cd - >/dev/null || true
  else
      echo "[!] Diretório não é um repo git: $DEV_REPO_PATH"
  fi

  echo "[+] Limpando serviço antigo"
  rm -f "$USER_HOME/.config/systemd/user/genesys-web.service"

  echo "[+] Corrigindo permissões"
  chown -R "$REAL_USER:$REAL_USER" \
    "$USER_HOME/.local" \
    "$USER_HOME/.config" \
    "$USER_HOME/Documents" || true

  chown "$REAL_USER:$REAL_USER" "$DEV_BRANCH_FILE" || true

  echo "[+] MIGRAÇÃO CONCLUÍDA"
}

update_1_1() {
  echo "[+] Update 1.1"
  # Adicione aqui
}

run_updates() {
  local INSTALLED
  INSTALLED=$(get_installed_version)

  echo "[+] Versão instalada: $INSTALLED"
  echo "[+] Versão alvo: $CURRENT_VERSION"
  
  # Adicione novas versoes ao fazer updates (Ex: for v in 1.0 1.1 1.2 do)
  for v in 1.0; do
    if version_gt "$v" "$INSTALLED"; then
      FUNC="update_${v//./_}"

      if declare -f "$FUNC" >/dev/null; then
        "$FUNC"
        set_version "$v"
      else
        echo "[-] Função $FUNC não encontrada"
        exit 1
      fi
    fi
  done

  echo "[+] Sistema atualizado para $CURRENT_VERSION"
}

main() {
  require_root
  run_updates
}

main "$@"
