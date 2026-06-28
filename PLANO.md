# Plano — Tema 5: Animações como Plugins Gráficos

## Contexto

O GenESyS já tem infraestrutura de animação madura e um sistema de `GuiExtensionPlugin` funcional em `extensions/`. O trabalho é separar o que já existe em dois grupos: animações do núcleo (ficam na GUI base) e animações de plugin (migram para `GuiExtensionPlugin`).

---

## O que fica (animações nucleares)

| Classe | Status |
|---|---|
| `AnimationTransition` | Funcional — não mexer |
| `AnimationTimer` | Funcional — não mexer |
| `AnimationCounter` | Funcional — não mexer |
| `AnimationVariable` | Funcional — não mexer |
| `AnimationPlot` | Parcialmente implementado — completar `paint()` |
| `AnimationStatistics` | Só placeholder com X vermelho — implementar |

## O que migra (animações de plugin)

| Classe | Para onde |
|---|---|
| `AnimationQueue` / `AnimationQueueDisplay` | `QueueAnimationGuiExtensionPlugin` |
| `AnimationResource` | `ResourceAnimationGuiExtensionPlugin` |
| `AnimationStation` | `StationAnimationGuiExtensionPlugin` |

## Placeholders nucleares ainda não implementados

Estas classes existem em `animations/` como subtipos de `AnimationPlaceholder` (exibem só X vermelho em design-time). São conceitos do núcleo DES, portanto **ficam na GUI base** — mas precisam de uma decisão explícita sobre escopo com o professor antes de implementar:

| Classe | Decisão |
|---|---|
| `AnimationAttribute` | Nuclear — implementar se houver tempo, senão manter como placeholder |
| `AnimationEntity` | Nuclear — implementar se houver tempo, senão manter como placeholder |
| `AnimationEvent` | Nuclear — implementar se houver tempo, senão manter como placeholder |
| `AnimationExpression` | Nuclear — implementar se houver tempo, senão manter como placeholder |

---

## Fases

### Fase 0 — Diagnóstico (2–3 dias)
Ler e anotar os pontos de acoplamento antes de mudar qualquer coisa.

- [x] Ler `mainwindow.ui` para entender quais itens de menu acionam ferramentas de animação e como chegam até os slots
- [x] Listar todos os métodos `animate*()` em `graphicals/ModelGraphicsScene.cpp` e anotar quais dependem de Queue/Resource/Station
- [x] Rastrear em `controllers/SimulationEventController.cpp` quais callbacks disparam cada método de animação
- [x] Verificar se `controllers/SceneToolController.cpp` ou `mainwindow_controller.cpp` referenciam esses tipos diretamente
- [x] Listar e classificar **todos** os subtipos de `AnimationPlaceholder` — não só Queue/Resource/Station, mas também Attribute, Entity, Event, Expression

**Saída:** lista anotada de tudo que precisa migrar.

---

### Fase 1 — Fronteira (1 dia)
Confirmar a tabela acima com o professor se algum caso for ambíguo.

---

### Fase 2 — Estender o contrato de plugin (2–3 dias)
Adicionar suporte a contribuições de animação no sistema de plugins existente.

Arquivos: `extensions/GuiExtensionContracts.h`, `extensions/GuiExtensionManager.h/.cpp`

```cpp
// GuiExtensionContracts.h — nova estrutura
struct GuiAnimationContribution {
    std::string animationType;
    std::function<AnimationPlaceholder*(QGraphicsScene*)> placeholderFactory;
    std::function<void(ModelGraphicsScene*, const SimEvent&)> onSimEvent;
};

// GuiExtensionRegistry — novo método
void addAnimationContribution(const GuiAnimationContribution& c);

// GuiExtensionPlugin — método opcional a sobrescrever
virtual void registerAnimations(GuiExtensionRegistry* r) {}
```

- [ ] Adicionar `GuiAnimationContribution` em `GuiExtensionContracts.h`
- [ ] Adicionar lista interna e `dispatchAnimationEvent()` em `GuiExtensionManager`
- [ ] Chamar `registerAnimations()` no carregamento de cada plugin, junto com `registerContributions()`

---

### Fase 3 — Consolidar animações nucleares (3–4 dias)

- [ ] `AnimationTimer`, `AnimationCounter`, `AnimationVariable` — confirmar que não têm `#include` de Queue/Resource/Station
- [ ] `AnimationPlot` — completar o `paint()` com renderização real (linha ou barra simples já serve)
- [ ] `AnimationStatistics` — implementar exibição de valor do `StatisticsCollector`, seguindo o padrão de `AnimationCounter`
- [ ] Rodar simulação sem plugins e confirmar que as seis animações nucleares funcionam

---

### Fase 4 — Criar plugins de animação (5–7 dias)
Seguir o padrão dos plugins Bio existentes (`REGISTER_GUI_EXTENSION_PLUGIN`, mesma estrutura de classe).

Arquivos novos:
- `extensions/QueueAnimationGuiExtensionPlugin.h/.cpp`
- `extensions/ResourceAnimationGuiExtensionPlugin.h/.cpp`
- `extensions/StationAnimationGuiExtensionPlugin.h/.cpp`

```cpp
class QueueAnimationGuiExtensionPlugin : public GuiExtensionPlugin {
public:
    std::string extensionId()  const override { return "queue-animation"; }
    std::string displayName() const override { return "Queue Animation"; }
    std::vector<std::string> requiredModelPlugins() const override { return {"Queue"}; }

    void registerContributions(GuiExtensionRegistry* r) override {
        // item de menu para inserir placeholder de fila
    }
    void registerAnimations(GuiExtensionRegistry* r) override {
        r->addAnimationContribution({
            .animationType      = "Queue",
            .placeholderFactory = [](QGraphicsScene* s) { return new AnimationQueueDisplay(s); },
            .onSimEvent         = [](ModelGraphicsScene* s, const SimEvent& e) {
                // lógica atual de animateQueueInsert/Remove vai para cá
            }
        });
    }
};
REGISTER_GUI_EXTENSION_PLUGIN(QueueAnimationGuiExtensionPlugin)
```

- [ ] Criar `QueueAnimationGuiExtensionPlugin` — mover lógica de `animateQueueInsert/Remove` do `ModelGraphicsScene` para cá
- [ ] Criar `ResourceAnimationGuiExtensionPlugin`
- [ ] Criar `StationAnimationGuiExtensionPlugin`
- [ ] Registrar os três no `GuiExtensionPluginCatalog`

---

### Fase 5 — Desacoplar a GUI base (2–3 dias)

- [ ] Remover `animateQueueInsert()`, `animateQueueRemove()` e equivalentes do `ModelGraphicsScene`
- [ ] Substituir por `dispatchAnimationEvent(type, event)` que delega ao `GuiExtensionManager`
- [ ] Atualizar `SimulationEventController` para usar o método de despacho
- [ ] Confirmar que `ModelGraphicsScene.h` não inclui mais headers de Queue/Resource/Station
- [ ] Build limpo sem warnings novos

---

### Fase 6 — Testes (2–3 dias)

- [ ] GUI inicia sem plugins de animação — Timer, Counter, Variable, Plot, Statistics funcionam
- [ ] `AnimationPlot` exibe dados reais de uma série durante simulação
- [ ] `AnimationStatistics` exibe valor de um `StatisticsCollector`
- [ ] Com `QueueAnimationPlugin` carregado — placeholder de fila funciona e anima entidades
- [ ] Com `ResourceAnimationPlugin` carregado — placeholder de recurso funciona
- [ ] Com `StationAnimationPlugin` carregado — placeholder de estação funciona
- [ ] Ausência de plugin de fila não causa crash
- [ ] Todos os três plugins carregados ao mesmo tempo — sem conflito

---

## Arquivos principais

```
animations/
  AnimationTransition.h/.cpp     ← nuclear, não mexer
  AnimationTimer.h/.cpp          ← nuclear, não mexer
  AnimationCounter.h/.cpp        ← nuclear, não mexer
  AnimationVariable.h/.cpp       ← nuclear, não mexer
  AnimationPlot.h/.cpp           ← nuclear, completar paint()
  AnimationStatistics.h/.cpp     ← nuclear, implementar
  AnimationQueue.h/.cpp          ← helper, usado pelo plugin de Queue

extensions/
  GuiExtensionContracts.h        ← adicionar GuiAnimationContribution
  GuiExtensionManager.h/.cpp     ← adicionar dispatch
  QueueAnimationGuiExtensionPlugin.h/.cpp     ← criar
  ResourceAnimationGuiExtensionPlugin.h/.cpp  ← criar
  StationAnimationGuiExtensionPlugin.h/.cpp   ← criar

graphicals/
  ModelGraphicsScene.h/.cpp      ← remover métodos de plugin, adicionar dispatch

controllers/
  SimulationEventController.cpp  ← atualizar chamadas de animação
```
