# GenesysQtGUI (Qt6) — Análise profunda e plano de correção

> Escopo analisado: `source/applications/gui/qt/GenesysQtGUI` e a integração com o kernel (`source/kernel/simulator`).

## 1) Análise profunda da estrutura atual

## 1.1 Arquitetura de alto nível (como está hoje)

A GUI está organizada em torno de três blocos principais:

1. **Janela principal (`MainWindow`)**
   - É o orquestrador da aplicação: cria `Simulator`, inicializa UI, conecta eventos e manipula quase todas as responsabilidades (edição de modelo, simulação, persistência, animação e sincronização da árvore de plugins/propriedades).
   - Arquivos centrais: `mainwindow.h`, `mainwindow.cpp`, `mainwindow_controller.cpp`, `mainwindow_scene.cpp`, `mainwindow_modelrepresentations.cpp`, `mainwindow_simulator.cpp`.

2. **Camada gráfica de modelo (`QGraphicsScene/QGraphicsView` + itens gráficos)**
   - `ModelGraphicsView` encapsula um `ModelGraphicsScene`.
   - `ModelGraphicsScene` concentra lógica de desenho, edição, agrupamento, conexão, undo/redo, animações e também parte da lógica de domínio (criação/remoção de conexões no kernel e manutenção de propriedades).
   - Classes `Graphical*` representam visualmente objetos do kernel (`ModelComponent`, `Connection`, `ModelDataDefinition` etc.).

3. **Kernel de simulação (`Simulator* simulator`)**
   - `MainWindow` usa `Simulator` como principal ponto de acesso ao modelo/simulação/plugins.
   - Eventos do kernel (`OnEventManager`, `TraceManager`) atualizam a GUI.

---

## 1.2 O que está forte/conceitualmente correto

- A ideia de **mapeamento Kernel ↔ GUI por classes Graphical** é boa e alinhada com um editor visual de simulação.
- O uso de **eventos do kernel** para atualizar GUI é adequado.
- O uso de **`QUndoStack` e comandos de edição** (Add/Delete/Move/Group/Ungroup/Paste) é um bom fundamento para editor gráfico.

---

## 1.3 Problemas estruturais observados

### A) Acoplamento excessivo entre camadas

Hoje há forte mistura entre:
- responsabilidade de interface,
- lógica de edição gráfica,
- sincronização com modelo de simulação,
- e regras de domínio.

Exemplos:
- `MainWindow` está muito grande e concentra muitas responsabilidades.
- `ModelGraphicsScene` manipula não só itens visuais mas também propriedades, conexões do modelo e animações.

**Efeito:** manutenção difícil, regressões frequentes, baixa previsibilidade de efeitos colaterais.

### B) Fronteiras confusas entre `QGraphicsScene` e `QGraphicsView`

Há sinais de confusão de papel entre View e Scene:
- View com lógica de delegação + estado de negócio.
- Scene com lógica de UI + domínio + persistência de estado de edição.

No Qt, idealmente:
- **View**: viewport/interação de visualização (zoom, pan, input de alto nível);
- **Scene**: gestão de itens e regras de manipulação gráfica;
- **itens (`QGraphicsItem`)**: desenho e comportamento local.

No estado atual, responsabilidades estão cruzadas em várias direções.

### C) Gestão de memória frágil

Há uso massivo de `new`/`delete`, listas de ponteiros crus e mapas de ponteiros crus.
- grande risco de vazamento, dangling pointer e dupla-liberação.
- difícil raciocinar ownership de objetos gráficos e objetos de edição de propriedades.

### D) Fragilidade de eventos e notificações

Há chamadas de notificação sem defesa forte contra estado inválido.
Exemplo típico: acessar `views().at(0)` sem validar se existe view associada.

### E) Sinais de “migração parcial” Qt4/Qt5→Qt6

O código compila em Qt6, mas com muitos comentários/ajustes pontuais e acoplamentos legados.
Isso indica que a migração foi funcional, porém sem refatoração arquitetural sistemática.

---

## 1.4 Conclusão arquitetural

A base é valiosa e funcional, mas hoje o principal risco não está em um único bug: está na **arquitetura de manutenção**, especialmente:
- acoplamento,
- ownership de memória,
- responsabilidades misturadas,
- e ausência de fronteiras claras entre GUI e kernel.

Esses fatores explicam por que o Property Editor e outros fluxos parecem “instáveis”: o sistema está com alta complexidade incidental.

---

## 2) Relação de problemas por gravidade

## 2.1 Problemas graves (prioridade P0/P1)

1. **Risco de corrupção de estado por ownership indefinido**
   - Múltiplos containers com ponteiros crus para os mesmos objetos.
   - Vida útil de objetos depende de vários pontos.

2. **Responsabilidades críticas concentradas em `MainWindow`**
   - Classe “God Object”; difícil testar e evoluir com segurança.

3. **`ModelGraphicsScene` com responsabilidades além da cena**
   - Mistura edição gráfica + domínio + propriedade + animação + sincronização.

4. **Tratamento de erro inseguro em pontos críticos de evento gráfico**
   - Trechos com `try/catch` em evento de cena mascarando problema real.

5. **Notificações com pressupostos frágeis (`views().at(0)`, handlers potencialmente não setados)**
   - Potencial crash em cenários de inicialização/parcial teardown.

6. **Projeto de build da GUI muito acoplado ao restante do repositório**
   - `.pro` compila uma massa muito grande de fontes do kernel/plugins/examples, aumentando tempo de build e risco de quebra transversal.

---

## 2.2 Problemas médios (P2)

1. **Duplicação de utilitários (`myrgba`) em várias classes**.
2. **Nomenclatura/ortografia inconsistente** (`Dimentions`, `Cheked`, `Arranje`, `Bototm`, `visivible`, etc.).
3. **Muita lógica em slots de UI sem camada intermediária clara**.
4. **Estruturas de dados duplicadas para os mesmos itens gráficos** (`_all*`, `_graphical*`).
5. **Uso de casts frequentes e lógica condicional extensa por tipo em tempo de execução**.

---

## 2.3 Problemas menores / melhoria contínua (P3)

1. Comentários legados e TODOs antigos sem ticket/owner.
2. Inconsistência de idioma em nomes/comentários.
3. Ausência de um documento de arquitetura oficial da GUI.
4. Ausência de suíte mínima de testes de regressão para fluxos essenciais de edição.

---

## 3) Plano seguro e consistente de resolução (sem quebrar build)

A estratégia deve ser **incremental, com “gates” de build e rollback fácil**.

## 3.1 Princípios de execução

1. **Um objetivo técnico por PR** (small batch).
2. **Não misturar refatoração estrutural com mudança de comportamento** no mesmo PR.
3. **Introduzir adapters/fachadas antes de mover lógica**.
4. **Compilar e validar a cada etapa**, com checklist mínimo repetível.
5. **Garantir compatibilidade temporária** com API atual enquanto migração acontece.

---

## 3.2 Fases propostas

### Fase 0 — Baseline e rede de segurança
- Definir baseline de build da GUI no Qt6.
- Criar checklist de smoke test manual:
  - abrir app,
  - criar modelo,
  - inserir plugin,
  - conectar componentes,
  - rodar start/step/pause/resume/stop,
  - salvar/abrir.
- Congelar comportamento atual com “testes de fluxo” mínimos (mesmo que inicialmente manuais + script).

**Saída da fase:** capacidade de detectar regressão cedo.

### Fase 1 — Separar responsabilidades sem mudar funcionalidade
- Extrair de `MainWindow` serviços/coordenadores:
  - `SimulationController` (start/step/pause/resume/stop + eventos do kernel),
  - `GraphicalModelController` (operações de cena),
  - `PluginBrowserController`.
- `MainWindow` vira orquestrador fino de UI.

**Gate de segurança:** nenhuma alteração de formato de arquivo/modelo; apenas reencaminhamento de chamadas.

### Fase 2 — Clarificar contrato View vs Scene
- Definir formalmente responsabilidades:
  - View = navegação/interação de viewport,
  - Scene = itens e comandos gráficos,
  - Item = desenho/comportamento local.
- Mover gradualmente lógica indevida para a camada correta.

**Gate:** comportamento visual idêntico nos fluxos de edição principais.

### Fase 3 — Higienização de ownership/memória
- Introduzir ownership explícito:
  - `std::unique_ptr`/`std::shared_ptr` quando aplicável,
  - objetos Qt parentados corretamente,
  - remoção de `new`/`delete` manuais onde possível.
- Documentar para cada coleção: quem cria, quem destrói, quando invalida.

**Gate:** sanitizer/valgrind (ou equivalente) sem erros críticos em fluxo básico.

### Fase 4 — Estabilizar Property Editor
- Reprodução guiada dos bugs atuais.
- Definir modelo de binding propriedade↔widget único e previsível.
- Remover estados paralelos redundantes (`propertyList`, `propertyEditorUI`, `propertyBox`) em favor de uma abstração única.

**Gate:** editar propriedade reflete no kernel e na GUI de forma consistente.

### Fase 5 — Fortalecer Undo/Redo e consistência de cena
- Revisar invariantes de comandos (`Add/Delete/Move/Paste/Group/Ungroup`).
- Garantir atomicidade das operações compostas (componente + conexão + grupos + dados associados).

**Gate:** sequência de undo/redo longa sem perda de consistência.

### Fase 6 — Limpeza final e governança técnica
- Padronização de nomenclatura.
- Remoção de TODOs resolvidos e abertura de issues para pendências reais.
- Documento arquitetural final da GUI.
- Pipeline CI focado na GUI Qt6.

---

## 3.3 Estratégia para não quebrar build durante todo o processo

1. **Branch de refatoração por fases** (`refactor/gui-phase-N`).
2. **PR pequeno com check obrigatório de build** por fase.
3. **Feature flags temporárias** para trocas de comportamento de risco.
4. **Compat layer transitória** para APIs antigas até concluir migração.
5. **Proibição de mudanças amplas no `.pro` junto de refatoração funcional**.
6. **Critério de merge rígido:** build verde + smoke test básico aprovado.

---

## 4) Backlog inicial priorizado (ordem recomendada)

1. Criar documento de arquitetura alvo da GUI (curto, 1–2 páginas).
2. Extrair `SimulationController` de `MainWindow` (sem mudar comportamento).
3. Extrair `GraphicalModelController`.
4. Revisar pontos de crash (notificação de eventos, null checks, handlers).
5. Introduzir ownership explícito das estruturas de propriedade.
6. Só então atacar Property Editor e fluxos avançados.

---

## 5) Resultado esperado após execução do plano

- GUI mais previsível e com menos regressões.
- Menor custo de manutenção.
- Correções do Property Editor com baixo risco.
- Evolução para Qt6 sustentável (e não apenas “compatível”).
- Base mais apropriada para colaboração via PRs pequenos e seguros.

---

## 6) Execução inicial do plano (primeira etapa já iniciada)

Nesta primeira execução prática (fase inicial de estabilização), as seguintes melhorias de baixo risco foram iniciadas no código:

1. **Defesa de notificações da cena**:
   - verificação de `views().isEmpty()` e `dynamic_cast` para evitar acesso inválido ao encaminhar eventos gráficos.

2. **Defesa de handlers opcionais na view**:
   - chamadas de callback agora validam se o handler foi configurado antes de invocar.

3. **Correção de ciclo de vida de evento gráfico**:
   - `GraphicalModelEvent` passou a ser liberado após notificação na view para evitar vazamento.

4. **Remoção de `try/catch` frágil em evento gráfico**:
   - simplificação de `sceneEvent` em `GraphicalConnection` para evitar mascarar falhas.

5. **Melhoria de desalocação no fechamento da janela principal**:
   - destrutor da `MainWindow` passou a liberar estruturas alocadas dinamicamente que antes não eram explicitamente liberadas.

### Próximo passo imediato

Na sequência, o próximo passo seguro será extrair um primeiro controlador fino para lógica de simulação (`SimulationController`) sem alterar comportamento funcional, mantendo PR pequeno e com validação incremental.

### Andamento adicional (passo seguinte iniciado)

Como evolução incremental, parte da lógica repetida dos comandos de simulação foi centralizada em helpers de `MainWindow`:

- `_hasCurrentModelSimulation()` para validar pré-condições de execução;
- `_ensureSimulationReady()` para consolidar check + sincronização do modelo antes de start/step/resume.

Isso reduz duplicação e risco de `null dereference` nos handlers de simulação.

### Andamento adicional (extração inicial de controlador)

Foi iniciada a extração de responsabilidade de simulação para uma classe dedicada `SimulationController`:

- validação centralizada de pré-condições (`hasCurrentModelSimulation`, `ensureReady`);
- uso dessa validação nos comandos start/step/pause/resume/stop;
- documentação doxygen adicionada na nova classe e nos handlers principais.

@ToDo: ampliar documentação doxygen para todas as classes/métodos restantes da GUI de forma incremental por módulo (`graphicals`, `dialogs`, `propertyeditor`), mantendo PRs pequenos e seguros.
