# Auditoria técnica estruturada — `ModelDataDefinition` em `source/plugins/data` (branch `WiP20261`)

## 1) Visão geral

- **Escopo analisado**:
  - Núcleo de contrato/registro: `ModelDataDefinition.*`, `ModelDataManager.*`, `PluginInformation.*`, `Plugin.*`, `PluginManager.*`.
  - Todos os `.h/.cpp` em `source/plugins/data`.
- **Classes `ModelDataDefinition` analisadas em `source/plugins/data`**: **17**
  1. `CppCompiler`
  2. `DummyElement`
  3. `EntityGroup`
  4. `Failure`
  5. `File`
  6. `Formula`
  7. `Label`
  8. `Queue`
  9. `Resource`
  10. `SPICERunner`
  11. `Schedule`
  12. `Sequence`
  13. `Set`
  14. `SignalData`
  15. `Station`
  16. `Storage`
  17. `Variable`

### Síntese de maturidade

- **Claramente funcionais (com contrato mínimo implementado e papel de domínio ativo)**: `Queue`, `Resource`, `Failure`, `Station`, `Schedule`, `Set`, `Label`, `Formula`, `EntityGroup`, `Sequence`, `Storage`, `Variable`, `SignalData`, `CppCompiler`.
- **Claramente template/scaffold/placeholder**: `DummyElement`.
- **Parcialmente placeholder/incompleto (não scaffold puro, mas com lacunas estruturais claras)**: `File`, `SPICERunner`.

### Classes com maior risco técnico (auditado no código)

- `Queue` (regra de ordenação persistida/configurável, mas não aplicada em runtime).
- `Resource` (suporte a `priority` em `seize()` e persistência de falhas explicitamente incompletos; TODOs explícitos).
- `Variable` (inconsistência grave de ponteiros em `_initBetweenReplications()`: alias entre `_values` e `_initialValues`).
- `Sequence` / `SequenceStep` (persistência de assignments por passo com índices potencialmente colidindo entre passos; métodos-base de `SequenceStep` vazios).
- `SignalData`, `Schedule`, `Formula`, `Failure`, `Set`, `Resource`, `Variable` (forte indício de risco de ciclo de vida/ownership por alocações heap não liberadas explicitamente).

### Maior valor para correção imediata

1. `Queue`
2. `Variable`
3. `Resource`
4. `Sequence`
5. `SignalData`

---

## 2) Matriz por classe

> Legenda de classificação dos achados:
> - **confirmado no código**
> - **forte indício**
> - **hipótese a validar**

## Classe: `CppCompiler`
- **Arquivos**: `source/plugins/data/CppCompiler.h`, `source/plugins/data/CppCompiler.cpp`
- **Papel aparente**: compilação dinâmica (executável/lib estática/lib dinâmica) + carga via `dlopen`.
- **Contrato implementado**:
  - Construtor/destrutor: construtor implementado; destrutor `= default`.
  - `show`, `GetPluginInformation`, `LoadInstance`, `NewInstance`, `_loadInstance`, `_saveInstance`, `_check`, `_initBetweenReplications`, `_createInternalAndAttachedData`: implementados.
  - `_getParserChangesInformation`: não sobrescrito (usa base).
- **Problemas encontrados**:
  - Símbolo exportado sob `PLUGINCONNECT_DYNAMIC` aparece como `getPluginInformation` e referência a `CppCompiler::getPluginInformation` (difere do padrão `GetPluginInformation`). **confirmado no código**.
  - `unloadLibrary()` tem `return true;` antes de uma atribuição `_libraryLoaded = false;` (linha morta). **confirmado no código**.
  - Capturas com `catch` amplo sem tratamento substantivo em múltiplos pontos. **confirmado no código**.
  - Inclusões redundantes e acoplamento pesado a APIs de sistema. **forte indício**.
- **Prioridade**: **média**.
- **Observações**: funcional, porém com dívida técnica de robustez e consistência de plugin-export.

## Classe: `DummyElement`
- **Arquivos**: `source/plugins/data/DummyElement.h`, `source/plugins/data/DummyElement.cpp`
- **Papel aparente**: scaffold/template de plugin de dados.
- **Contrato implementado**:
  - Mínimo (`show`, fábricas, load/save) implementado.
  - `_check`, `_initBetweenReplications`, `_createInternalAndAttachedData`, `_getParserChangesInformation`: só esqueleto/comentado.
- **Problemas encontrados**:
  - Metadados e comentários explicitamente de template; campos exemplo (`someString/someUint`). **confirmado no código**.
  - Trechos de exemplo comentados e TODOs de não-implementação. **confirmado no código**.
- **Prioridade**: **baixa**.
- **Observações**: útil como referência de desenvolvimento, não como classe funcional final.

## Classe: `EntityGroup`
- **Arquivos**: `source/plugins/data/EntityGroup.h`, `source/plugins/data/EntityGroup.cpp`
- **Papel aparente**: agrupamento de entidades por chave (ex.: `Entity.Group`) com estatística interna.
- **Contrato implementado**:
  - Construtor/destrutor, `show`, fábricas, `_loadInstance`, `_saveInstance`, `_check`, `_createInternalAndAttachedData` implementados.
  - `_initBetweenReplications` e `_getParserChangesInformation` não sobrescritos.
- **Problemas encontrados**:
  - `getGroup()` retorna `new List<Entity*>()` quando chave não existe, transferindo ownership implícito ao chamador (sem contrato explícito). **forte indício**.
  - `show()` com TODO explícito.
  - Estrutura `_groupMap` alocada em heap e sem liberação explícita no destrutor (destrutor praticamente vazio). **forte indício**.
- **Prioridade**: **média**.
- **Observações**: funcional para uso principal, mas com risco de vazamento e contrato de retorno pouco seguro.

## Classe: `Failure`
- **Arquivos**: `source/plugins/data/Failure.h`, `source/plugins/data/Failure.cpp`
- **Papel aparente**: regras de falha de recurso por tempo/contagem.
- **Contrato implementado**:
  - Implementa `show`, fábricas, `_loadInstance`, `_saveInstance`, `_check`, `_initBetweenReplications`.
  - `_createInternalAndAttachedData` e `_getParserChangesInformation` não sobrescritos.
- **Problemas encontrados**:
  - Lista de recursos falhantes `_falingResources` não é persistida (comentários de extensão). **confirmado no código**.
  - TODOs explícitos sobre contrato semântico de unidades/expressões e limpeza em replicação. **confirmado no código**.
  - Alocações heap (`_releaseCounts`, `_falingResources`) sem liberação explícita na classe. **forte indício**.
- **Prioridade**: **média-alta**.
- **Observações**: núcleo funcional existe; maior lacuna é completar persistência e robustez de ciclo de vida.

## Classe: `File`
- **Arquivos**: `source/plugins/data/File.h`, `source/plugins/data/File.cpp`
- **Papel aparente**: definição de metadados de arquivo externo para módulos consumidores.
- **Contrato implementado**:
  - Mínimo (`show`, fábricas, `_loadInstance`, `_saveInstance`, `_check`, `_getParserChangesInformation`) implementado.
  - `_initBetweenReplications` e `_createInternalAndAttachedData` não sobrescritos.
- **Problemas encontrados**:
  - Quase todo conteúdo específico de domínio está comentado (load/save/check com placeholders). **confirmado no código**.
  - Classe parece funcionalmente mínima/incompleta para a semântica descrita no cabeçalho. **forte indício**.
- **Prioridade**: **média**.
- **Observações**: não é scaffold puro, mas está próxima de “stub funcional mínimo”.

## Classe: `Formula`
- **Arquivos**: `source/plugins/data/Formula.h`, `source/plugins/data/Formula.cpp`
- **Papel aparente**: armazenamento de expressões por índice e avaliação via parser do modelo.
- **Contrato implementado**:
  - Implementa `show`, fábricas, `_loadInstance`, `_saveInstance`, `_check`, `_createInternalAndAttachedData`.
  - `_initBetweenReplications` e `_getParserChangesInformation` não sobrescritos.
- **Problemas encontrados**:
  - `_createInternalAndAttachedData()` insere auto-referência (`this`) para evitar órfão (workaround explícito). **confirmado no código**.
  - TODOs explícitos sobre parser dedicado e rastreabilidade de expressão ausente. **confirmado no código**.
  - Mapa de expressões alocado em heap sem liberação explícita na classe. **forte indício**.
- **Prioridade**: **média**.
- **Observações**: funcional, mas com workaround arquitetural e potencial risco de ownership.

## Classe: `Label`
- **Arquivos**: `source/plugins/data/Label.h`, `source/plugins/data/Label.cpp`
- **Papel aparente**: ponto lógico de roteamento/transferência para componentes de entrada.
- **Contrato implementado**:
  - Implementa `show`, fábricas, `_loadInstance`, `_saveInstance`, `_check`.
  - `_createInternalAndAttachedData`, `_initBetweenReplications`, `_getParserChangesInformation` não sobrescritos.
- **Problemas encontrados**:
  - Falta de tratamento explícito para vínculo ausente de componente (apenas `_check` sinaliza). **forte indício**.
  - `catch` amplo silencioso em load. **confirmado no código**.
- **Prioridade**: **baixa-média**.
- **Observações**: contrato principal está coerente.

## Classe: `Queue`
- **Arquivos**: `source/plugins/data/Queue.h`, `source/plugins/data/Queue.cpp`
- **Papel aparente**: estrutura de espera com política de ordenação e estatísticas.
- **Contrato implementado**:
  - Implementa `show`, fábricas, `_loadInstance`, `_saveInstance`, `_check`, `_initBetweenReplications`, `_createInternalAndAttachedData`, `_getParserChangesInformation`.
- **Problemas encontrados**:
  - **Problema funcional confirmado**: `setOrderRule()` registra `OrderRule`, mas mantém TODO para ordenar (`SORT THE QUEUE...`). **confirmado no código**.
  - Persistência de `orderRule` existe, porém sem efeito de reordenação efetiva. **confirmado no código**.
  - Estrutura de espera (`Waiting*`) sem liberação explícita ao limpar lista/encerrar instância. **forte indício**.
- **Prioridade**: **alta**.
- **Observações**: classe madura, com bug funcional já conhecido e prioritário.

## Classe: `Resource`
- **Arquivos**: `source/plugins/data/Resource.h`, `source/plugins/data/Resource.cpp`
- **Papel aparente**: capacidade, custos, estado e eventos de recursos.
- **Contrato implementado**:
  - Implementa `show`, fábricas, `_loadInstance`, `_saveInstance`, `_check`, `_initBetweenReplications`, `_createInternalAndAttachedData`.
  - `_getParserChangesInformation` não sobrescritos.
- **Problemas encontrados**:
  - `seize(quantity, priority)` com TODO explícito de prioridade não considerada. **confirmado no código**.
  - Persistência de falhas (`_failures`) marcada com TODO para salvar/carregar. **confirmado no código**.
  - TODO conhecido no header para estatística temporal em capacidade > 1. **confirmado no código**.
  - Listas em heap (`_resourceEventHandlers`, `_failures`) sem liberação explícita. **forte indício**.
- **Prioridade**: **alta**.
- **Observações**: alta criticidade por impacto transversal em fluxo e custo de simulação.

## Classe: `SPICERunner`
- **Arquivos**: `source/plugins/data/SPICERunner.h`, `source/plugins/data/SPICERunner.cpp`
- **Papel aparente**: integração com simulação eletrônica (ngspice) e coleta de medidas.
- **Contrato implementado**:
  - Implementa `show`, fábricas, `_loadInstance`, `_saveInstance`.
  - Não implementa `_check`, `_createInternalAndAttachedData`, `_initBetweenReplications`, `_getParserChangesInformation` (usa base).
- **Problemas encontrados**:
  - Persistência usa campos genéricos (`someString/someUint`) de template, sem refletir estado principal do runner. **confirmado no código**.
  - Chamadas a `std::system` e parsing textual com pouca validação defensiva. **forte indício**.
  - Métodos variádicos aparentam typos de chamada (`PlotVPlotVRelative`, `PlotVPlotIRelative`). **confirmado no código**.
- **Prioridade**: **média-alta**.
- **Observações**: funcionalidade especializada, porém ainda com sinais fortes de código em evolução.

## Classe: `Schedule`
- **Arquivos**: `source/plugins/data/Schedule.h`, `source/plugins/data/Schedule.cpp`
- **Papel aparente**: agenda temporal de expressões (com repetição opcional).
- **Contrato implementado**:
  - Implementa `show`, fábricas, `_loadInstance`, `_saveInstance`, `_check`, `_initBetweenReplications`, `_createInternalAndAttachedData`, `_getParserChangesInformation`.
- **Problemas encontrados**:
  - `_schedulableItems` recebe `new SchedulableItem` em load sem limpeza de objetos previamente alocados (apenas `clear` da lista). **forte indício**.
  - Possível risco de laço longo/indefinido em `getExpression()` quando `_repeatAfterLast=true` e itens com duração zero. **hipótese a validar**.
- **Prioridade**: **média**.
- **Observações**: boa cobertura de contrato, mas atenção para ownership de itens.

## Classe: `Sequence`
- **Arquivos**: `source/plugins/data/Sequence.h`, `source/plugins/data/Sequence.cpp` (e suporte `AssignmentItem.*`)
- **Papel aparente**: sequência de passos (estação/label + assignments) para roteamento de entidade.
- **Contrato implementado**:
  - `Sequence`: implementa `show`, fábricas, `_loadInstance`, `_saveInstance`, `_check`.
  - `SequenceStep`: implementa overloads persistentes por índice; overrides base existem mas um par é vazio.
- **Problemas encontrados**:
  - `SequenceStep::_loadInstance/_saveInstance` (overrides base sem `parentIndex`) vazios; alto risco de uso indevido futuro. **confirmado no código**.
  - Persistência de assignments por passo usa chaves sem incluir índice do passo (`assignDest0`, etc.), potencialmente colidindo entre passos. **forte indício**.
  - Alocações de `SequenceStep` e `Assignment` sem liberação explícita. **forte indício**.
- **Prioridade**: **alta**.
- **Observações**: modelo importante para roteamento; consistência de persistência precisa revisão prioritária.

## Classe: `Set`
- **Arquivos**: `source/plugins/data/Set.h`, `source/plugins/data/Set.cpp`
- **Papel aparente**: conjunto tipado de `ModelDataDefinition`.
- **Contrato implementado**:
  - Implementa `show`, fábricas, `_loadInstance`, `_saveInstance`, `_check`, `_createInternalAndAttachedData`, `_getParserChangesInformation`.
- **Problemas encontrados**:
  - `load` depende de ordem de carregamento global; membros ausentes resultam em erro fatal local sem estratégia de resolução tardia. **forte indício**.
  - `List<ModelDataDefinition*>` em heap sem liberação explícita na classe. **forte indício**.
- **Prioridade**: **média**.
- **Observações**: semântica principal está implementada; robustez de carga pode ser melhorada.

## Classe: `SignalData`
- **Arquivos**: `source/plugins/data/SignalData.h`, `source/plugins/data/SignalData.cpp`
- **Papel aparente**: sinalização para liberar entidades em componentes observadores.
- **Contrato implementado**:
  - Implementa `show`, fábricas, `_loadInstance`, `_saveInstance`, `_check`, `_initBetweenReplications` (vazio), `_createInternalAndAttachedData` (vazio).
  - `_getParserChangesInformation` não sobrescrito.
- **Problemas encontrados**:
  - Metadado de help ainda `//@TODO`. **confirmado no código**.
  - `_check()` exige handlers (>0), mas não há persistência de handlers (esperado runtime) e init vazio pode deixar estado sensível entre replicações. **forte indício**.
  - Lista de handlers alocada em heap sem limpeza explícita. **forte indício**.
- **Prioridade**: **média-alta**.
- **Observações**: mecanismo útil, porém ciclo de vida e contrato de replicação merecem reforço.

## Classe: `Station`
- **Arquivos**: `source/plugins/data/Station.h`, `source/plugins/data/Station.cpp`
- **Papel aparente**: estação lógica/física com estatísticas de ocupação e tempo.
- **Contrato implementado**:
  - Implementa `show`, fábricas, `_loadInstance`, `_saveInstance`, `_check`, `_createInternalAndAttachedData`.
  - `_initBetweenReplications` existe como método público `initBetweenReplications()` (não override protegido padrão).
- **Problemas encontrados**:
  - TODO explícito sobre checagem de `EntityType` reportando estatística antes de uso.
  - Potencial inconsistência de naming de atributo (`Entity.SequenceStep` vs documentação menciona Jobstep em outras áreas) depende de validação sistêmica. **hipótese a validar**.
- **Prioridade**: **média**.
- **Observações**: funcional para estatística/entrada/saída de estação.

## Classe: `Storage`
- **Arquivos**: `source/plugins/data/Storage.h`, `source/plugins/data/Storage.cpp`
- **Papel aparente**: capacidade/área/unidades por área.
- **Contrato implementado**:
  - Implementa `show`, fábricas, `_loadInstance`, `_saveInstance`, `_check`, `_getParserChangesInformation`.
  - `_createInternalAndAttachedData` e `_initBetweenReplications` não sobrescritos.
- **Problemas encontrados**:
  - Header menciona TODO de estatística de uso (`ProportionOfStorageUsage`) não implementada.
  - Validação `_check()` é praticamente neutra (sempre true). **confirmado no código**.
- **Prioridade**: **baixa-média**.
- **Observações**: base funcional simples; baixa urgência comparativa.

## Classe: `Variable`
- **Arquivos**: `source/plugins/data/Variable.h`, `source/plugins/data/Variable.cpp`
- **Papel aparente**: variável escalar/vetorial com valores iniciais e atuais.
- **Contrato implementado**:
  - Implementa `show`, fábricas, `_loadInstance`, `_saveInstance`, `_check`, `_initBetweenReplications`.
  - `_createInternalAndAttachedData` e `_getParserChangesInformation` não sobrescritos.
- **Problemas encontrados**:
  - `_initBetweenReplications()` faz `this->_values = this->_initialValues;` após `clear`, criando alias entre mapas e potencial perda de separação conceitual entre estado inicial/estado corrente. **confirmado no código**.
  - `_saveInstance()` não chama `ModelDataDefinition::_saveInstance()` (contrato de persistência base pode ficar incompleto se não for tratado externamente). **forte indício**.
  - Estruturas heap (`_dimensionSizes`, `_values`, `_initialValues`) sem liberação explícita na classe. **forte indício**.
- **Prioridade**: **alta**.
- **Observações**: risco funcional real em reinicialização entre replicações.

---

## 3) Ranking final — 5 melhores candidatas para iniciar correções

1. **`Queue`** (prioridade alta)  
   `OrderRule` é configurável e persistido, mas não executa ordenação efetiva (`TODO` explícito); impacto direto em lógica de filas.

2. **`Variable`** (prioridade alta)  
   `_initBetweenReplications()` cria alias entre estado inicial e atual; risco funcional direto em simulações com múltiplas replicações.

3. **`Resource`** (prioridade alta)  
   lacunas explícitas em prioridade de `seize` e persistência de falhas; alto impacto sistêmico.

4. **`Sequence`** (prioridade alta)  
   forte indício de inconsistência de persistência de assignments por passo + stubs vazios em `SequenceStep`.

5. **`SignalData`** (prioridade média-alta)  
   mecanismo de notificação com contrato de replicação frágil e risco de lifecycle de handlers.

---

## Anotações adicionais de validação de infraestrutura

- **Build**: projeto usa CMake modular; `source/plugins/data/CMakeLists.txt` cria `genesys_plugins_data` (STATIC) com C++23.
- **Testes**: há suíte em `source/tests` (unit/smoke) e gtest local, inclusive testes de suporte do runtime/plugin manager; não foi executada bateria nesta etapa por escopo focado em auditoria estática.

