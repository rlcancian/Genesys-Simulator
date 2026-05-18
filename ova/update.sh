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
  echo "[+] Aplicando update 1.0"
  # exemplo: apt update
}

update_1_1() {
  echo "[+] Aplicando update 1.1"
  # exemplo: apt install -y htop
}

update_1_2() {
  echo "[+] Aplicando update 1.2"
  # exemplo: systemctl enable ssh || true
}

update_1_3() {
  echo "[+] Aplicando update 1.3"
  # exemplo: echo "Nova config" > /etc/genesys.conf
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
