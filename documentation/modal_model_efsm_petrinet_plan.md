# Plano de implementação: ModalModel, EFSM e Redes de Petri Coloridas

## Objetivo
Definir uma base funcional mínima para modelos modais compostos por nodos e transições, com duas especializações iniciais:
- EFSM (Extended Finite State Machine)
- Rede de Petri Colorida (CPN)

## Estado atual (neste PR)
1. **Núcleo de rede modal**
   - `DefaultModalModel` agora possui:
     - lista de nodos e transições,
     - definição de nodo de entrada,
     - limite de passos/transições por despacho,
     - persistência de configuração base,
     - lógica de despacho mínimo para caminhar por transições habilitadas.
2. **Nodo base**
   - `DefaultNode` agora possui:
     - flags `initialNode` e `finalNode`,
     - lista de transições de saída,
     - persistência de flags.
3. **Transição base**
   - `DefaultNodeTransition` agora possui:
     - origem/destino, nome, guarda, ação de saída, evento de entrada,
     - prioridade, probabilidade e tipo,
     - avaliação de habilitação e execução base.
4. **Especializações mínimas**
   - `ModalModelFSM` e `ModalModelPetriNet`
   - `FSMState` (ações de entrada/saída)
   - `PetriPlace` (tokens coloridos)
   - `EFSMTransition` e `PetriTransition`

## Roadmap incremental sugerido
### Fase 1 — Estrutura e persistência (concluída neste PR)
- Implementar hierarquia de classes.
- Garantir compilação e integração nos targets de plugins.
- Persistir campos essenciais de configuração.

### Fase 2 — Semântica de execução EFSM (próximo passo)
- Implementar evento de trigger em `EFSMTransition` via atributo de entidade/contexto.
- Implementar ações de entrada/saída de `FSMState` no despacho do modal model.
- Definir política formal de seleção de transição (prioridade + probabilidade + fallback).

### Fase 3 — Semântica de execução CPN (próximo passo)
- Generalizar `PetriTransition` para múltiplos arcos de entrada/saída (não apenas source/destination únicos).
- Implementar binding de cor por expressão (ex.: `colorExpression`).
- Implementar validação de conservação de tokens e tracing de marcação.

### Fase 4 — Verificação e usabilidade
- `_check` robusto para todos os componentes da família modal.
- `GetPluginInformation` com help completo e exemplos.
- Casos de teste unitários para:
  - EFSM com guardas e transição probabilística.
  - CPN com duas cores e múltiplas transições concorrentes.

## Critérios de “mínimo funcional”
### EFSM mínimo
- Ao menos 2 estados (`FSMState`) e 1 `EFSMTransition`.
- Guardas avaliadas por expressão.
- Ação de transição executada.
- Estado atual persistido por atributo da entidade.

### CPN mínimo
- Ao menos 2 lugares (`PetriPlace`) e 1 `PetriTransition`.
- Tokens por cor com consumo/produção básicos.
- Habilitação por disponibilidade de tokens por cor.

## Riscos e observações
- A API atual de transições baseia-se em ponteiros de nodos; para CPN completa, será necessário modelo de arcos explícitos N->T e T->N.
- A seleção probabilística atual é deliberadamente simples; convém migrar para sampler do kernel para consistência estatística global.
