# Mini relatorio: consolidacao EFSM no ModalModel

## Escopo
As alteracoes implementam a semantica minima esperada para as classes descritas na secao 6 do enunciado: `ModalModelDefault`, `ModalModelFSM`, `DefaultNode`, `DefaultNodeTransition` e `FSMState`. A implementacao se concentra em EFSM e nao altera a semantica de Petri alem da compatibilidade ja existente.

## Escolhas de projeto
- O evento de disparo foi integrado ao `DefaultNodeTransition::canFire`. Um `inputEvent` numerico representa a porta de entrada do evento (`inputPortNumber`); um `inputEvent` nao numerico e interpretado como expressao booleana do parser. Essa escolha reaproveita informacao que o simulador ja carrega no evento e evita criar uma infraestrutura paralela de eventos.
- A guarda continua sendo cumulativa com o evento. Uma transicao so dispara quando o evento bate e a guarda avalia para verdadeiro.
- A politica de selecao preserva a compatibilidade existente: menor valor de prioridade vence. A selecao probabilistica passa a ocorrer apenas entre transicoes habilitadas com a melhor prioridade.
- Empates deterministas preservam a ordem de cadastro das transicoes, usando ordenacao estavel. Isso torna o desempate reprodutivel sem introduzir uma nova configuracao.
- As acoes de estado foram executadas na ordem esperada para EFSM: acao de saida do estado atual, acao da transicao, acao de entrada do estado destino e atualizacao do atributo de estado modal da entidade.
- O estado modal continua sendo armazenado por indice no atributo `Entity.ModalModel.<Modal>.CurrentNode`, e o ultimo estado por id em `Entity.ModalModel.<Modal>.LastNode`. Durante o despacho, esses atributos sao criados na entidade caso ainda nao existam.
- No primeiro despacho de uma entidade, quando ainda nao ha estado modal registrado, o componente usa explicitamente o `entryNode`, mesmo que ele nao seja o nodo de indice zero.
- `ModalModelFSM` nao cria mais um `FSMState` interno implicito no construtor. O estado criado antes nao era inserido no submodelo, nao participava da simulacao e podia vazar memoria.

## Mudancas por classe
- `DefaultNodeTransition`: adicionada sobrecarga de `canFire` com evento de despacho, avaliacao de `inputEvent` e metodo virtual `effectiveProbability`.
- `EFSMTransition`: `triggerEvent` agora alimenta o mesmo campo usado pela transicao base, e `probabilityExpression` pode determinar a probabilidade efetiva em tempo de execucao.
- `ModalModelDefault`: validacao estrutural mais robusta, validacao de expressoes de guarda/evento/acao/probabilidade, selecao formal por prioridade/probabilidade, persistencia opcional de `EFSMTransition` e execucao das acoes de entrada e saida de `FSMState`.
- `ModalModelFSM`: validacao especifica para exigir nodos `FSMState` e ao menos um estado inicial.
- `FSMState`: a persistencia existente foi preservada; suas expressoes agora participam efetivamente do ciclo de execucao.

## Testes adicionados
Foi criado `source/tests/unit/test_modal_model_efsm.cpp`, cobrindo:
- evento de entrada e guarda como condicoes cumulativas;
- execucao de acao de saida, acao de transicao e acao de entrada durante a simulacao;
- atualizacao dos atributos `CurrentNode` e `LastNode`;
- uso correto do `entryNode` no primeiro despacho;
- preservacao retrocompativel da persistencia de transicoes base e suporte aos campos especificos de `EFSMTransition`;
- validacao de `ModalModelFSM` exigindo estado inicial.
