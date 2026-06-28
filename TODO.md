Tema 5 — Conclusão das ações pendentes da interface gráfica do GenESyS
Tema 5 — Conclusão das ações pendentes da interface gráfica do GenESyS
Neste trabalho, o objetivo é concluir funcionalidades ainda pendentes da interface gráfica do GenESyS, mantendo o foco principal na camada gráfica e, em especial, nas animações. O nome do tema permanece o mesmo, e o propósito geral também permanece: completar, estabilizar e organizar corretamente aquilo que ainda não está concluído na GUI.

Entretanto, o recorte deste tema deve agora ser entendido de forma mais precisa. A interface gráfica já avançou bastante e várias ações de menu, controladores e fluxos básicos da GUI estão mais maduros do que quando este tema foi originalmente formulado. Por isso, o núcleo do trabalho deixa de ser a simples implementação de ações de menu e passa a ser a reorganização arquitetural das animações da GUI.

Em particular, este trabalho parte da seguinte distinção:

há animações que dizem respeito ao núcleo do GenESyS e, portanto, fazem sentido como parte fixa da GUI, como a animação do relógio de simulação, de contadores, de statistics collectors e de gráficos;
há animações que dizem respeito a plugins específicos do simulador, como filas, recursos, estações e outros componentes de domínio, e que, portanto, não deveriam estar embutidas rigidamente na GUI principal.
Assim, o objetivo central deste tema passa a ser: retirar da GUI fixa as animações específicas de plugins e transformá-las em plugins da interface gráfica, aproveitando a infraestrutura atual de extensibilidade gráfica do GenESyS.

1. Objetivo geral do tema
O aluno deverá:

identificar quais animações pertencem ao núcleo da interface gráfica e devem permanecer fixas na GUI;
identificar quais animações são semanticamente associadas a plugins do simulador e, portanto, não devem continuar fixas na GUI principal;
reorganizar a arquitetura da interface para que animações específicas de plugins sejam fornecidas por plugins gráficos;
consolidar as animações nucleares já existentes;
definir uma estrutura coerente para que novos plugins do GenESyS possam acrescentar também seus próprios comportamentos gráficos e animações;
garantir que a GUI continue estável, extensível e consistente após essa reorganização.

2. Ideia central da reorganização
Atualmente, a GUI do GenESyS já possui infraestrutura rica para desenho e ativação de animações, e isso deve ser aproveitado. O ponto em aberto não é mais “criar toda a infraestrutura de animação do zero”, mas sim separar corretamente o que é animação do núcleo do que é animação de plugin.

A regra conceitual desejada neste tema é a seguinte:

animações ligadas ao comportamento geral do simulador e a estruturas nucleares do GenESyS permanecem na GUI base;
animações ligadas a componentes que só existem quando determinados plugins estão carregados devem sair da GUI fixa e ser implementadas como plugins gráficos da interface.
Isso significa, por exemplo, que animações como:

relógio de simulação;
contadores;
statistics collectors;
gráficos gerais do simulador;
podem continuar sendo tratadas como parte da interface base do GenESyS, enquanto animações como:

filas;
recursos;
estações;
ou outras estruturas específicas de plugins;
devem migrar para uma arquitetura em que a própria camada gráfica associada ao plugin seja responsável por introduzir suas animações.

3. Novo foco técnico do trabalho
O foco principal deste tema deixa de ser a implementação de itens isolados do menu e passa a ser a transformação das animações específicas de plugins em plugins para a interface gráfica.

Em termos práticos, o trabalho deve se concentrar em três frentes:

consolidar as animações nucleares que devem permanecer na GUI base;
remover o acoplamento rígido entre a GUI principal e animações específicas de plugins;
criar a infraestrutura e/ou os plugins gráficos concretos que passem a fornecer essas animações de forma modular.
4. Onde este trabalho se concentra no repositório
O trabalho continua concentrado principalmente na aplicação gráfica Qt do GenESyS, em especial nas seguintes regiões:

source/applications/gui/qt/GenesysQtGUI/mainwindow.ui
source/applications/gui/qt/GenesysQtGUI/mainwindow.h
source/applications/gui/qt/GenesysQtGUI/mainwindow.cpp
source/applications/gui/qt/GenesysQtGUI/mainwindow_controller.cpp
source/applications/gui/qt/GenesysQtGUI/controllers/
source/applications/gui/qt/GenesysQtGUI/graphicals/
source/applications/gui/qt/GenesysQtGUI/animations/
Além disso, o aluno deverá localizar no código atual os pontos que já dão suporte à inclusão de plugins para interface gráfica, pois a solução desejada depende exatamente dessa extensibilidade.

5. Diagnóstico atualizado do estado da GUI
A GUI do GenESyS já evoluiu bastante. Hoje, várias ações da janela principal já estão encaminhadas por controllers específicos, e a cena gráfica já possui uma base de desenho e de animação muito mais madura do que anteriormente. Isso altera o diagnóstico:

o problema principal já não é mais fazer a ação de menu “chegar” a algum slot;
o problema principal passa a ser dar a arquitetura correta às animações e separar bem a camada gráfica nuclear da camada gráfica específica de plugins.
Em particular, o GenESyS já possui infraestrutura suficiente para:

acionar ferramentas de desenho de animações pela GUI;
armazenar listas de animações na cena gráfica;
suportar diferentes tipos de animação por modo de desenho;
manter animações que já fazem sentido como parte do núcleo gráfico.
O que falta é reorganizar essa base de modo que a GUI principal não fique acoplada a tipos de animação que dependem da existência de plugins específicos do simulador.

6. O que deve permanecer na GUI fixa
O aluno deverá considerar como candidatas naturais a permanecer na GUI base as animações relacionadas ao próprio núcleo do GenESyS, especialmente aquelas que fazem sentido independentemente de plugins específicos de modelagem. Entre elas, destacam-se:

animação do relógio de simulação;
animação de contadores;
animação de statistics collectors e elementos estatísticos gerais;
animações e estruturas gráficas gerais de gráficos de acompanhamento da simulação.
Essas animações devem ser consolidadas, estabilizadas e mantidas na GUI base.

7. O que deve migrar para plugins gráficos
O aluno deverá tratar como candidatas naturais à migração para plugins gráficos as animações que dependem da presença de plugins específicos do simulador, por exemplo:

animação de filas;
animação de recursos;
animação de estações;
outras animações ligadas a elementos de domínio que não pertencem ao núcleo mínimo do GenESyS.
A ideia é que, se determinado plugin do simulador não estiver carregado, sua camada gráfica associada também não deva estar embutida rigidamente na GUI principal. Em vez disso, o próprio plugin gráfico correspondente deve:

registrar a animação que oferece;
acrescentar itens de menu ou comandos associados;
fornecer os objetos gráficos e o comportamento de animação necessários;
integrar-se à cena gráfica do GenESyS de forma modular.
8. O que o aluno deve estudar com prioridade
8.1. Encaminhamento das ações da GUI
O aluno deve estudar:

mainwindow.ui
mainwindow_controller.cpp
controllers/SceneToolController.cpp
Esses arquivos mostram como as ações de animação atualmente são ativadas a partir da interface.

8.2. Cena gráfica e infraestrutura de animação
O aluno deve estudar com muita atenção:

graphicals/ModelGraphicsScene.h
graphicals/ModelGraphicsScene.cpp
Essa classe é hoje o centro real da infraestrutura gráfica e de animação. Ela deve ser analisada para identificar:

quais animações já estão semanticamente maduras;
quais animações ainda estão tratadas apenas como placeholders ou casos especiais;
quais delas pertencem ao núcleo e quais deveriam migrar para plugins gráficos.
8.3. Subpasta
animations/
A subpasta

animations/
deve continuar sendo usada como ponto de apoio importante. O aluno não deve abandonar essa infraestrutura, mas deve reorganizá-la conforme a nova separação entre:
animações nucleares;
animações associadas a plugins.
8.4. Infraestrutura de plugins gráficos
O aluno deverá localizar e estudar no código atual os mecanismos já existentes para extensibilidade da interface gráfica, incluindo:

adição dinâmica de itens de menu;
adição dinâmica de ações;
adição dinâmica de janelas ou ferramentas;
adição de itens gráficos e comportamentos gráficos por plugin.
Esta parte é central, porque a solução desejada para o tema depende justamente de usar a arquitetura de plugins gráficos já existente ou já iniciada na GUI do GenESyS.

9. O que se espera que o aluno implemente
O aluno deverá implementar, no mínimo:

a revisão arquitetural das animações da GUI, separando claramente animações nucleares e animações específicas de plugins;
a migração de um conjunto coerente de animações específicas de plugins para plugins gráficos da interface;
a consolidação das animações nucleares que permanecerão fixas na GUI principal;
a criação de plugins gráficos concretos para pelo menos alguns casos como fila, recurso, estação ou equivalentes;
a integração estável desses plugins com menus, cena gráfica e execução da simulação.
10. Direcionamento específico para as animações
10.1. Animações do núcleo
O aluno deve consolidar e estabilizar animações que façam sentido como parte da GUI base, como:

relógio de simulação;
contadores;
statistics collectors;
gráficos gerais do simulador.
10.2. Animações específicas de plugins
O aluno deve tratar como alvo principal de migração arquitetural animações como:

fila;
recurso;
estação;
outras animações ligadas a plugins específicos.
O objetivo não é apenas “fazer funcionar”, mas fazer com que essas animações deixem de ser dependências fixas da GUI e passem a existir de maneira modular.

11. O que não é o foco principal do trabalho
Este tema não deve ser interpretado como:

reescrita completa da interface gráfica;
reimplementação total da cena gráfica;
refatoração ampla sem critério da arquitetura Qt;
eliminação da infraestrutura já existente de animação.
O trabalho deve privilegiar:

mudanças pequenas e arquiteturalmente coerentes;
reaproveitamento da infraestrutura atual;
migração incremental das animações específicas de plugins;
preservação da estabilidade da GUI.
12. Requisitos mínimos do trabalho
o desenvolvimento deve ocorrer sobre o código real do GenESyS no branch 2026-1;
o tema deve manter o nome Conclusão das ações pendentes da interface gráfica do GenESyS;
o foco principal deve continuar sendo a interface gráfica e suas animações;
deve haver distinção clara entre animações nucleares e animações específicas de plugins;
deve haver migração de animações específicas de plugins para plugins gráficos da interface;
deve haver consolidação das animações nucleares que permanecerem fixas na GUI;
o código deve compilar e a GUI deve permanecer estável;
devem ser incluídos testes manuais sistemáticos e, quando viável, testes automatizados de integração.
13. O que os testes e a validação devem demonstrar
O aluno deverá demonstrar, no mínimo:

que as animações nucleares continuam funcionando corretamente na GUI;
que as animações específicas de plugins deixam de depender rigidamente da GUI principal;
que plugins gráficos conseguem acrescentar suas próprias animações e ações associadas;
que a GUI permanece estável com e sem os plugins gráficos correspondentes carregados;
que a integração entre animação, cena gráfica e simulação permanece correta após a reorganização arquitetural.
14. Informações adicionais com o professor
Para este tema, detalhes específicos sobre:

quais animações devem ser consideradas nucleares;
quais animações devem obrigatoriamente migrar para plugins gráficos;
o nível de detalhe visual esperado de cada animação;
o escopo adequado dos plugins gráficos a serem criados;
podem ser obtidos com o professor, sob demanda.

15. Entregável técnico esperado
Espera-se que o aluno entregue uma implementação que consolide a camada de animações da GUI do GenESyS e, principalmente, reorganize a arquitetura dessas animações para que aquilo que é específico de plugins do simulador passe a ser fornecido por plugins gráficos da interface.

16. Síntese final do que deve ser feito
Em termos objetivos, este trabalho pede ao aluno que mantenha o foco do Tema 5 na conclusão da interface gráfica e, sobretudo, das animações, mas com um recorte arquitetural atualizado: o aluno deverá preservar e consolidar as animações nucleares do GenESyS e transformar as animações específicas de plugins em plugins gráficos da interface, tornando a GUI mais modular, mais coerente e mais compatível com a natureza extensível do simulador.