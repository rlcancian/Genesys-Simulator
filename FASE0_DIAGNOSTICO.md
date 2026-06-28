# Fase 0 — Diagnóstico de Acoplamento

> Levantamento feito em 2026-06-21. Todos os arquivos estão em
> `Genesys-Simulator/source/applications/gui/qt/GenesysQtGUI/`.

---

## 1. `mainwindow.ui` — Itens de menu e toolbar de animação

### Menu `View > Animate` (linhas 920–935)

| Action name | Texto exibido | Toolbar também? |
|---|---|---|
| `actionAnimateSimulatedTime` | Simulated Time | Sim |
| `actionAnimateEntity` | Entity | Sim |
| `actionAnimateEvent` | Event | Sim |
| `actionAnimateCounter` | Counter | Sim |
| `actionAnimateStatistics` | Statistics | Sim |
| `actionAnimatePlot` | Plot | Sim |
| `actionAnimateVariable` | Variable and Attribute | Sim |
| `actionAnimateResource` | Resource | Sim |
| `actionAnimateQueue` | Queue | Sim |
| `actionAnimateExpression` | Expression | Sim |
| `actionAnimateStation` | Station | **só toolbar** |

`actionAnimateStation` aparece na `toolBarAnimate` (linha 1351) mas **não** no menu `menuViewAnimate` — lacuna na UI.

### Toggle geral

`actionAnimation` (checkable, checked por padrão) no menu Simulation (linha 886) — habilita/desabilita toda animação.

### Controle de velocidade

`horizontalSliderAnimationSpeed` na status bar (linhas 818–842), range 1–100, valor padrão 40.

### Como chegam aos slots

O `.ui` não tem `<connections/>` explícitas. A wiring é por convenção de nome Qt:

```
on_<objectName>_triggered()  →  mainwindow_controller.cpp
    → _sceneToolController->onActionAnimate<X>Triggered()
        → SceneToolController.cpp  activateAnimationDrawingTool(...)
            → ModelGraphicsScene::drawing<X>()
```

---

## 2. `graphicals/ModelGraphicsScene.cpp` — Métodos `animate*()`

| Método | Linha (impl.) | Depende de Queue/Resource/Station? |
|---|---|---|
| `animateTransition(component, dest, viewSim, event)` | 1791 | Não — usa `AnimationTransition` |
| `runAnimateTransition(anim, event, restart)` | 1843 | Não — usa `AnimationTransition` |
| `handleAnimationStateChanged(...)` | 2013 | Não — usa `AnimationTransition` |
| `animateQueueInsert(component, visible)` | 2064 | **Sim — `AnimationQueue`** |
| `animateQueueRemove(component)` | 2075 | **Sim — `AnimationQueue`** |
| `animateCounter()` | 2087 | Não |
| `animateVariable(entity)` | 2099 | Não |
| `animateTimer(time)` | 2106 | Não |
| `animatePlot()` | 2112 | Não |

**Nota:** `AnimationQueueDisplay`, `AnimationResource` e `AnimationStation` são instanciados em `createPlaceholderAnimation()` (linhas 136–157), não nos métodos `animate*` acima. O ponto de acoplamento de Queue nos animate é exclusivamente `animateQueueInsert` / `animateQueueRemove`.

---

## 3. `controllers/SimulationEventController.cpp` — Callbacks que disparam animações

| Callback | Linha | Animações chamadas | Condição |
|---|---|---|---|
| `onProcessEventHandler()` | 399 | `animatePlot()` (linha 411) | `animationsEnabled()` |
| `onMoveEntityEvent()` | 430 | `animateCounter()` (445), `animateVariable()` (446), `animateQueueRemove()` (462), `animateTransition()` (464) | `animationsEnabled()` + `_activateGraphicalSimulation->isChecked()` |
| `onAfterProcessEvent()` | 477 | `animateCounter()` (482), `animateVariable()` (483), `animateTimer()` (484) | `animationsEnabled()` |
| `onSimulationResumeHandler()` | 319 | `runAnimateTransition()` (344) | `animationsEnabled()` + animações pausadas existem |

`animateQueueInsert` não é chamado daqui — verificar se é chamado em outro lugar (possivelmente dentro de `animateTransition` ou evento de insert de fila).

---

## 4. `controllers/SceneToolController.cpp` e `mainwindow_controller.cpp` — Referências diretas

### SceneToolController.cpp

- `#include "../animations/AnimationTransition.h"` — **único include de Animation\***
- `AnimationTransition::setRunning(false/true)` — linhas 384, 388
- `AnimationTransition::setTimeExecution(newValue)` — linha 407
- **Não referencia** `AnimationQueue`, `AnimationResource`, `AnimationStation`
- Mapeia actions para `drawing*()` via `activateAnimationDrawingTool()` (linhas 277–323)

### mainwindow_controller.cpp

- **Nenhum `#include` de Animation\***
- **Nenhuma referência direta** a Queue/Resource/Station
- Apenas delega para `_sceneToolController->onActionAnimate*Triggered()` (linhas 327–528)

**Conclusão:** os dois controllers de UI são limpos — o acoplamento está concentrado em `ModelGraphicsScene`.

---

## 5. Subtipos de `AnimationPlaceholder` — Inventário completo

Todos definidos em `animations/AnimationPlaceholder.h` (linhas 39–138) e implementados em `animations/AnimationPlaceholder.cpp`.

| Classe | `paint()` próprio? | Classificação |
|---|---|---|
| `AnimationAttribute` | Não — herda X vermelho | Nuclear — placeholder (não implementado) |
| `AnimationEntity` | Não — herda X vermelho | Nuclear — placeholder (não implementado) |
| `AnimationEvent` | Não — herda X vermelho | Nuclear — placeholder (não implementado) |
| `AnimationExpression` | Não — herda X vermelho | Nuclear — placeholder (não implementado) |
| `AnimationStatistics` | Não — herda X vermelho | Nuclear — placeholder (não implementado) |
| `AnimationPlot` | **Sim** (linhas 255–420) | Nuclear — implementado (axes, gridlines, line/bar) |
| `AnimationQueueDisplay` | Não — herda X vermelho | Plugin candidate — Queue |
| `AnimationResource` | Não — herda X vermelho | Plugin candidate — Resource |
| `AnimationStation` | Não — herda X vermelho | Plugin candidate — Station |

**Total: 9 subtipos.** 6 nucleares (1 implementado, 5 só placeholder), 3 de plugin.

---

## Mapa de acoplamento — o que precisa migrar

```
ModelGraphicsScene.h/.cpp
  ├─ #include AnimationQueue       → remover após Fase 4
  ├─ animateQueueInsert()          → mover para QueueAnimationGuiExtensionPlugin
  ├─ animateQueueRemove()          → mover para QueueAnimationGuiExtensionPlugin
  └─ createPlaceholderAnimation()
       ├─ AnimationQueueDisplay    → instanciação via factory do plugin
       ├─ AnimationResource        → instanciação via factory do plugin
       └─ AnimationStation         → instanciação via factory do plugin

SimulationEventController.cpp
  └─ onMoveEntityEvent()
       ├─ animateQueueRemove()     → substituir por dispatchAnimationEvent("Queue.remove", ...)
       └─ (animateQueueInsert chamado em outro lugar — localizar)

mainwindow.ui
  └─ actionAnimateStation presente só na toolbar — adicionar ao menu View > Animate
```
