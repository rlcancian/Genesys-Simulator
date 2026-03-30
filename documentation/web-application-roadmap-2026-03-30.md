# GenESyS — Roadmap para novo tipo de aplicação Web (Webhook/API HTTP)

## Objetivo
Adicionar um terceiro tipo de aplicação em `source/applications`, além de Terminal e GUI: **Web**.

A primeira versão deve:
- subir um processo HTTP;
- escutar requisições (requisito inicial: porta 80);
- traduzir chamadas HTTP em invocações de API do kernel (`Simulator`, `PluginManager`, `ModelManager`, etc.);
- devolver resposta estruturada (JSON) com sucesso/erro.

## Estado atual do código (baseline)

1. Existe uma interface de aplicação comum (`GenesysApplication_if`) com contrato `main(int argc, char** argv)`.
2. A aplicação Terminal deriva desse contrato e inicializa o `Simulator`, `PluginManager` e `Model` em `BaseGenesysTerminalApplication`.
3. O entrypoint atual de terminal instancia a classe configurada via `TraitsTerminalApp`.
4. A árvore CMake raiz compila o kernel e testes, mas não expõe ainda um alvo dedicado para `source/applications/web`.

## Decisões de arquitetura recomendadas (antes de codificar)

### 1) Separar “HTTP transport” de “Genesys service layer”
Criar duas camadas:

- **Camada de Transporte HTTP** (rota, parse de request, serialização de response);
- **Camada de Serviço Genesys** (classe adaptadora chamando `Simulator`/`ModelManager`/`PluginManager`).

Motivo: facilita testes unitários sem socket real e evita acoplamento do kernel com tecnologia HTTP.

### 2) Definir API mínima da v1 (small surface)
Evitar expor todo o kernel de uma vez. Começar com endpoints mínimos e estáveis:

- `GET /health` → status do processo.
- `GET /api/v1/simulator/info` → versão/nome (`Simulator`).
- `POST /api/v1/plugins/autoload` → `autoInsertPlugins(...)`.
- `POST /api/v1/models` → `newModel()` e retorno de id lógico.
- `GET /api/v1/models/current` → metadados do modelo corrente.
- `POST /api/v1/models/load` e `POST /api/v1/models/save` → persistência básica.

Somente após estabilizar isso, incluir operações de simulação (start/pause/step/stop).

### 3) Thread-safety e modelo de concorrência
Como o kernel não foi desenhado originalmente para uso concorrente em múltiplas requisições simultâneas, assumir inicialmente:

- **um único contexto de simulação por processo**;
- serialização de acesso por mutex no service layer;
- filas de comando (opcional) para operações longas.

### 4) Porta 80 por padrão: requisito e risco operacional
Escutar em porta 80 em Linux geralmente requer privilégios elevados. Recomendação prática:

- manter requisito funcional “aceitar porta 80”,
- porém implementar configuração por variável/CLI (`--port`) com default seguro (ex.: 8080) para desenvolvimento;
- documentar claramente que produção pode usar reverse proxy para publicar em 80.

## Estrutura de diretórios sugerida

```text
source/applications/web/
├── main.cpp
├── TraitsWebApp.h
├── BaseGenesysWebApplication.h
├── BaseGenesysWebApplication.cpp
├── http/
│   ├── HttpServer_if.h
│   ├── HttpRequest.h
│   ├── HttpResponse.h
│   └── SimpleHttpServer.cpp
├── api/
│   ├── ApiRouter.h
│   ├── ApiRouter.cpp
│   ├── JsonCodec.h
│   └── ErrorMapper.h
└── service/
    ├── GenesysService.h
    └── GenesysService.cpp
```

## Plano de implementação em fases

### Fase 0 — Preparação de build
1. Adicionar opção CMake para app web (ex.: `GENESYS_BUILD_WEB`).
2. Criar alvo executável dedicado (ex.: `genesys_webhook`) em `source/applications/web/CMakeLists.txt`.
3. Linkar com bibliotecas do kernel já existentes (`genesys_kernel_*`).
4. Garantir que build web não dependa da GUI.

**Pronto quando**: `cmake --build` compilar o executável web isoladamente.

### Fase 1 — Esqueleto da aplicação Web
1. Criar `BaseGenesysWebApplication` com padrão parecido ao terminal:
   - inicializa `Simulator`;
   - configura trace;
   - carrega plugins básicos;
   - inicializa/obtém modelo atual.
2. Criar `main.cpp` que instancia app via traits (`TraitsWebApp`).
3. Incluir endpoint `GET /health`.

**Pronto quando**: processo sobe e responde healthcheck.

### Fase 2 — Bridge HTTP → Kernel (v1)
1. Implementar `GenesysService` encapsulando chamadas do kernel:
   - `getSimulatorInfo()`;
   - `autoloadPlugins(path)`;
   - `createModel()`;
   - `getCurrentModelInfo()`;
   - `loadModel(file)` / `saveModel(file)`.
2. `ApiRouter` converte request HTTP para métodos do service.
3. Padronizar erros JSON (`code`, `message`, `details`).

**Pronto quando**: chamadas básicas via `curl` funcionarem de ponta a ponta.

### Fase 3 — Segurança e robustez mínima
1. Limitar payload e timeout.
2. Validar input (paths, parâmetros ausentes, método HTTP incorreto).
3. Bloquear operações perigosas sem whitelist (especialmente caminhos de arquivo).
4. Adicionar token de autenticação simples (header) para não expor API aberta.

**Pronto quando**: API rejeita entradas inválidas com status adequado.

### Fase 4 — Simulação remota (opcional na v1.1)
1. Endpoints de execução:
   - `POST /api/v1/simulation/start`
   - `POST /api/v1/simulation/pause`
   - `POST /api/v1/simulation/resume`
   - `POST /api/v1/simulation/step`
   - `POST /api/v1/simulation/stop`
2. Política de estado: retornar `409 Conflict` em transições inválidas.
3. Possível endpoint de eventos/telemetria (polling ou SSE).

**Pronto quando**: ciclo básico de execução puder ser controlado remotamente.

## Contrato HTTP recomendado (v1)

### Exemplo: `GET /api/v1/simulator/info`
Resposta 200:

```json
{
  "ok": true,
  "data": {
    "name": "GenESyS - GENeric and Expansible SYstem Simulator",
    "versionName": "thestrech",
    "versionNumber": 260330
  }
}
```

### Exemplo: erro padrão

```json
{
  "ok": false,
  "error": {
    "code": "MODEL_LOAD_FAILED",
    "message": "Could not load model",
    "details": {
      "filename": "..."
    }
  }
}
```

## Checklist técnico antes de declarar “novo tipo Web disponível”

- [ ] Diretório `source/applications/web` criado com entrypoint próprio.
- [ ] Alvo CMake dedicado para aplicação web.
- [ ] Processo HTTP responde `/health`.
- [ ] Endpoints v1 de Simulator/PluginManager/ModelManager implementados.
- [ ] Erros JSON consistentes e códigos HTTP corretos.
- [ ] Testes unitários do service layer.
- [ ] Smoke test de integração com `curl`.
- [ ] Documentação de execução/local/prod (incluindo porta 80).

## Riscos e mitigação

1. **Permissão de bind na porta 80**
   - Mitigação: porta configurável + reverse proxy.
2. **Condição de corrida no kernel**
   - Mitigação: serialização por mutex + escopo de sessão único na v1.
3. **Superexposição de API interna**
   - Mitigação: começar pequeno, versionar API (`/api/v1`) e autenticar.
4. **Quebra de build por dependência HTTP externa**
   - Mitigação: encapsular backend HTTP atrás de interface (`HttpServer_if`).

## Próximo passo recomendado (execução incremental)
1. Implementar Fase 0 e Fase 1 primeiro (sem simulação remota).
2. Entregar demo com `health + simulator info + create model`.
3. Só então avançar para endpoints de controle de simulação.
