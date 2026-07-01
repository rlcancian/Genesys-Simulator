# Tema 5 - Como os problemas da GUI foram solucionados

Este README resume a solucao implementada para o Tema 5, com foco em explicar
no video o que foi feito, por que foi feito e onde isso aparece no codigo.

## 1. Problema original

O tema pedia a conclusao das acoes pendentes da interface grafica do GenESyS,
com foco principal nas animacoes da GUI.

O diagnostico principal foi que a GUI ja possuia uma infraestrutura de cena,
desenho e animacao, mas ainda misturava dois tipos de animacao:

- animacoes nucleares da GUI, como tempo de simulacao, contadores, variaveis,
  statistics collectors e graficos;
- animacoes especificas de plugins de simulacao, como filas, recursos e
  estacoes.

O problema arquitetural era que animacoes especificas de dominio ficavam
acopladas diretamente a `ModelGraphicsScene`, que e a classe central da cena
grafica. Isso fazia a GUI principal conhecer detalhes que deveriam pertencer a
plugins graficos.

## 2. Objetivo da solucao

A solucao buscou separar melhor o que pertence ao nucleo da GUI daquilo que
deve ser fornecido por plugins graficos.

Em termos praticos:

- as animacoes nucleares continuam na GUI base;
- animacoes de fila, recurso e estacao passam a ser registradas como
  contribuicoes de plugins graficos;
- a `ModelGraphicsScene` deixa de instanciar e atualizar diretamente todas as
  animacoes especificas;
- o `GuiExtensionManager` passa a intermediar a criacao e o despacho de eventos
  dessas animacoes.

## 3. Infraestrutura criada

A infraestrutura principal fica em:

- `source/applications/gui/qt/GenesysQtGUI/extensions/GuiExtensionContracts.h`
- `source/applications/gui/qt/GenesysQtGUI/extensions/GuiExtensionManager.h`
- `source/applications/gui/qt/GenesysQtGUI/extensions/GuiExtensionManager.cpp`

### 3.1. GuiAnimationContribution

`GuiAnimationContribution` define o que um plugin grafico oferece para a GUI:

```cpp
struct GuiAnimationContribution {
    std::string animationType;
    std::function<AnimationPlaceholder*(void)> createPlaceholder;
    std::function<void(ModelGraphicsScene*, const GuiSimAnimationEvent&)> onSimEvent;
};
```

Cada contribuicao informa:

- `animationType`: o tipo da animacao, por exemplo `"Queue"`, `"Resource"` ou
  `"Station"`;
- `createPlaceholder`: funcao que cria o item visual da animacao;
- `onSimEvent`: funcao chamada quando ocorre um evento de simulacao relacionado
  aquela animacao.

### 3.2. GuiSimAnimationEvent

`GuiSimAnimationEvent` e o envelope de evento usado para enviar informacoes da
simulacao ao plugin grafico.

Ele carrega:

- tipo do evento, como `Insert` ou `Remove`;
- componente afetado;
- nome do alvo da animacao, como o nome do recurso ou da estacao;
- flag de visibilidade.

Com isso, a cena grafica nao precisa conhecer os detalhes internos de cada
plugin. Ela apenas cria um evento generico e o envia para o manager.

### 3.3. GuiExtensionManager

O `GuiExtensionManager` atua como intermediario entre a GUI principal e os
plugins graficos.

Ele centraliza tres responsabilidades:

- coleta as contribuicoes registradas pelos plugins;
- cria placeholders de animacao sob demanda;
- encaminha eventos de simulacao para o plugin responsavel.

Trecho principal de coleta:

```cpp
plugin->registerContributions(&registry);
plugin->registerAnimations(&registry);

_animationContributions = registry.animations();
```

Trecho principal de criacao de placeholder:

```cpp
AnimationPlaceholder* GuiExtensionManager::createAnimationPlaceholder(
    const std::string& animationType) const
{
    for (const GuiAnimationContribution& contrib : _animationContributions) {
        if (contrib.animationType == animationType && contrib.createPlaceholder) {
            return contrib.createPlaceholder();
        }
    }
    return nullptr;
}
```

Trecho principal de despacho de eventos:

```cpp
void GuiExtensionManager::dispatchAnimationEvent(
    const std::string& animationType,
    ModelGraphicsScene* scene,
    const GuiSimAnimationEvent& event) const
{
    for (const GuiAnimationContribution& contrib : _animationContributions) {
        if (contrib.animationType == animationType && contrib.onSimEvent) {
            contrib.onSimEvent(scene, event);
        }
    }
}
```

## 4. Desacoplamento da ModelGraphicsScene

Antes, a `ModelGraphicsScene` tendia a conhecer diretamente classes concretas
de animacao especificas de plugins.

Conceitualmente, o fluxo era:

```text
ModelGraphicsScene
  -> AnimationQueue
  -> AnimationResource
  -> AnimationStation
```

Depois da reorganizacao, a cena passa a depender de um contrato comum:

```text
ModelGraphicsScene
  -> GuiExtensionManager
      -> plugin grafico responsavel
```

Na criacao de placeholders, a cena nao instancia diretamente as classes de
fila, recurso ou estacao. Ela pede ao manager:

```cpp
return manager ? manager->createAnimationPlaceholder("Queue") : nullptr;
return manager ? manager->createAnimationPlaceholder("Resource") : nullptr;
return manager ? manager->createAnimationPlaceholder("Station") : nullptr;
```

No despacho de eventos, a cena tambem nao aplica a regra especifica sozinha.
Ela envia o evento para o manager:

```cpp
_guiExtensionManager->dispatchAnimationEvent("Queue", this, event);
```

Para recursos e estacoes, a cena identifica o tipo de evento da simulacao e
encaminha para o plugin correspondente:

```text
Seize  -> Resource Insert
Release -> Resource Remove
Enter  -> Station Insert
Leave  -> Station Remove
```

Esses fluxos aparecem em:

- `ModelGraphicsScene::notifyEntityMovePluginAnimations`
- `ModelGraphicsScene::notifyAfterProcessPluginAnimations`
- `SimulationEventController::onMoveEntityEvent`
- `SimulationEventController::onAfterProcessEvent`

## 5. Plugins graficos criados

Foram criados ou reorganizados plugins graficos concretos para os principais
casos pedidos no tema.

### 5.1. QueueAnimationGuiExtensionPlugin

Arquivo:

`source/applications/gui/qt/GenesysQtGUI/extensions/QueueAnimationGuiExtensionPlugin.cpp`

Responsabilidade:

- registra a animacao `"Queue"`;
- cria o placeholder visual de fila;
- trata eventos de insert/remove usando a logica de `AnimationQueue`.

### 5.2. ResourceAnimationGuiExtensionPlugin

Arquivo:

`source/applications/gui/qt/GenesysQtGUI/extensions/ResourceAnimationGuiExtensionPlugin.cpp`

Responsabilidade:

- registra a animacao `"Resource"`;
- cria o placeholder visual de recurso;
- mantem um contador visual de ocupacao (`entityCount`);
- incrementa no `Seize`;
- decrementa no `Release`;
- atualiza a cor e o texto do placeholder conforme o recurso fica livre ou
  ocupado.

Fluxo resumido:

```text
Seize(Resource)
  -> ModelGraphicsScene
  -> dispatchAnimationEvent("Resource")
  -> ResourceAnimationGuiExtensionPlugin
  -> entityCount++
```

### 5.3. StationAnimationGuiExtensionPlugin

Arquivo:

`source/applications/gui/qt/GenesysQtGUI/extensions/StationAnimationGuiExtensionPlugin.cpp`

Responsabilidade:

- registra a animacao `"Station"`;
- cria o placeholder visual de estacao;
- mantem um contador visual de ocupacao;
- incrementa no `Enter`;
- decrementa no `Leave`.

Fluxo resumido:

```text
Enter(Station)
  -> ModelGraphicsScene
  -> dispatchAnimationEvent("Station")
  -> StationAnimationGuiExtensionPlugin
  -> entityCount++
```

## 6. Consolidacao das animacoes nucleares

O tema tambem pedia consolidar animacoes que continuam pertencendo ao nucleo da
GUI.

Um dos principais pontos tratados foi `AnimationStatistics`, em:

- `source/applications/gui/qt/GenesysQtGUI/animations/AnimationPlaceholder.h`
- `source/applications/gui/qt/GenesysQtGUI/animations/AnimationPlaceholder.cpp`

Ela passou a ter:

- vinculo com `Collector_if`;
- metodo `setCollector`;
- metodo `refreshValue`;
- metodo `clearRuntimeState`;
- renderizacao propria com valor e numero de amostras.

A associacao entre placeholders de estatistica e os collectors do modelo e
feita por nome em:

- `ModelGraphicsScene::setStatisticsCollectors`

E a atualizacao durante a simulacao e feita por:

- `ModelGraphicsScene::animateStatistics`

## 7. Integracao com a simulacao

A integracao com a simulacao passa principalmente pelo
`SimulationEventController`.

Arquivos principais:

- `source/applications/gui/qt/GenesysQtGUI/controllers/SimulationEventController.cpp`
- `source/applications/gui/qt/GenesysQtGUI/graphicals/ModelGraphicsScene.cpp`

Eventos tratados:

- movimentacao de entidade: atualiza fila/recurso quando necessario;
- pos-processamento: atualiza estacao quando necessario;
- inicio de simulacao: limpa estados visuais e religa estatisticas;
- eventos de simulacao: atualizam contadores, variaveis, timer, plots e
  statistics.

Assim, a reorganizacao nao ficou apenas estrutural. Ela foi conectada ao ciclo
real de execucao da simulacao.

## 8. Testes adicionados

Foi adicionado um conjunto de testes automatizados em:

`source/tests/unit/test_gui_animation_dispatch.cpp`

Esses testes demonstram que:

- os plugins de animacao registram contribuicoes para `Queue`, `Resource` e
  `Station`;
- o `GuiExtensionManager` cria placeholders pelo tipo de animacao;
- recursos incrementam no `Seize` e decrementam no `Release`;
- estacoes incrementam no `Enter` e decrementam no `Leave`;
- nomes diferentes de alvo sao ignorados corretamente;
- os overlays sao limpos no inicio de uma nova simulacao;
- `AnimationStatistics` vincula corretamente ao `StatisticsCollector`;
- a cena nao quebra quando nao ha `GuiExtensionManager` configurado.



## 9. Resumo final

A solucao tornou a GUI mais modular porque moveu a responsabilidade das
animacoes especificas de dominio para plugins graficos.

Antes:

```text
ModelGraphicsScene conhecia diretamente detalhes de Queue, Resource e Station.
```

Depois:

```text
ModelGraphicsScene delega criacao e eventos ao GuiExtensionManager,
e cada plugin grafico implementa sua propria animacao.
```

Isso reduz acoplamento, preserva as animacoes nucleares na GUI base e cria uma
estrutura extensivel para novas animacoes no futuro.
