# GenESyS Docker

Este diretório contém o ambiente Docker para executar o GenESyS sem instalar Qt,
CMake, compiladores ou bibliotecas do projeto na máquina do usuário.

## Uso

Execute:

```bash
bash docker/exec_genesys.sh
```

O script mostra três opções:

1. **Usar o GenESyS como usuário**
   - usa o branch padrão do repositório remoto, resolvido pelo `HEAD` simbólico;
   - baixa ou atualiza o repositório Docker-local;
   - compila o alvo `genesys_gui`;
   - abre a aplicação `GenesysQtGUI` em um desktop virtual acessível pelo navegador.

2. **Usar o GenESyS como desenvolvedor**
   - usa o branch remoto `currentStable`;
   - baixa ou atualiza o repositório Docker-local;
   - pré-configura o projeto com o preset CMake `gui-app`;
   - abre o QtCreator com o projeto GenESyS.

3. **Iniciar o servidor web do GenESyS**
   - usa o branch padrão do repositório remoto, resolvido pelo `HEAD` simbólico;
   - baixa ou atualiza o repositório Docker-local;
   - compila o alvo `genesys_webhook`;
   - inicia o servidor web na porta `8080`.

Na primeira execução, se a imagem Docker ainda não existir, o script avisa o
usuário e monta a imagem automaticamente.

## Interface Gráfica

As opções gráficas rodam dentro do container com `Xvfb`, `x11vnc` e `noVNC`.
Depois de escolher a opção 1 ou 2, abra:

```text
http://localhost:6080/vnc.html?autoconnect=1&resize=scale
```

O script tenta abrir esse endereço automaticamente quando o sistema possui
`xdg-open` ou `open`.

## Servidor Web

A opção 3 expõe o servidor em:

```text
http://localhost:8080
```

Enquanto o servidor estiver rodando, o terminal fica ocupado exibindo o processo.
Para parar, pressione `Ctrl+C`.

## Variáveis Opcionais

As variáveis abaixo podem ser usadas antes de executar o script:

```bash
export GENESYS_IMAGE="genesys-simulator:local"
export GENESYS_REPO_URL="https://github.com/rlcancian/Genesys-Simulator.git"
export GENESYS_STATE_DIR="/caminho/para/cache/genesys"
export GENESYS_NOVNC_PORT=6080
export GENESYS_WEB_PORT=8080
```

O diretório `GENESYS_STATE_DIR` guarda os clones dos branches usados pelo Docker.
Ele é persistente para evitar baixar e recompilar tudo a cada execução.

## Observação Sobre Branches

O script valida a existência do branch remoto antes de clonar ou atualizar o
repositório.

As opções de usuário e servidor web usam o `HEAD` simbólico do remoto para
detectar automaticamente o branch padrão configurado no GitHub. No repositório
consultado em abril de 2026, esse branch padrão é `master`.

A opção de desenvolvedor usa explicitamente o branch remoto `currentStable`.
`origin` não é um branch: é apenas o nome local dado pelo Git ao repositório
remoto. Se o branch necessário não existir no repositório remoto configurado, a
execução para com uma mensagem de erro explícita.
