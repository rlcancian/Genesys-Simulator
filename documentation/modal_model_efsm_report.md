# Mini relatorio: consolidacao EFSM no ModalModel

## Escopo
As alteracoes consolidam uma EFSM minima para o componente `ModalModelFSM`. O objetivo e oferecer estados, transicoes com evento de disparo, guarda, acao de transicao, acoes de entrada/saida, atualizacao de estado por entidade e parada em estado final.

Esta implementacao mantem apenas a semantica necessaria para uma EFSM funcional no simulador. Campos antigos de prioridade, probabilidade e tipo de transicao podem existir em arquivos salvos anteriormente, mas sao ignorados no carregamento e nao sao gravados novamente.

## Separacao arquitetural
- `ModalModelDefault` permanece como base modal generica: cadastro de nodos/transicoes, persistencia comum, validacao estrutural, atributos anexados e hooks protegidos para especializacoes.
- `ModalModelFSM` concentra a semantica EFSM: despacho por evento de entrada, selecao de transicoes habilitadas, execucao das acoes de estado, tratamento de estado final, validacao de `FSMState` e persistencia dos campos especificos de `EFSMTransition`.
- `EFSMTransition` concentra o comportamento especifico da transicao EFSM: `triggerEvent`, guarda herdada e expressao de saida herdada.
- `DefaultNodeTransition` continua sendo uma transicao base generica. O campo `inputEvent` e persistido por compatibilidade, mas a interpretacao EFSM fica em `EFSMTransition`.

## Semantica implementada
- O primeiro despacho de uma entidade usa o `entryNode` quando ainda nao ha estado modal registrado.
- O estado atual e armazenado por indice no atributo `Entity.ModalModel.<Modal>.CurrentNode`.
- O ultimo estado alcancado e armazenado por id no atributo `Entity.ModalModel.<Modal>.LastNode`.
- Uma transicao EFSM fica habilitada quando o `triggerEvent` corresponde ao evento de despacho e a guarda avalia para verdadeiro.
- `triggerEvent` numerico representa a porta de entrada do evento. Valor nao numerico e tratado como expressao booleana do parser.
- Quando mais de uma transicao esta habilitada, dispara a primeira transicao cadastrada na lista de saida do estado atual.
- Ao disparar uma transicao EFSM, a ordem e: acao de saida do estado atual, acao da transicao, acao de entrada do estado destino e atualizacao dos atributos modais.
- Estado final interrompe o ciclo de disparos do despacho e direciona a entidade para a porta de saida 1, quando ela existir.

## Limites conhecidos
Esta e uma EFSM minima. Nao estao implementados:
- transicoes padrao;
- transicoes nao deterministicas como construto formal separado;
- separacao formal entre acoes de saida e acoes de atualizacao de estado.

Esses pontos devem ser tratados como extensoes futuras, nao como comportamento implicito desta entrega.

## Testes adicionados
`source/tests/unit/test_modal_model_efsm.cpp` cobre:
- preservacao do tipo `ModalModelFSM`;
- despacho generico de `ModalModelDefault` sem dependencias EFSM;
- evento de disparo e guarda como condicoes cumulativas de `EFSMTransition`;
- ordem de execucao das acoes: saida do estado, transicao e entrada do estado destino;
- atualizacao de `CurrentNode` e `LastNode`;
- saida por porta 0 para estado nao final e porta 1 para estado final;
- manutencao de `LastNode` quando nenhuma transicao esta habilitada;
- interrupcao do despacho quando o estado atual ja e final;
- validacao de `ModalModelFSM` exigindo `FSMState`, estado inicial e `EFSMTransition`;
- persistencia dos campos especificos minimos de `EFSMTransition`;
- tolerancia de carregamento para campos antigos de prioridade/probabilidade, sem grava-los novamente.
