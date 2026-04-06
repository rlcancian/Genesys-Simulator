# Genesys Web Application

Primeira implementação funcional do tipo de aplicação **Web** do GenESyS.

## O que já existe
- Executável CMake: `genesys_webhook` (habilitado com `-DGENESYS_BUILD_WEB=ON`).
- Servidor HTTP simples baseado em socket TCP.
- Endpoints iniciais:
  - `GET /health`
  - `GET /api/v1/simulator/info`

## Como executar
```bash
cmake --preset debug -DGENESYS_BUILD_WEB=ON
cmake --build --preset debug --target genesys_webhook
./build/debug/source/applications/web/genesys_webhook --port 8080
```

Parâmetros opcionais:
- `--port <N>`: porta HTTP (default 8080).
- `--max-requests <N>`: encerra após N requisições (útil para testes automatizados).

## Próximos incrementos
- Rotas `POST` para `PluginManager` e `ModelManager`.
- Tratamento de payload JSON.
- Autenticação por token.
- Endpoints de controle de simulação.
# Genesys Web Application (proposta)

Este diretório está reservado para o novo tipo de aplicação **Web** do GenESyS.

Objetivo inicial (v1): executar um processo HTTP que converta requisições em chamadas de API para o kernel (`Simulator`, `PluginManager`, `ModelManager`).

Consulte o roadmap técnico em:
- `documentation/web-application-roadmap-2026-03-30.md`
