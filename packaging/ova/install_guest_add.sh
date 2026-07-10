#!/bin/bash
set -e

# Verifica se está rodando como root
if [ "$EUID" -ne 0 ]; then
  echo "Este script deve ser executado como root."
  echo "Use: sudo $0 <versao-do-virtualbox>"
  exit 1
fi

# Recebe a versão como argumento
VBOX_VERSION="$1"

if [ -z "$VBOX_VERSION" ]; then
  echo "Uso: $0 <versao-do-virtualbox>"
  echo "Exemplo: $0 7.0.14"
  exit 1
fi

ISO_NAME="VBoxGuestAdditions_${VBOX_VERSION}.iso"
ISO_URL="https://download.virtualbox.org/virtualbox/${VBOX_VERSION}/${ISO_NAME}"

apt update
apt install -y build-essential dkms linux-headers-$(uname -r) wget

cd /tmp

wget -O "$ISO_NAME" "$ISO_URL"

mkdir -p /mnt/cdrom
mount -o loop "$ISO_NAME" /mnt/cdrom

cd /mnt/cdrom
sh VBoxLinuxAdditions.run

cd /
umount /mnt/cdrom
rm -f "/tmp/$ISO_NAME"

echo "Instalação concluída. Reinicie a VM."
