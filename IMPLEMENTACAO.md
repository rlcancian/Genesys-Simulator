# Implementação — Tema 5: Conclusão das ações pendentes da interface gráfica do GenESyS

Branch de trabalho: `throwaway`  
Branch de referência: `origin/2026-1`

---

## Resumo executivo

O trabalho reorganizou a arquitetura de animações da GUI do GenESyS, separando as animações nucleares (que pertencem ao núcleo da aplicação gráfica) das animações específicas de plugins (fila, recurso, estação). As animações específicas de plugins foram migradas para o sistema de extensões gráficas (`GuiExtensionPlugin`) e a `ModelGraphicsScene` foi desacoplada de `AnimationQueue`. Adicionalmente, a animação de `Statistics Collectors` foi consolidada como uma animação nuclear com renderização própria.

---

## Alterações por arquivo

### 1. `extensions/GuiExtensionContracts.h` — Extensão do contrato de plugins

Adicionadas três estruturas que formam o contrato de animação para plugins gráficos:

- **`GuiSimAnimationEvent`** — envelope de evento de simulação enviado aos plugins de animação. Carrega o tipo do evento (`Insert` ou `Remove`), o componente de modelo afetado e uma flag `visible`.
- **`GuiAnimationContribution`** — contribuição registrada por um plugin de animação. Contém o identificador do domínio de animação (`"Queue"`, `"Resource"`, `"Station"`, …), uma função fábrica para criar o placeholder visual em modo de desenho, e um handler chamado quando um evento de simulação atinge aquele tipo de animação.
- Método `addAnimationContribution()` e acessor `animations()` adicionados a `GuiExtensionRegistry`.
- Método virtual `registerAnimations(GuiExtensionRegistry*)` adicionado a `GuiExtensionPlugin` (padrão vazio — retrocompatível com todos os plugins existentes).

### 2. `extensions/GuiExtensionManager.h` / `.cpp` — Extensão do gerenciador

Três novos métodos públicos:

- **`animationContributions()`** — retorna o vetor de contribuições coletadas na última chamada a `rebuild()`.
- **`dispatchAnimationEvent(animationType, scene, event)`** — percorre as contribuições e chama o handler do plugin cujo `animationType` coincide com o parâmetro.
- **`createAnimationPlaceholder(animationType)`** — chama a fábrica do plugin registrado para o tipo pedido; retorna `nullptr` se nenhum plugin registrou aquele tipo.

`rebuild()` agora chama `plugin->registerAnimations(&registry)` além de `registerContributions()`. `clear()` também limpa `_animationContributions`.

### 3. `extensions/QueueAnimationGuiExtensionPlugin.cpp` — Novo arquivo

Plugin gráfico responsável pela animação de filas. Define localmente a classe `AnimationQueueDisplay` (anteriormente declarada em `AnimationPlaceholder.h`) e registra uma `GuiAnimationContribution` para o tipo `"Queue"`:

- **Fábrica de placeholder** → `new AnimationQueueDisplay()`
- **Handler de evento de simulação** → cria um `AnimationQueue(scene, component)` e chama `addAnimationQueue(visible)` para `Insert` ou `removeAnimationQueue()` para `Remove`, replicando exatamente o comportamento que antes estava embutido diretamente em `ModelGraphicsScene`.

### 4. `extensions/ResourceAnimationGuiExtensionPlugin.cpp` — Novo arquivo

Plugin gráfico para animação de recursos. Define localmente `AnimationResource` com:

- Campo `_entityCount` (inteiro, nunca negativo).
- `setEntityCount(int)` / `entityCount()`.
- `paint()` próprio: borda e fundo verde (recurso livre) ou laranja (recurso ocupado), com o nome do alvo e a contagem entre colchetes quando ocupado.

O handler de evento de simulação percorre `scene->getAnimationsPlaceholder()`, localiza os placeholders do tipo `AnimationResource` cujo `targetName` coincide com o nome do componente, e incrementa ou decrementa `entityCount` conforme o tipo do evento (`Insert` +1 / `Remove` -1).

### 5. `extensions/StationAnimationGuiExtensionPlugin.cpp` — Novo arquivo

Plugin gráfico para animação de estações. Segue o mesmo padrão do plugin de recursos:

- `AnimationStation` com `_entityCount`, `setEntityCount()` / `entityCount()`.
- `paint()` próprio: borda e fundo cinza (estação livre) ou azul (estação ocupada).
- Handler de evento idêntico ao de recursos, porém operando sobre instâncias de `AnimationStation`.

### 6. `animations/AnimationPlaceholder.h` / `.cpp` — Limpeza e expansão de `AnimationStatistics`

**Removidas** as declarações de `AnimationQueueDisplay`, `AnimationResource` e `AnimationStation` do cabeçalho e do `.cpp`, pois essas classes foram movidas para seus respectivos plugins gráficos.

**Expandida** a classe `AnimationStatistics`:

- Adicionados `setCollector(Collector_if*)`, `getCollector()` e `refreshValue()`.
- `refreshValue()` lê `_collector->getLastValue()` e `_collector->numElements()` e força a atualização visual.
- `paint()` customizado: em modo de design (sem coletor vinculado) exibe fundo azul-claro com header azul e texto "no collector linked" em cinza tracejado. Em tempo de simulação (coletor vinculado) exibe o último valor com 4 casas decimais e a contagem de amostras `n = X`.

### 7. `graphicals/ModelGraphicsScene.h` / `.cpp` — Desacoplamento de `AnimationQueue`

**Cabeçalho:**
- `#include "animations/AnimationQueue.h"` **removido**; substituído por `#include "extensions/GuiExtensionManager.h"`.
- Forward declaration de `GuiExtensionManager` adicionada.
- Novos membros públicos: `setGuiExtensionManager(GuiExtensionManager*)` e `animateStatistics()`.
- Novo membro privado: `GuiExtensionManager* _guiExtensionManager = nullptr`.

**Implementação:**
- `animateQueueInsert()` não cria mais `AnimationQueue` diretamente; monta um `GuiSimAnimationEvent{Insert, component, visible}` e delega para `_guiExtensionManager->dispatchAnimationEvent("Queue", this, event)`.
- `animateQueueRemove()` idem, com evento do tipo `Remove`.
- `createPlaceholderAnimation()` (função livre interna) passa a receber `GuiExtensionManager*` e substitui `new AnimationQueueDisplay()`, `new AnimationResource()` e `new AnimationStation()` por chamadas a `manager->createAnimationPlaceholder(tipo)`.
- `animateStatistics()` percorre `_animationsPlaceholder`, faz `dynamic_cast<AnimationStatistics*>` e chama `refreshValue()` em cada instância encontrada.

### 8. `controllers/SimulationEventController.cpp` — Integração de `animateStatistics()`

Adicionadas duas chamadas a `_scene->animateStatistics()`:

- Em `onMoveEntityEvent()` — junto com as chamadas existentes a `animateCounter()` e `animateVariable()`.
- Em `onAfterProcessEvent()` — junto com `animateCounter()`, `animateVariable()` e `animateTimer()`.

Isso garante que os `AnimationStatistics` placeholders sejam atualizados em cada avanço da simulação.

### 9. `mainwindow.cpp` — Injeção do `GuiExtensionManager` na cena

Em `_refreshGuiExtensions()`, após chamar `_guiExtensionManager->rebuild(extensionContext)`, é feita a injeção:

```cpp
if (extensionContext.graphicsScene != nullptr) {
    extensionContext.graphicsScene->setGuiExtensionManager(_guiExtensionManager.get());
}
```

Isso conecta o gerenciador de extensões à cena gráfica, permitindo que `animateQueueInsert`/`animateQueueRemove` e `createPlaceholderAnimation` encontrem os plugins registrados.

### 10. `source/tools/` — Remoção de cabeçalhos bio/estatísticos

Removidos aproximadamente 35 arquivos de cabeçalho e implementação que estavam em `source/tools/` e eram usados pelos plugins bio e de estatísticas, mas não pertenciam ao núcleo do simulador. Entre eles:

- Interfaces e implementações de distribução de probabilidade (`ContinuousDistribution_if`, `DiscreteDistribution_if`, `ProbabilityDistribution`, etc.)
- Ferramentas de ajuste estatístico (`Fitter_if`, `FitterDefaultImpl`, `FitterDummyImpl`)
- Testes de hipótese (`HypothesisTester_if`, `HypothesisTesterDefaultImpl1`)
- Otimizadores e solucionadores (`Optimizer_if`, `Solver_if`, `SolverDefaultImpl1`, `OdeSolver_if`, `RungeKutta4OdeSolver`)
- Ferramentas específicas para biologia computacional (`BioKineticLawExpression`, `BioSimulationAnalysis`, `BioSimulationResult`, `MassActionOdeSystem`, `MetabolicFluxBalanceSolver`)

---

## Arquitetura resultante

```
GuiExtensionPlugin
  └─ registerAnimations(registry)
       └─ registry.addAnimationContribution({ type, createPlaceholder, onSimEvent })

GuiExtensionManager::rebuild()
  └─ coleta todas as GuiAnimationContribution dos plugins ativos

ModelGraphicsScene
  ├─ setGuiExtensionManager(manager)          ← injetado pelo MainWindow
  ├─ createPlaceholderAnimation(mode, manager)
  │    └─ manager->createAnimationPlaceholder("Queue"|"Resource"|"Station")
  ├─ animateQueueInsert(component, visible)
  │    └─ manager->dispatchAnimationEvent("Queue", this, Insert event)
  ├─ animateQueueRemove(component)
  │    └─ manager->dispatchAnimationEvent("Queue", this, Remove event)
  └─ animateStatistics()
       └─ refreshValue() em cada AnimationStatistics placeholder

Plugins registrados:
  QueueAnimationGuiExtensionPlugin   → "Queue"    (usa AnimationQueue)
  ResourceAnimationGuiExtensionPlugin → "Resource" (entity count + cor)
  StationAnimationGuiExtensionPlugin  → "Station"  (entity count + cor)
```

---

## O que ainda não foi implementado

- **Testes manuais sistemáticos** — a GUI não foi exercida com simulação em execução para confirmar que as animações de fila, recurso e estação se comportam identicamente ao comportamento pré-refatoração.
- **Testes automatizados de integração** — não foram criados testes de integração para o despacho de eventos de animação.
- **Vinculação de `AnimationStatistics` a coletores em tempo de carregamento de modelo** — `setCollector()` existe, mas a lógica que percorre os componentes do modelo e vincula cada placeholder ao coletor correto ainda precisa ser implementada.