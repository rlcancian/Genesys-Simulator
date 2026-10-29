---
document_type: evidence-report
authority: informative
owner: project-maintainer
period: 2026-10
status: active
tracks: 550
---

# Continuous Models Component Integration Handoff — 2026-10-29

## 1. Escopo analisado.

Integração local da hierarquia `ContinuousModelDataDefinition` (ODESolver, DiffusionField e BioNetwork), sincronização via `InternalEvent`, ciclo de execução dos componentes ContinuousSystemComponent, DiffusionSimulate e BioSimulate, fluxo de entidades, persistência, fábricas, regressões numéricas e documentação associada à PR #550.

As alterações desta execução não incluem merge para `WorkInProgress`. A PR deve permanecer aberta e em draft. O arquivo local `teste1.gui`, preexistente e não rastreado, foi preservado fora do staging.

## 2. Diagnóstico inicial.

**Confirmado no código:** a base temporal iniciava eventos de sincronização do calendário, e os modelos avançavam durante o despacho desses eventos. Os componentes DiffusionSimulate e BioSimulate chamavam `simulate()` e encaminhavam imediatamente a entidade, sem distinguir início de conclusão. O caminho reativo de ContinuousSystemComponent mantinha integração direta do ODE.

**Confirmado por execução:** no teste com o calendário real, a entidade saía de BioSimulate no tempo global 0 enquanto a Data Definition ainda estava `Running`. Um evento discreto intermediário, antes do primeiro evento interno, observava o estado contínuo do último ponto confirmado; não observava um estado matemático futuro.

**Forte indício:** as diferenças de execução e de retenção de entidade estavam duplicadas entre os componentes e permitiam encaminhamento prematuro, perda de erro ou reentrância sem política comum.

## 3. Evidências e defeitos encontrados.

- **Confirmado no código:** não havia um contrato comum de solicitação/conclusão consumível pelos ModelComponents.
- **Confirmado por execução:** o cenário de integração `Create → BioSimulate → Dispose` falhava semanticamente antes da correção porque a entidade era encaminhada antes da conclusão; após a implementação, duas entidades que chegam antes da conclusão compartilham a mesma execução e ambas seguem uma única vez no tempo global final.
- **Confirmado por execução:** um evento discreto a meio da janela de sincronização observa `CurrentTime` no ponto confirmado anterior e a execução no estado `WaitingForSynchronization`; não vê antecipadamente o estado futuro.
- **Confirmado no código:** a janela de execução das Data Definitions é local, começa em zero, e o início absoluto é o tempo global capturado na solicitação. `StartTime` não-zero em componentes legados é rejeitado porque não possui interpretação inequívoca no contrato de tempo local.
- **Forte indício:** o default `ODESolver(StopTime=60, StepSize=.01, StepsToSynchronize=1)` pode criar cerca de 6.000 eventos internos por execução automática. O default foi mantido por compatibilidade; a documentação agora explicita o custo.

## 4. Impacto arquitetural.

Foi introduzida uma base reutilizável abstrata `ContinuousModelComponent`, derivada de `ModelComponent`. Os três componentes concretos agora compartilham controle de solicitação, retenção de IDs de entidades, inscrição de conclusão, encaminhamento único, rejeição de conflito, limpeza em destruição e tratamento de falha. As Data Definitions permanecem responsáveis pelo cálculo matemático e pelo ciclo temporal; o kernel continua dono do relógio global e do calendário.

**Confirmado no código:** as classes concretas, nomes de plugins, factories, registradores, categorias, conexões e campos de persistência dos plugins foram preservados. A nova infraestrutura de runtime não adiciona campos persistidos. A hierarquia efetiva é:

```text
ModelComponent
└── ContinuousModelComponent (abstrata)
    ├── ContinuousSystemComponent → ODESolver
    ├── DiffusionSimulate → DiffusionField
    └── BioSimulate → BioNetwork
```

## 5. Solução implementada.

`ContinuousModelDataDefinition` centraliza estado tipado (`Idle`, `Running`, `WaitingForSynchronization`, `Completed`, `Failed`, `Cancelled`), solicitação, parâmetros temporais ativos, origem global, avanço de passo completo/parcial, eventos internos, cancelamento e notificações de conclusão. As subclasses preservam seus métodos numéricos e não atualizam o relógio do kernel.

`ContinuousModelComponent` mantém IDs, não ponteiros crus, para as entidades retidas; no callback re-resolve a entidade no modelo e só então a encaminha uma vez. Solicitações compatíveis para a mesma janela compartilham a execução ativa; janelas conflitantes são rejeitadas. Destruição cancela a inscrição e remove do modelo entidades ainda retidas quando ainda existem.

`ODESolver` ganhou uma sobrecarga de integração que devolve erro ao consumidor sem remover a API `void integrate(targetTime)` existente. O componente limita o avanço reativo ao instante confirmado pelo calendário. As janelas dos componentes de Diffusion/Bio validam início local zero e limites finitos.

**Decisão segura implementada:** a evolução matemática ocorre apenas ao despachar `InternalEvent`; criar o evento não avança o relógio global nem o estado contínuo. Se uma falha ocorrer sem porta de erro estabelecida, as entidades retidas não são encaminhadas e o erro é registrado como fatal. Não foi inventada semântica de encaminhamento em porta inexistente.

**Decisão pendente:** a semântica física de eventos discretos que ocorrem entre fronteiras de sincronização continua sendo `HUM-SCI-002`. Esses eventos observam o estado do último ponto confirmado, potencialmente defasado. A implementação não antecipa estado compartilhado nem introduz snapshots/tentative state. Os testes de calendário comprovam a causalidade do comportamento implementado, mas não decidem se essa é a semântica científica final desejada.

## 6. Patches e commits realizados.

O patch de código/testes foi confirmado depois das validações locais. A documentação/handoff segue em commit separado. O SHA final da branch após publicação será informado no registro de conclusão; o SHA deste commit de documentação não pode ser auto-referenciado pelo próprio arquivo.

- SHA inicial desta execução: `c00921b91db131087cff16c12488940d38801f32`.
- SHA de implementação/testes: `8a5a42cc10194c25ff1d6b07c36e6b94a0030895`.
- SHA de documentação/handoff: será o commit que contém este arquivo.
- SHA final publicado: commit de documentação/handoff (SHA completo registrado na resposta final após push).
- A alteração anterior de crash da GUI está no histórico inicial do branch (commit `c00921b`) e não foi editada nesta integração. Antes de integrar a PR, recomenda-se separar essa alteração de animação/GUI em PR própria sem reescrever o histórico remoto.

## 7. Validação executada.

Toolchain registrada: GCC 13.3.0, CMake 3.28.3, Ninja 1.11.1 e CTest 3.28.3. Os presets configuram C++23 com extensões desativadas.

| Verificação | Resultado observado |
|---|---|
| `tests-unit` build e CTest após a última alteração C++ | **Confirmado por execução:** 1.919 aprovados, 0 falhos, 4 desabilitados, de 1.923 listados. |
| `tests-kernel-unit` build/CTest | **Confirmado por execução:** build passou; 1.919 aprovados, 0 falhos, 4 desabilitados, de 1.923 listados. |
| `tests-smoke` build/CTest | **Confirmado por execução:** 3/3 aprovados, incluindo o oscilador harmônico. |
| `gui-app` build Qt6 | **Confirmado por execução:** `genesys-gui` compilado e linkado. Aviso Qt sobre `QString::count()` depreciada em código existente. GUI não foi iniciada interativamente. |
| Manual do desenvolvedor | **Confirmado por execução:** PDF regenerado com sucesso, 388 páginas. Avisos LaTeX de overfull/page annotations/hyperlink bounds preexistentes; sem erro LaTeX. |
| `python3 scripts/validate-ai-docs.py` | **Confirmado por execução:** passou com este relatório e o ledger atualizados. |
| ASan/UBSan focados em Bio/Diffusion, `detect_leaks=0` | **Confirmado por execução:** testes focados aprovados. |
| LeakSanitizer, `detect_leaks=1` | **Confirmado por execução:** CTest passou, mas processo retornou falha por 4.910 bytes/161 alocações, com stack no carregamento/registro de plugins do fixture. Não foi executada comparação de baseline; atribuição definitiva permanece não comprovada. |
| CI da PR | CI observada anteriormente estava verde no SHA inicial; checks do novo SHA devem ser verificados após push. |

Quatro casos desabilitados de forma preexistente: `SearchQueueFindsEntityInRangeSavesRankAndRoutesToFoundPort`, `SearchQueueNotFoundRoutesToPortZeroAndSavesZeroRank`, `RemoveEqualStartAndEndRankRemovesExactlyOneAndRoutesCorrectly`, `RemoveRangeRemovesOnlyEntitiesInsideConfiguredInterval`.

## 8. Riscos remanescentes.

- `HUM-SCI-002` permanece decisão científica/arquitetural: qual estado contínuo os eventos discretos intermediários devem observar, e se são necessários subpassos sincronizados, snapshots ou estado tentativo.
- Solicitações incompatíveis para uma Data Definition ocupada são rejeitadas; a semântica de múltiplos componentes compartilhando a mesma Data Definition pode exigir uma decisão de domínio.
- O componente não oferece saída de erro específica; falha interrompe o fluxo retido com diagnóstico fatal.
- O uso legado de janela em `DiffusionSimulate`/`BioSimulate` ainda atualiza parâmetros da Data Definition para executar a janela. Compatibilidade de modelos persistidos com overrides conflitantes requer revisão de modelos reais.
- O custo de sincronização do default do ODE pode ser elevado; alterar defaults de modelos persistidos foi evitado.
- LeakSanitizer falha com alocações do caminho de carregamento de plugins no fixture; a origem pré-existente ainda precisa ser isolada.
- A alteração de crash da GUI está misturada no histórico da branch antes desta execução e deve ser separada antes do merge, sem reescrever commits publicados.

## 9. Próximos passos recomendados.

1. Completar as rodadas finais kernel, smoke e GUI, validar documentação, revisar o diff e completar este relatório com os resultados reais.
2. Comitar em mudanças seletivas (código/teste e documentação), enviar para `WiP20261016/ContinuousModels`, manter a PR #550 em draft e conferir os checks do novo SHA.
3. Solicitar decisão do mantenedor para `HUM-SCI-002` e política para Data Definitions compartilhadas por múltiplos componentes.
4. Executar revisão independente e comparar LeakSanitizer contra baseline antes de atribuir as alocações a regressão.
5. Separar a correção anterior de crash da GUI em PR própria antes do merge da integração contínua.

## Apêndice — Evidências de revisão

- SHA inicial: `c00921b91db131087cff16c12488940d38801f32`.
- SHA final e SHAs dos commits: pendentes de publicação.
- PR: #550, `WiP20261016/ContinuousModels` → `WorkInProgress`; estado visto no início: aberta, draft, mergeable. Estado/checks pós-push pendentes.
- Arquivos C++ previstos: `source/plugins/components/Continuous/ContinuousModelComponent.{h,cpp}`, `ContinuousSystemComponent.{h,cpp}`, `DiffusionSimulate.{h,cpp}`, `source/plugins/components/BiochemicalSimulation/BioSimulate.{h,cpp}`, `source/plugins/data/Continuous/ContinuousModelDataDefinition.{h,cpp}`, `ODESolver.{h,cpp}`, `source/tests/unit/test_simulator_runtime.cpp`.
- Arquivos documentais previstos: referência `docs/ai_assistants/reference/CONTINUOUS_MODEL_EXECUTION.md`, índice da referência, capítulo do Developer Manual, fonte de sumário LaTeX, PDF do Developer Manual, este handoff e ledger de outubro.
- API adicionada: `ContinuousModelComponent`; tipos/métodos de solicitação, estado e callback em `ContinuousModelDataDefinition`; sobrecarga `ODESolver::integrate(targetTime, errorMessage)`.
- API removida: flags internas `_running` e `_awaitingSynchronization`, consolidadas no enum de estado.
- Persistência: sem novos campos persistidos ou nomes de tipos serializados; validar round-trips através das suítes e CI final.
- Testes criados: fluxo assíncrono com calendário real, duas entidades na mesma execução e observação discreta entre sincronizações.
- Comandos de build/test: `cmake --build --preset tests-unit --parallel 2 && ctest --preset tests-unit --output-on-failure`; `cmake --build --preset tests-kernel-unit --parallel 2 && ctest --preset tests-kernel-unit --output-on-failure`; `cmake --build --preset tests-smoke --parallel 2 && ctest --preset tests-smoke --output-on-failure`; `cmake --build --preset gui-app --parallel 2`.
- Sanitizers: ASan/UBSan focados aprovados com LeakSanitizer desligado; LeakSanitizer com 4.910 bytes/161 alocações, atribuição ainda não provada.
- Artifacts: PDF do Developer Manual; detalhes locais de build em `build/` e `build/continuous-sanitizer/`.
- Classificações usadas: `Confirmado no código`, `Confirmado por execução`, `Forte indício`, `Hipótese a validar`.
- Decisões tomadas: causalidade do calendário; atualização contínua no despacho de evento interno; retenção por ID e conclusão antes do fluxo; conflito de execução rejeitado explicitamente.
- Decisões do mantenedor: estado físico observável entre sincronizações (`HUM-SCI-002`), compartilhamento/conflição entre componentes e semântica de falha sem porta dedicada.
