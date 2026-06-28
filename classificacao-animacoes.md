# Classificação das Animações da GUI do GenESyS

## Critério de classificação

Uma animação pertence ao **núcleo da GUI** se faz sentido independentemente de qualquer plugin específico do simulador — ou seja, se depende apenas do kernel do GenESyS (simulação, entidades, tempo, contadores, variáveis, estatísticas gerais).

Uma animação é **específica de plugin** se depende da presença de um plugin concreto de domínio (fila, recurso, estação, etc.) para ter significado ou para funcionar corretamente.

---

## 1. Animações do núcleo — permanecem fixas na GUI

Estas animações referenciam apenas estruturas do kernel do GenESyS e devem ser consolidadas e mantidas na GUI base.

| Classe | Arquivo | Justificativa |
|---|---|---|
| `AnimationTimer` | `animations/AnimationTimer.{h,cpp}` | Exibe o relógio de simulação (horas:minutos:segundos). Depende apenas do tempo interno do simulador. Não tem nenhuma dependência de plugin. |
| `AnimationCounter` | `animations/AnimationCounter.{h,cpp}` | Exibe o valor de um `Counter` do kernel em tempo real. `Counter` é uma estrutura nuclear do GenESyS, não um plugin. |
| `AnimationVariable` | `animations/AnimationVariable.{h,cpp}` | Exibe o valor de `Variable`, `Attribute` ou qualquer `ModelDataDefinition` nuclear. Suporta arrays e fatias dimensionais. Depende apenas do kernel. |
| `AnimationTransition` | `animations/AnimationTransition.{h,cpp}` | Orquestra a movimentação visual de entidades entre componentes. É plugin-agnóstica: recebe qualquer `ModelComponent` de origem e destino. É a infraestrutura central de animação de execução. |
| `AnimationPlot` | (subclasse de `AnimationPlaceholder`) | Gráfico de acompanhamento geral da simulação (séries temporais, barras). Coleta dados de estruturas nucleares, não de plugins específicos. |
| `AnimationStatistics` | (subclasse de `AnimationPlaceholder`) | Exibe estatísticas gerais coletadas pelo kernel (`StatisticsCollector`). Não depende de plugin de domínio. |
| `GraphicalImageAnimation` | `graphicals/GraphicalImageAnimation.{h,cpp}` | Renderizador genérico de ícones de animação de entidades em movimento. Usado por `AnimationTransition`. Não tem dependência de plugin. |

### Infraestrutura nuclear que também permanece

Estas não são animações em si, mas fazem parte da camada que suporta as animações nucleares acima:

- `AnimationPlaceholder` (base) — widget genérico de placeholder; não contém lógica de plugin.
- `AnimationAttribute`, `AnimationEntity`, `AnimationEvent`, `AnimationExpression` — subclasses de placeholder sem lógica de domínio; são widgets de desenho que o usuário posiciona na cena.
- `GraphicalAnimateExpression` — stub vazio; pode evoluir como suporte a expressões do kernel.
- Métodos de `ModelGraphicsScene`: `animateTimer()`, `animateCounter()`, `animateVariable()`, `animatePlot()`, `animateTransition()` — orquestração das animações nucleares.

---

## 2. Animações específicas de plugin — devem migrar para plugins gráficos

Estas animações dependem diretamente de plugins concretos do simulador. Se o plugin não estiver carregado, a animação não tem objeto; portanto, não deve estar embutida na GUI principal.

| Classe | Arquivo | Plugin dependente | Evidência no código |
|---|---|---|---|
| `AnimationQueue` | `animations/AnimationQueue.{h,cpp}` | Plugin `Queue` (DiscreteProcessing) | Inclui `#include "plugins/data/DiscreteProcessing/Queue.h"` diretamente. Acessa métodos e estado da classe `Queue`. |
| `AnimationResource` | (subclasse de `AnimationPlaceholder`) | Plugin `Resource` | Ferramenta de desenho `drawingResource()` no `SceneToolController`; o conceito de recurso não existe no kernel mínimo. |
| `AnimationStation` | (subclasse de `AnimationPlaceholder`) | Plugin `Server`/`Station` | Ferramenta de desenho `drawingStation()` no `SceneToolController`; estação é um componente de plugin de domínio. |
| `AnimationQueueDisplay` | (subclasse de `AnimationPlaceholder`) | Plugin `Queue` | Anotação visual ligada à fila; sem a fila carregada, o elemento não tem significado. |

### Por que migrar

`AnimationQueue` é o caso mais direto: ela importa um header de plugin (`plugins/data/DiscreteProcessing/Queue.h`) e manipula a classe `Queue` diretamente. Isso cria uma dependência de compilação entre a GUI principal e o plugin, o que é o acoplamento que o trabalho precisa eliminar.

`AnimationResource` e `AnimationStation` são hoje apenas placeholders (sem lógica própria), mas suas ferramentas de ativação (`drawingResource`, `drawingStation`) estão fixas no `SceneToolController`, o que acopla a GUI ao vocabulário desses plugins mesmo quando eles não estão carregados.

---

## 3. Resumo visual

```
GUI Base (permanece fixa)
├── AnimationTimer          ← relógio de simulação
├── AnimationCounter        ← contadores do kernel
├── AnimationVariable       ← variáveis / atributos nucleares
├── AnimationTransition     ← movimentação de entidades (genérica)
├── AnimationPlot           ← gráficos gerais da simulação
├── AnimationStatistics     ← statistics collectors do kernel
└── (placeholders genéricos sem lógica de domínio)

Plugins Gráficos (deve migrar)
├── AnimationQueue          ← depende de Queue.h do plugin
├── AnimationResource       ← depende do plugin Resource
├── AnimationStation        ← depende do plugin Server/Station
└── AnimationQueueDisplay   ← anotação visual da fila
```

---

## 4. Observações para a implementação

1. **Ponto de entrada da migração**: `AnimationQueue` é o caso mais urgente porque já tem dependência de compilação (`#include`) de um header de plugin. Deve ser o primeiro a migrar.

2. **Ferramentas de menu acopladas**: os métodos `drawingQueue()`, `drawingResource()` e `drawingStation()` em `SceneToolController` precisam ser removidos da GUI fixa. Cada plugin gráfico correspondente deve registrar sua própria ação de menu e sua própria ferramenta de desenho.

3. **Listas em `ModelGraphicsScene`**: as listas `_animationsPlaceholder` e os modos de desenho `QUEUE_PLACEHOLDER`, `RESOURCE`, `STATION` precisam ser abstraídos para que plugins gráficos possam registrar seus próprios modos sem modificar a cena principal.

4. **`AnimationTransition` detecta fila**: o método `animateTransition()` já contém lógica que detecta se um componente tem fila e dispara `animateQueueInsert()`. Esse acoplamento também precisa ser desfeito — o plugin gráfico de fila deve se inscrever no evento de chegada de entidade, não a cena principal.

5. **Animações placeholder sem lógica** (`AnimationAttribute`, `AnimationEntity`, `AnimationEvent`, `AnimationExpression`) são ambíguas: hoje são apenas widgets de desenho sem comportamento de domínio. Podem permanecer na GUI base como facilidades genéricas, mas devem ser avaliadas caso a caso quando forem implementadas com lógica real.
