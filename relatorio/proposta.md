NOSSO DESENVOLVIMENTO:
    Tema 11.3.1 - Foco maior em diagnósticos adicionais e ajustes de curvas


Tema 11 - Ferramenta de Análise de Dados no GenESyS:

Ferramenta de Análise de Dados no GenESyS

Neste trabalho, o objetivo é desenvolver, no pacote
tools
do GenESyS, uma ferramenta unificada de análise de dados, capaz de trabalhar tanto com dados de entrada quanto com dados de saída de simulação. A proposta continua sendo a de um analisador geral de dados, conceitualmente análogo ao papel combinado de um Input Analyzer e de um Output Analyzer, mas o foco do desenvolvimento deve ser ajustado ao estado atual do código do GenESyS: o aluno não deve reimplementar tudo do zero, e sim integrar, completar e consolidar aquilo que já existe.

A ferramenta deverá receber dados por arquivo ou por estruturas em memória, produzir síntese estatística, executar ajuste de distribuições, realizar inferência estatística e fornecer estruturas numéricas de apoio à análise exploratória. Este tema não exige nenhuma ferramenta gráfica. Embora a ferramenta deva fornecer dados que normalmente alimentam histogramas, boxplots, correlogramas e análises semelhantes, o trabalho deve se concentrar apenas na implementação de métodos de cálculo e de estruturas de saída. Em outras palavras, o aluno não precisa desenhar gráficos; basta implementar métodos que devolvam, por exemplo, quartis, limites de classes, frequências, séries de autocorrelação e demais informações que futuramente possam ser usadas por uma interface visual.
1. Objetivo geral do tema

O aluno deverá desenvolver uma ferramenta capaz de:

    receber dados por arquivo ou por estruturas em memória;
    calcular medidas descritivas e de dispersão;
    fornecer estruturas numéricas para histogramas, boxplots e análises temporais;
    ajustar distribuições de probabilidade a dados observados;
    executar testes de hipótese de uma população e de duas populações;
    fornecer corretamente estatísticas de teste, intervalos de confiança e p-valores;
    permitir configuração explícita do nível de confiança e do nível de significância;
    realizar análise temporal básica, como médias móveis e correlograma/autocorrelação;
    servir como base para análise de dados tanto de entrada quanto de saída de simulação.

2. Posição do tema dentro do GenESyS

Este tema se encaixa principalmente no pacote
source/tools/
, mas deve reutilizar fortemente infraestrutura já existente em
source/kernel/statistics/
e em algumas ferramentas analíticas já presentes no próprio pacote
tools
.

Em termos arquiteturais, este tema não deve ser tratado como um desenvolvimento do zero. O GenESyS já possui blocos importantes que podem e devem ser reutilizados:

    infraestrutura para ingestão e organização de datasets;
    estatísticas baseadas em dados completos;
    interfaces de ajuste de distribuições;
    interfaces de testes de hipótese;
    distribuições e funções de probabilidade;
    solvers numéricos e integrais usados por parte da inferência estatística.

3. O que já existe no repositório
3.1. Estruturação e leitura de datasets

O pacote
tools
já possui uma base concreta para ingestão e organização de dados por meio de estruturas voltadas a datasets de resultados de simulação. Isso significa que a ferramenta de análise de dados já pode partir de uma camada existente para:

    carregar dados a partir de arquivos texto;
    reconhecer formatos distintos de datasets;
    armazenar observações, replicações e, quando houver, tempo associado às observações;
    extrair vetores de valores e subconjuntos por replicação.

Portanto, a parte de ingestão de dados já possui uma base real e não precisa ser reinventada. O trabalho do aluno deve consistir em integrar essa base à nova ferramenta.
3.2. Estatísticas descritivas baseadas em dados completos

O kernel estatístico já possui a interface
StatisticsDataFile_if
, que prevê operações como:

    mode()
    mediane()
    quartil(...)
    decil(...)
    centil(...)
    setHistogramNumClasses(...)
    histogramNumClasses()
    histogramClassLowerLimit(...)
    histogramClassFrequency(...)

Isso significa que o GenESyS já possui uma base importante para medidas posicionais, estatística descritiva baseada em ordenação e estruturas numéricas de histograma. Portanto, a parte descritiva não precisa ser reimplementada do zero, mas deve ser exposta de forma mais clara e unificada ao usuário da nova ferramenta.
3.3. Ajuste de distribuições

O pacote
tools
já possui a interface
Fitter_if
, que define operações de ajuste para distribuições como:

    Uniforme;
    Triangular;
    Normal;
    Exponencial;
    Erlang;
    Beta;
    Weibull;
    e uma rotina geral
    fitAll(...)
    .

Contudo, a implementação concreta hoje existente ainda é apenas um placeholder. Isso significa que o contrato do fitting já existe, mas a implementação funcional do ajuste de curvas ainda precisa ser consolidada. Portanto, essa continua sendo uma das partes mais importantes do tema.
3.4. Testes de hipótese e intervalos de confiança

O pacote
tools
também já possui a interface
HypothesisTester_if
, bastante ampla, prevendo:

    intervalos de confiança de média, proporção e variância;
    intervalos de confiança para diferenças entre duas populações;
    testes de hipótese de média, proporção e variância para uma população;
    testes de hipótese de média, proporção e variância para duas populações;
    versões baseadas em parâmetros e versões baseadas em arquivos de dados.

A implementação concreta já existente executa boa parte dessa inferência paramétrica. Portanto, esta parte do tema já está mais madura e não deve ser o foco central da implementação. O trabalho do aluno deve reaproveitar essa infraestrutura e concentrar esforço no que ainda falta, especialmente em testes adicionais de aderência e na integração em uma ferramenta única.
3.5. Infraestrutura probabilística e matemática

O GenESyS já possui infraestrutura de distribuições e quantis por meio de classes probabilísticas e de suporte numérico. Essa infraestrutura já é usada pela implementação atual dos testes de hipótese e deve ser reaproveitada pelo aluno sempre que possível.
4. Diagnóstico do estado atual

O estado atual do GenESyS sugere que este tema não é mais o de criar uma ferramenta completamente nova, mas sim o de integrar e completar capacidades que hoje já existem em partes.

Em resumo:

    a parte de ingestão e estruturação de datasets já possui uma base concreta;
    a parte de estatística descritiva já possui contrato e infraestrutura relevante;
    a parte de inferência paramétrica já possui uma implementação importante;
    a parte de ajuste de distribuições ainda é o maior vazio funcional;
    a parte de integração em uma ferramenta única ainda precisa ser feita;
    a parte de diagnósticos adicionais, como qui-quadrado, correlograma e médias móveis, ainda precisa ser consolidada.

Portanto, embora muita coisa já exista, ainda há desenvolvimento real suficiente para manter o grau de complexidade do tema, desde que o foco seja corretamente ajustado.
5. Escopo funcional esperado da ferramenta
5.1. Entrada de dados

    carregamento de dados a partir de arquivo;
    recebimento de dados diretamente em memória, por vetor ou lista;
    suporte a uma ou duas amostras, conforme o tipo de análise;
    integração com a estrutura de dataset já existente no GenESyS.

5.2. Estatística descritiva

    número de observações;
    mínimo, máximo e amplitude;
    média, mediana e moda;
    quartis, decis e percentis/centis;
    variância, desvio-padrão e coeficiente de variação;
    eventualmente assimetria e curtose, se o aluno decidir incluir.

5.3. Estruturas numéricas para análise exploratória futura

A ferramenta não precisa gerar gráficos, mas deve produzir os dados que seriam usados por uma futura ferramenta visual. Entre as saídas esperadas, incluem-se:

    classes de histograma, com limites inferiores e frequências;
    valores necessários para boxplot, como mínimo, quartil 1, mediana, quartil 3 e máximo;
    eventualmente estruturas auxiliares para diagnósticos de aderência, se o escopo permitir.

5.4. Ajuste de distribuições

Esta continua sendo a parte mais importante do tema. O aluno deverá implementar ou consolidar:

    ajuste de distribuições clássicas;
    comparação entre distribuições candidatas;
    métrica de erro ou aderência;
    seleção da melhor candidata;
    interface unificada para fitting a partir de dados.

5.5. Inferência estatística

A ferramenta deve integrar as rotinas já existentes de:

    intervalos de confiança com nível configurável;
    testes de hipótese de uma população;
    testes de hipótese de duas populações;
    testes de média, proporção e variância;
    cálculo de p-valores.

Aqui, o foco não é reinventar a inferência paramétrica já implementada, mas oferecer uma API de mais alto nível que a torne mais simples de usar dentro da nova ferramenta.
5.6. Testes adicionais

    teste de qui-quadrado;
    idealmente, teste de Kolmogorov–Smirnov, se couber no escopo final;
    outras rotinas de aderência, se o aluno julgar pertinente e conseguir implementá-las com consistência.

5.7. Análise temporal básica

    médias móveis;
    autocorrelação;
    correlograma, entendido aqui como a estrutura numérica da autocorrelação por defasagem.

6. Direção arquitetural recomendada

A recomendação mais forte é que o aluno crie uma nova ferramenta de alto nível, por exemplo:

    DataAnalyzer_if
    DataAnalyzerDefaultImpl1

Essa ferramenta deve funcionar como uma fachada unificada, reutilizando internamente:

    a estrutura de datasets já existente;
    StatisticsDataFile_if
    para estatística descritiva e histogramas;
    Fitter_if
    e uma implementação concreta real para fitting;
    HypothesisTester_if
    para inferência estatística;
    infraestrutura probabilística e numérica já disponível no GenESyS.

Em outras palavras, o objetivo não é apenas acrescentar métodos soltos às classes já existentes, mas criar uma ferramenta de análise de dados coerente e integrada.
7. Classes e arquivos que o aluno deve estudar
7.1. Infraestrutura do pacote tools

    source/tools/CMakeLists.txt

O aluno deve estudá-lo para entender quais classes já compõem o pacote e como a nova ferramenta deverá ser integrada ao build.
7.2. Estruturação de datasets

    source/tools/SimulationResultsDataset.h
    source/tools/SimulationResultsDataset.cpp

Esses arquivos devem ser estudados para reaproveitamento da lógica de ingestão, organização e extração de dados.
7.3. Estatística descritiva

    source/kernel/statistics/StatisticsDataFile_if.h

Essa interface deve ser estudada para entender a base já existente de medidas posicionais, histogramas e estatística baseada em dados completos.
7.4. Ajuste de distribuições

    source/tools/Fitter_if.h
    source/tools/FitterDummyImpl.h
    source/tools/FitterDummyImpl.cpp

Esta é a região mais importante do ponto de vista de desenvolvimento novo. O aluno deve decidir se irá completar a implementação atual ou criar uma nova implementação concreta preservando a interface já existente.
7.5. Inferência estatística

    source/tools/HypothesisTester_if.h
    source/tools/HypothesisTesterDefaultImpl1.h
    source/tools/HypothesisTesterDefaultImpl1.cpp

Esses arquivos devem ser estudados para reaproveitar a inferência já pronta e identificar as lacunas ainda presentes.
7.6. Infraestrutura matemática e probabilística

O aluno deve estudar também as classes probabilísticas e numéricas já usadas pelas implementações atuais, sobretudo aquelas responsáveis por quantis, distribuições, integrais e funções auxiliares.
8. O que realmente deve ser desenvolvido

O aluno não precisa reimplementar toda a estatística descritiva e toda a inferência já cobertas pela infraestrutura atual. O desenvolvimento real deste tema deve se concentrar em:

    criar a nova ferramenta de alto nível unificada para análise de dados;
    implementar de forma funcional o ajuste de distribuições, hoje ainda em estado placeholder;
    integrar a estatística descritiva já existente à nova ferramenta;
    integrar a inferência estatística já existente à nova ferramenta;
    acrescentar testes adicionais como qui-quadrado e, idealmente, Kolmogorov–Smirnov;
    implementar estruturas numéricas para médias móveis, autocorrelação e correlograma;
    padronizar a saída dos métodos para uso futuro por interfaces visuais, sem implementar gráficos.

9. Métodos que provavelmente precisarão ser criados

Como a ferramenta unificada ainda não aparece formalizada, espera-se que o aluno proponha métodos novos, por exemplo:

    setDataFilename(...)
    setDataValues(...)
    clearData()
    loadSecondSample(...)
    summaryStatistics()
    histogramStructure(...)
    boxplotStatistics()
    movingAverage(...)
    autocorrelation(...)
    correlogram(...)
    fitAll()
    fitDistribution(...)
    chiSquareGoodnessOfFit(...)
    kolmogorovSmirnov(...)
    , se incluído;
    métodos integrados para reaproveitar
    testAverage(...)
    ,
    testProportion(...)
    e
    testVariance(...)
    na nova ferramenta.

A nomenclatura exata pode variar, mas a API final deve ser coerente, previsível e independente de qualquer camada gráfica.
10. O que não faz parte do escopo

Este tema não exige:

    janela gráfica;
    renderização de histogramas;
    renderização de boxplots;
    renderização de correlogramas;
    qualquer biblioteca visual de plotagem.

O aluno deve implementar apenas os cálculos e estruturas de saída que uma ferramenta gráfica futura possa consumir.
11. Requisitos mínimos do trabalho

    o desenvolvimento deve ocorrer sobre o código real do GenESyS no branch 2026-1;
    o trabalho deve manter o nome e a natureza de ferramenta de análise de dados no GenESyS;
    deve continuar cobrindo dados de entrada e dados de saída;
    o aluno deve criar ou consolidar uma ferramenta unificada de alto nível;
    deve haver estatística descritiva, fitting funcional, inferência estatística e diagnósticos adicionais;
    deve haver suporte explícito a níveis de confiança/significância configuráveis;
    não deve ser exigida qualquer camada gráfica;
    o código deve compilar e integrar-se ao restante do GenESyS sem quebrar os demais pacotes.

12. O que os testes devem demonstrar

O aluno deverá demonstrar, no mínimo:

    carregamento correto de dados por arquivo e por memória;
    uso correto da estrutura de dataset já existente;
    cálculo correto de estatísticas descritivas;
    geração correta das estruturas numéricas de histograma e boxplot;
    funcionamento do fitting em distribuições clássicas;
    integração correta com os testes estatísticos já existentes;
    funcionamento de pelo menos um novo teste de aderência;
    funcionamento de médias móveis e autocorrelação/correlograma, se incluídos no escopo final.

13. Entregável técnico esperado

Espera-se que o aluno entregue uma implementação que transforme os blocos estatísticos já existentes do GenESyS em uma ferramenta integrada de análise de dados, útil tanto para análise de dados de entrada quanto para análise de dados de saída de simulação, sem exigir qualquer componente gráfico.
14. Síntese final do que deve ser feito

Em termos objetivos, este trabalho pede ao aluno que crie, no GenESyS, um analisador geral de dados, capaz de unificar estatística descritiva, ajuste de distribuições, inferência estatística e análise exploratória/temporal. O aluno deverá reaproveitar o que já existe, especialmente na ingestão de datasets, na estatística descritiva e na inferência paramétrica, e concentrar o desenvolvimento real principalmente em: fitting funcional, integração das capacidades em uma ferramenta única e estruturas numéricas de saída para uso futuro por ferramentas visuais.
