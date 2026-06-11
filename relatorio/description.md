Desenvolvimento de Componente de Simulação no GenESyS

Esta atividade tem como objetivo proporcionar aos estudantes uma experiência prática de desenvolvimento em uma plataforma real de modelagem e simulação, de modo a aprofundar a compreensão sobre como simuladores representam o comportamento de sistemas do mundo real. O trabalho deverá estar vinculado ao GenESyS, plataforma de modelagem e simulação cujo código-fonte está disponível em repositório GitHub.

O tema específico de cada trabalho poderá variar, mas deverá envolver a implementação, extensão, correção ou validação de funcionalidades típicas de simuladores de sistemas, tais como: geração de números aleatórios, coleta de resultados, análise estatística, criação de componentes de modelagem, implementação de novas técnicas ou mecanismos de simulação, ou desenvolvimento de elementos relacionados a paradigmas e formalismos como cadeias de Markov, autômatos celulares, sistemas de equações diferenciais, elementos finitos ou abordagens correlatas.

Todo trabalho deverá ser desenvolvido sobre o código real do GenESyS, obtido a partir do repositório GitHub no branch 2026-1. O desenvolvimento poderá ser realizado por interface textual ou gráfica, conforme a natureza do tema. Recomenda-se o uso da versão textual do GenESyS, por terminal/shell, sempre que o trabalho não exigir interface gráfica. Nesses casos, a compilação deverá ser feita por CMake, utilizando a configuração correspondente à aplicação terminal do GenESyS. Para trabalhos que envolvam interface gráfica, recomenda-se o uso da IDE Qt Creator.

O trabalho será desenvolvido ao longo do semestre, com prazo aproximado de dois meses. Durante esse período, os estudantes poderão consultar o professor para esclarecimentos e orientação técnica.
Entregáveis obrigatórios

A entrega deverá ser realizada no Moodle e deverá incluir:

    o código-fonte produzido ou modificado, incluindo todos os arquivos necessários à avaliação;
    um pull request sem conflitos direcionado ao branch 2026-1 do GenESyS;
    um relatório técnico descrevendo de forma objetiva o trabalho desenvolvido.

Requisitos mínimos do desenvolvimento

    o trabalho deve estar efetivamente integrado ao GenESyS;
    as modificações devem ser tecnicamente consistentes com a arquitetura do simulador;
    deve haver evidência clara de funcionamento correto da solução implementada;
    devem ser incluídos testes unitários e, quando pertinente, testes de integração ou validações experimentais;
    o estudante deve demonstrar como a funcionalidade desenvolvida se conecta ao restante do simulador.

Estrutura esperada do relatório

O relatório deverá ser sucinto, técnico e bem organizado, contendo pelo menos:

    Capa;
    Introdução, com contextualização do tema, objetivo do trabalho e justificativa;
    Desenvolvimento, com descrição do que foi implementado, corrigido ou estendido, arquivos modificados, classes envolvidas, decisões de projeto adotadas e forma de integração com o GenESyS;
    Validação, com apresentação dos testes realizados, resultados obtidos e evidências de funcionamento;
    Conclusão, com síntese do trabalho e principais resultados alcançados;
    Referências, quando houver uso de bibliografia, documentação, artigos ou materiais técnicos.

Critérios gerais de avaliação

    adequação do tema ao contexto de modelagem e simulação;
    qualidade técnica da implementação;
    correção, clareza e integração da solução ao GenESyS;
    qualidade e abrangência dos testes;
    qualidade técnica do relatório;
    organização da entrega no Moodle e do pull request.

Espera-se que o estudante demonstre capacidade de compreender a arquitetura de um simulador, modificar ou estender uma base de código real e justificar tecnicamente as decisões tomadas durante o desenvolvimento.
