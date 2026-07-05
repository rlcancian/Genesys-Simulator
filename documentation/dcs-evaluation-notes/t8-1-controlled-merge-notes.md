# DCS Tema 8.1 — Notas de preparação parcial para avaliação

Estas notas registram decisões do professor/avaliador para a avaliação final do DCS Tema 8.1 — EDOs.

## PR original

- PR original dos alunos: #437 — `Tema 8.1 - EDOs`
- Base: `2026-1`
- Head: `feature/continuous-system-ode-solver`

## Decisões de aceitação parcial

O PR original **não deve ser mergeado integralmente como está**.

### Alterações não aceitas

1. A alteração em `PluginManager` que adiciona `insertStaticPlugin(Plugin*)` **não é aceita** e não deve ser incorporada ao GenESyS. O método insere plugins diretamente no gerenciador, aparentemente para viabilizar testes, mas contorna o fluxo normal de inserção/conexão/validação de plugins.
2. O `std::cout` de debug em `ContinuousSystemComponent::_onDispatchEvent` **não é aceito** como código de produção.
3. O nome `ContinuousSystemComponent` foi aceito provisoriamente apenas por falta de alternativa imediata, mas é considerado inadequado para o propósito do componente e deve ser revisto em refatoração posterior.

### Observações técnicas

- `RungeKutta4OdeSolver` já existia no branch `2026-1`; o PR apenas adiciona comentários a esse arquivo. Portanto, o solver RK4 genérico não deve ser contado como desenvolvimento novo dos alunos.
- O novo `ODESolver` representa equações diferenciais como strings avaliadas pelo parser do modelo. As variáveis das expressões são `Variable` do modelo, criadas/consultadas pelo `ModelDataManager` a partir dos nomes configurados no solver.
- A sincronização com o tempo do simulador está incompleta ou, no mínimo, limitada: o componente chama `ODESolver::integrate(simTime)` usando o `simulatedTime` atual como alvo, mas não altera o avanço do relógio de `ModelSimulation`. Assim, o tempo interno do solver avança até o tempo discreto já alcançado pelo kernel, e não há uma integração contínua acoplada ao mecanismo de avanço do tempo do kernel.
- O vídeo foi entregue apenas por link externo; para a regra da avaliação, isso deve ser considerado não entregue ou parcialmente entregue.

## Consequência para avaliação

A avaliação final deve diferenciar:

- código novo aceitável dos alunos: `ODESolver`, `ContinuousSystemComponent` com ressalvas, parte de `DiffEquations`, correção de `LSODE`, modelos e testes;
- código preexistente: `RungeKutta4OdeSolver`;
- alterações rejeitadas: `insertStaticPlugin(Plugin*)` e `std::cout` de debug.

Esta nota não representa aprovação para merge integral do PR original.