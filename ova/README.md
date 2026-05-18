# OVA - Ambiente Virtual do GenESyS Simulator

## Motivação e Objetivo

Para execução do simulador em ambientes onde o sistema possui restrições, como ausência de permissões necessárias, ambientes inconsistentes entre máquinas e falta de dependências necessárias, foi desenvolvido um ambiente de virtualização por meio do uso de OVAs.

Uma OVA (Open Virtual Appliance) é um formato de distribuição de máquinas virtuais que encapsula disco, configuração e metadados em um único arquivo. Isso permite importar e executar o mesmo ambiente em diferentes máquinas, garantindo reprodutibilidade.

Neste projeto, a OVA é usada para fornecer um ambiente completo e já configurado para execução do GenESyS Simulator.

A escolha da imagem base seguiu critérios de estabilidade, compatibilidade e tamanho. Optou-se por Debian (netinst), com ambiente gráfico LXDE e LightDM, por ser uma combinação leve e suficientemente funcional para o uso em laboratório. Por esse motivo, os scripts para configuração do ambiente foram desenvolvidos para ambientes baseados em Debian.

---

## Como construir uma nova OVA

O processo consiste em criar uma VM base, instalar o sistema e aplicar o script de configuração de ambiente.

Após instalar o Debian (netinst) sem interface visual no VirtualBox, deve-se obter e executar um dos scripts de setup mínimo. Segue exemplo:

```bash
wget https://raw.githubusercontent.com/joameloo/Genesys-Simulator/refs/heads/2026-1/scripts/min_setup_qtcreator.sh
su -
chmod + ./min_setup_qtcreator.sh
./min_setup_qtcreator.sh
