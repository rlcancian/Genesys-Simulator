# Capa

## Desenvolvimento de Componente de Simulação

## Ferramenta de Análise de Dados para o GenESyS

**Título do DCS:** Ferramenta de Análise de Dados para Ajuste de Distribuições e Inferência Paramétrica no GenESyS

**Grupo:** `11.3.2 - Foco em ajuste de distribuições e inferência paramétrica`

**Integrantes e matrículas:**

| Integrante | Matrícula |
| --- | --- |
| `Henrique Mateus Teodoro` | `23100472` |
| `Jonatan Felipe Hartmann` | `23104231` |
| `Rodrigo Schwartz` | `23100471` |

**Disciplina:** `INE5425 - Modelagem e Simulação`

**Professor:** `Rafael Luiz Cancian`

**Data considerada:** 2026-06-23

**Repositório/base de comparação:** branch `dev` comparada com `upstream/2026-1` do repositório `rlcancian/Genesys-Simulator`. Pull request aberto: https://github.com/rlcancian/Genesys-Simulator/pull/424

---

# Introdução

## Contextualização

O Desenvolvimento de Componente de Simulação (DCS) requer o projeto, a implementação, o teste e a documentação de um componente de software em C++, vinculado ao simulador GenESyS. O componente pode representar um bloco de simulação ou uma funcionalidade típica de simuladores de sistemas, como tratamento de dados, geração de variáveis aleatórias, projeto experimental ou análise estatística.

Este trabalho se enquadra na categoria de ferramenta de análise de dados. Em modelos de simulação, a análise estatística é necessária tanto antes da execução, para caracterizar dados de entrada e ajustar distribuições, quanto depois da execução, para avaliar resultados simulados, estimar parâmetros e apoiar decisões com inferência estatística.

A branch `dev` consolidou uma ferramenta de análise em `source/tools/analysis`, com foco em ajuste de distribuições, estatísticas descritivas, intervalos de confiança, testes de hipóteses paramétricos e testes de aderência. A implementação foi estruturada para ser utilizada diretamente por código C++, por exemplos, por testes, pela GUI ou por outros consumidores do GenESyS.

## Objetivo do Trabalho

O objetivo principal foi transformar a análise de dados em uma ferramenta independente dentro do pacote `tools`, exposta por uma fachada simples e desacoplada do `source/kernel`.

Os objetivos específicos foram:

- consolidar `DataAnalyserDefaultImpl` como ponto de entrada da ferramenta;
- permitir entrada de dados por arquivo e por memória;
- expor resumo estatístico, histograma e boxplot numéricos;
- consolidar o ajuste de distribuições e o ranking estruturado de ajustes;
- consolidar testes de hipóteses e intervalos de confiança;
- implementar testes de aderência chi-square e Kolmogorov-Smirnov;
- manter a ferramenta de análise sem dependência direta do kernel;
- fornecer exemplos executáveis, testes unitários, testes de integração, diagramas e documentação.

## Justificativa

A consolidação da ferramenta é relevante porque análise de dados é uma funcionalidade recorrente em simuladores de eventos discretos. Sem uma ferramenta integrada, o usuário precisa exportar dados para pacotes externos ou repetir cálculos estatísticos manualmente. Com este componente, o GenESyS passa a oferecer um backend de análise reutilizável, testável e documentado.

A decisão de desacoplar `tools/analysis` do kernel também melhora a manutenibilidade. A direção arquitetural adotada permite que aplicações, GUI, exemplos, testes ou módulos do kernel dependam da ferramenta de análise, sem que a ferramenta dependa de detalhes internos do kernel. Essa separação facilita o uso direto do componente em código e reduz o risco de que mudanças no motor de simulação quebrem a análise estatística.

# Desenvolvimento

## Escopo Implementado

As funcionalidades consolidadas incluem:

- carregamento de datasets por arquivo e por memória;
- resumo estatístico mínimo;
- histogramas numéricos;
- estrutura numérica de boxplot;
- ajuste de distribuições;
- ranking estruturado de ajustes;
- intervalos de confiança;
- testes de hipóteses paramétricos;
- testes de aderência chi-square e Kolmogorov-Smirnov;
- exemplo executável analítico;
- exemplo executável com modelagem, simulação e análise;
- testes unitários e de integração;
- documentação técnica, diagramas UML e registro de resultados.

Permaneceram fora do escopo funcional atual, mas mantidos na interface por compatibilidade e roadmap:

- `DataAnalyserDefaultImpl::newDataSet`;
- `DataAnalyserDefaultImpl::saveDataSet`;
- `DataAnalyserDefaultImpl::sampler`;
- `DataAnalyserDefaultImpl::experimenter`.

Quando não há colaborador externo injetado, esses caminhos lançam `std::runtime_error` indicando que ainda são escopo futuro.

## Casos de Uso Considerados

| Caso de uso | Descrição | Status |
| --- | --- | --- |
| UC01 - Carregar dataset de arquivo | Usuário informa um arquivo numérico ou resultado de simulação para carregar observações. | Implementado |
| UC02 - Carregar dataset em memória | Usuário passa `std::vector<double>` diretamente para a fachada. | Implementado |
| UC03 - Obter resumo estatístico | Usuário consulta contagem, mínimo, máximo, média, variância, desvio padrão e flag de negativos. | Implementado |
| UC04 - Obter histograma numérico | Usuário solicita classes e frequências para inspeção exploratória. | Implementado |
| UC05 - Obter boxplot numérico | Usuário solicita quartis, cercas, whiskers e outliers. | Implementado |
| UC06 - Ajustar distribuições | Usuário ajusta distribuições conhecidas ao dataset carregado. | Implementado |
| UC07 - Obter ranking de ajustes | Usuário recebe ranking estruturado das distribuições por erro. | Implementado |
| UC08 - Calcular intervalos de confiança | Usuário calcula ICs para média, proporção, variância, diferenças e razão de variâncias. | Implementado |
| UC09 - Executar testes paramétricos | Usuário testa médias, proporções e variâncias para uma ou duas populações. | Implementado |
| UC10 - Executar testes de aderência | Usuário executa chi-square e KS com CDF especificada. | Implementado |
| UC11 - Usar overloads baseados em arquivo | Usuário executa inferências diretamente a partir de arquivos. | Implementado |
| UC12 - Usar exemplo integrado | Usuário executa exemplos completos da ferramenta. | Implementado |
| UC13 - Sampler e experimenter pela fachada | Mantidos como pontos de extensão futuros. | Fora do escopo atual |
| UC14 - `newDataSet` e `saveDataSet` | Mantidos por compatibilidade/roadmap. | Fora do escopo atual |

## Arquitetura Geral

A arquitetura foi reorganizada para centralizar o componente em `source/tools/analysis`. O ponto de entrada principal é `DataAnalyserDefaultImpl`, que implementa `DataAnalyser_if` e funciona como fachada da ferramenta.

| Componente | Responsabilidade |
| --- | --- |
| `DataAnalyser_if` | Contrato público da fachada de análise. |
| `DataAnalyserDefaultImpl` | Carrega dataset, expõe estatísticas descritivas e fornece fitter/tester. |
| `DatasetLoader` | Carrega dados, valida observações e calcula estatísticas básicas. |
| `SimulationResultsDataset` / `SimulationResultsParser` | Leem formatos de resultado do GenESyS sem depender do kernel. |
| `Fitter_if` / `FitterDefaultImpl` | Ajustam distribuições e produzem ranking de aderência. |
| `HypothesisTester_if` / `HypothesisTesterDefaultImpl` | Calculam ICs, testes paramétricos e testes de aderência. |
| `ProbabilityDistributionBase` / `ProbabilityDistribution` | Fornecem funções matemáticas de densidade, massa, CDF auxiliar e quantis. |
| `Solver_if` / `SolverDefaultImpl` | Fornecem integração numérica reutilizada pela ferramenta. |
| `TraitsAnalysis` | Centraliza tipos padrão, constantes numéricas e definições da ferramenta. |

## Diagramas

### Diagrama de classes

![Class overview](../../source/tools/analysis/diagrams/analysis_class_overview.png)

### Diagrama de casos de uso

![Use cases](../../source/tools/analysis/diagrams/analysis_use_cases.png)

### Diagrama de sequência do fluxo principal

![Dataset flow sequence](../../source/tools/analysis/diagrams/analysis_dataset_flow_sequence.png)

### Diagrama de fronteiras de dependência

![Dependency boundaries](../../source/tools/analysis/diagrams/analysis_dependency_boundaries.png)

## Integração com o GenESyS

O componente é exposto por:

```text
genesys_tools_analysis
```

O alvo é definido em `source/tools/CMakeLists.txt` e contém apenas o pacote de análise e utilitários locais necessários. Ele deve compilar sem linkar `genesys_kernel_*`.

A regra arquitetural adotada foi:

```text
applications / GUI / tests / kernel consumers
    -> genesys_tools_analysis
    -> helpers locais de analysis
```

Assim, se um módulo do kernel, GUI ou aplicação precisar de análise estatística, ele inclui e vincula `genesys_tools_analysis`. A dependência inversa não deve existir.

Também foram adicionados:

- opção `GENESYS_BUILD_EXAMPLES`;
- preset `examples`;
- executável `genesys_examples_analysis_tools`;
- executável `genesys_examples_simulation_analysis`;
- alvo agregador `genesys_examples`;
- alvo `genesys_tools_unit_tests`;
- alvo `genesys_tools_integration_tests`.

O projeto pode ser aberto no QtCreator usando os `CMakeLists.txt`/presets e os alvos CMake podem ser executados diretamente pela IDE. O `Makefile` foi criado apenas como uma camada de conveniência para terminal, sem substituir CMake ou QtCreator.

## Arquivos Modificados ou Adicionados

A branch `dev` foi comparada com `upstream/2026-1` após atualizar a referência remota com:

```sh
git fetch upstream 2026-1
```

Resumo do diff:

```text
69 files changed, 8073 insertions(+), 3375 deletions(-)
```

Principais áreas alteradas:

| Área | Principais mudanças |
| --- | --- |
| `source/tools/analysis` | Criação/consolidação do pacote principal de análise de dados. |
| `source/tools` | Criação do alvo `genesys_tools_analysis`, reorganização dos arquivos de análise, README e arquitetura. |
| `source/tests/unit/tools/analysis` | Testes unitários dedicados para dataset, fitter, hypothesis tester e fachada. |
| `source/tests/integration/tools/analysis` | Testes de integração ponta a ponta da ferramenta. |
| `examples` | Exemplos executáveis e datasets de exemplo. |
| `documentation` | Ajuste de Doxygen developer 2026, notas históricas e este relatório. |
| Build/CMake/Makefile | Preset de exemplos, alvo de análise, alvos de testes e atalhos de execução. |
| GUI/Qt `.pro` e consumidores | Ajustes de inclusão/link para o novo pacote de análise quando necessário. |

Commits principais da branch `dev` relacionados ao componente:

```text
feat: DatasetLoader and refactor fitter
refactor: move data analysis files into tools/analysis/ subdirectory
add tests for fitter, dataset loader and simulation results dataset
feat: consolidate analysis fitter and hypothesis tester defaults
feat: add goodness-of-fit tests to analysis tools
feat: resumo estatístico descritivo
feat: histogramas e boxplot
feat: ranking de distribuições no fitter
fix: chiSquareGoodnessOfFit
fix: garante que fitter e hypothesis usem o mesmo dataset
refactor: desacoplamento de ProbabilityDistribution
feat: diagrams
feat: integration tests
```

## Refatorações, Adições e Remoções

### Refatorações

- Arquivos de análise foram movidos de `source/tools` para `source/tools/analysis`.
- `TraitsTools.h` foi consolidado como `TraitsAnalysis.h`.
- `ProbabilityDistribution` e `ProbabilityDistributionBase` foram movidos para `tools/analysis`, mantendo a API matemática local ao pacote.
- `Solver_if` e `SolverDefaultImpl1` foram movidos para `tools/analysis` e consolidados como `Solver_if` e `SolverDefaultImpl`.
- `TraitsAnalysis<Solver_if>` passou a definir `SolverDefaultImpl` como implementação padrão para quadratura numérica da análise.
- `HypothesisTesterDefaultImpl1` foi substituído/consolidado por `HypothesisTesterDefaultImpl`.
- `FitterDefaultImpl` passou a usar `Fitter_if` e dataset em memória/arquivo de forma alinhada.
- Os overloads baseados em arquivo do `HypothesisTesterDefaultImpl` passaram a usar parser/loader locais da análise.

### Adições

- Alvo CMake `genesys_tools_analysis`.
- `DataAnalyserDefaultImpl` funcional.
- `DatasetLoader`.
- `SimulationResultsDataset` e `SimulationResultsParser`.
- Entrada por memória na fachada.
- `DataSetSummary`, `DataSetHistogram`, `HistogramBin`, `DataSetBoxPlot`.
- `FittingResult`, `FittedParameter`, `FitSummary`.
- `fitAllSummary()`.
- Testes chi-square e Kolmogorov-Smirnov.
- Testes unitários dedicados de `tools/analysis`.
- Testes de integração dedicados de `tools/analysis`.
- Exemplo executável analítico em `examples/analysis_tools_example.cpp`.
- Exemplo executável de simulação e análise em `examples/simulation_analysis_example.cpp`.
- Datasets de exemplo.
- READMEs e diagramas da ferramenta.

### Remoções ou Substituições

- Implementações antigas soltas em `source/tools` foram removidas ou movidas para `source/tools/analysis`.
- `SolverDefaultImpl1` foi substituído por `analysis/SolverDefaultImpl`; consumidores passaram a importar a nova localização.
- Dependências diretas de `source/kernel` foram retiradas da ferramenta de análise.
- A ferramenta deixou de depender de coletores estatísticos do kernel para operar em arquivos/datasets.

## Decisões de Projeto

### Fachada como ponto de entrada

Foi adotada uma fachada (`DataAnalyserDefaultImpl`) para simplificar o uso direto por código C++ e por futuros consumidores GUI. O usuário pode criar:

```cpp
DataAnalyserDefaultImpl analyser;
```

sem precisar instanciar manualmente o fitter ou o tester padrão. Esses defaults são definidos por `TraitsAnalysis`.

### Dataset único e consistente

A fachada mantém uma cópia validada do dataset em `DatasetLoader`. Ao carregar um dataset, a mesma amostra é propagada para o `FitterDefaultImpl`. Isso evita que summary, histograma, boxplot e fitting usem bases diferentes.

Para testes de aderência que exigem dados crus, a recomendação documentada é passar `analyser.data()` ao `HypothesisTesterDefaultImpl`.

### APIs estruturadas

Foram adicionadas estruturas de retorno em vez de depender apenas de ponteiros de saída:

- `DataSetSummary`;
- `DataSetHistogram`;
- `DataSetBoxPlot`;
- `FittingResult`;
- `FitSummary`;
- `TestResult`;
- `GoodnessOfFitDetails`.

As assinaturas legadas baseadas em ponteiros foram preservadas quando necessário por compatibilidade.

### Validação explícita

A implementação valida:

- datasets vazios;
- valores não finitos;
- níveis de confiança fora de `(0, 1)`;
- tamanhos amostrais inválidos;
- frequências esperadas inválidas;
- CDFs inválidas;
- graus de liberdade não positivos.

Falhas de carregamento retornam `false` na fachada. Falhas matemáticas de uso incorreto lançam `std::invalid_argument` quando apropriado.

### Solver reutilizado como dependência interna

O solver não constitui uma funcionalidade nova implementada por este DCS. As implementações de `Solver_if` e `SolverDefaultImpl1` já existiam no projeto e foram reutilizadas sem alteração de seus algoritmos numéricos, da regra de Simpson 1/3, das sobrecargas de derivação ou de seu comportamento matemático.

O trabalho desta entrega foi arquitetural: mover o código para `tools/analysis`, renomear `SolverDefaultImpl1` para `SolverDefaultImpl`, ajustar includes e alvos de compilação e registrar sua implementação padrão em `TraitsAnalysis`. Assim, o solver passou a ser uma dependência interna local da ferramenta para integrações numéricas necessárias ao fitting e à inferência, sem criar um novo caso de uso público e sem ampliar o escopo funcional do DCS.

## Algoritmos Estatísticos e Desempenho

### Matriz de rastreabilidade por caso de uso

| Caso de uso | Métodos da ferramenta | Algoritmos, estatísticas e políticas empregados |
| --- | --- | --- |
| UC01 - Carregar dataset de arquivo | `loadDataSet(filename)`, `DatasetLoader::loadFromFile` | Parsing por `strtod`; validação de finitude com `std::isfinite`; texto delimitado/espaços e binário de `double`. Linhas de metadados iniciadas por `#`, como as de `Record`, são ignoradas. Não há inferência estatística nesta etapa. |
| UC02 - Carregar dataset em memória | `loadDataSet(vector)`, `DatasetLoader::loadFromVector` | Varredura linear para validar valores finitos e cópia do vetor; em seguida executa a mesma pré-computação estatística do UC01. |
| UC03 - Resumo estatístico | `summary()` | Média, variância amostral, desvio padrão, mínimo e máximo por algoritmo online de Welford; variância dividida por `n - 1`; flag de negativos por comparação com o mínimo. |
| UC04 - Histograma numérico | `histogram(k)` | Classes de largura igual no intervalo `[min, max]`; contagem por índice aritmético; frequência relativa `f_i/n`. Com `k=0`, usa regra de Sturges: `ceil(1 + 3.322 log10(n))`. |
| UC05 - Boxplot numérico | `boxplot()` | Quartis e mediana por interpolação linear na amostra ordenada, com posição `p(n-1)`; IQR `Q3-Q1`; cercas de Tukey `Q1-1.5IQR` e `Q3+1.5IQR`; whiskers são os valores extremos dentro das cercas. |
| UC06 - Ajustar distribuições | `fitUniform`, `fitTriangular`, `fitNormal`, `fitExpo`, `fitErlang`, `fitBeta`, `fitWeibull` | Estimadores por estatísticas amostrais, método dos momentos e inversão numérica. Todos os candidatos são avaliados pelo SSE EDF/CDF com posições de Hazen. |
| UC07 - Ranking de ajustes | `fitAllSummary()` | Executa os sete ajustes, marca falhas e usa `std::stable_sort` por sucesso e SSE crescente. O ranking não é um teste formal de aderência. |
| UC08 - Intervalos de confiança | Métodos `*ConfidenceInterval` | IC t-Student para média; normal para proporções; correção de população finita; chi-square para variância; F para razão de variâncias; pooled t ou Welch-Satterthwaite para diferença de médias. |
| UC09 - Testes paramétricos | Métodos `testAverage`, `testProportion`, `testVariance` | Estatísticas t, z, chi-square e F; p-valor uni/bilateral pela CDF correspondente; teste z de duas proporções usa proporção combinada; teste de médias escolhe pooled/Welch pela compatibilidade do IC da razão de variâncias com 1. |
| UC10 - Testes de aderência | `chiSquareGoodnessOfFit`, `kolmogorovSmirnov` | Pearson chi-square com frequências esperadas por CDF, agrupamento sequencial de classes de baixa frequência esperada e `df=k-1-p`; KS de uma amostra com `D=max(D+,D-)` e série assintótica clássica para p-valor. |
| UC11 - Inferência baseada em arquivo | Overloads de IC, testes e KS | Reaplica os algoritmos de UC08 a UC10 após converter o arquivo em `DatasetLoader`; prioriza parser de resultados GenESyS para preservar apenas os valores observados. |
| UC12 - Exemplos executáveis | `analysis_tools_example`, `simulation_analysis_example` | Demonstra os algoritmos anteriores. O exemplo de simulação usa chegadas exponenciais `expo(5)` e gera medições normais `norm(50,9.83)` em um `Record`, que são então analisadas pela ferramenta. |
| UC13 - Sampler/experimenter | `sampler()`, `experimenter()` | Não implementado; não há algoritmo estatístico associado neste escopo. |
| UC14 - Novo/salvar dataset | `newDataSet()`, `saveDataSet()` | Não implementado; são ganchos de ciclo de vida/persistência, sem algoritmo estatístico associado. |

### Procedimentos numéricos transversais

| Necessidade | Procedimento implementado | Uso |
| --- | --- | --- |
| Quantil normal | Aproximação racional de Peter J. Acklam | Quantis z de ICs/testes e normal inversa pública. |
| CDF normal | Função erro complementar `erfc` | P-valores de testes z e CDF normal do fitting. |
| CDF chi-square, t e F no `HypothesisTester` | Integração composta de Simpson 1/3 da implementação preexistente, agora integrada como `SolverDefaultImpl`, com precisão `1e-6` e até 10.000 passos; chi-square possui atalhos fechados para `df=1` e `df=2`. | P-valores de testes de variância, média e razão de variâncias, além de chi-square. |
| CDF chi-square, t e F no módulo de quantis | Regra composta do ponto médio, com 8.192 subintervalos configurados em `TraitsAnalysis`. | CDFs usadas pela inversão de quantis. |
| Quantis chi-square, t e F | Expansão de intervalo seguida de bisseção; tolerâncias e limites de iteração centralizados em `TraitsAnalysis`. | Limites críticos e ICs. |
| Cache de quantis | `std::map` indexado pelos parâmetros e probabilidade. | Evita recalcular integrações/bisseções repetidas. |
| Função gamma | `std::tgamma`; função beta por relação `B(a,b)=Gamma(a)Gamma(b)/Gamma(a+b)`. | PDFs de beta, gamma, t, chi-square, F e escala Weibull. |

Os limites numéricos devem ser considerados ao interpretar resultados muito extremos de cauda. Os testes unitários com valores tabelados cobrem as regiões usuais de uso didático da ferramenta.

### Estatísticas descritivas

`DatasetLoader` carrega dados de texto delimitado, texto separado por espaços e arquivo binário de `double`.

Estatísticas básicas são calculadas em uma única passada usando acumulação incremental para média e variância amostral. A variância usa acumulação do tipo Welford, reduzindo perda numérica em relação a uma fórmula ingênua baseada em soma dos quadrados.

| Operação | Algoritmo | Complexidade |
| --- | --- | --- |
| Validação de dados | Varredura linear e checagem `std::isfinite` | O(n) |
| Média/variância/mínimo/máximo | Passada única com acumulação incremental | O(n) |
| Dados ordenados | `std::sort` | O(n log n) |
| `summary()` | Retorna valores pré-computados pelo loader | O(1) |
| `histogram(k)` | Cria `k` classes e varre a amostra contando frequências | O(n + k) |
| `histogram(0)` | Usa regra de Sturges | O(n + k) |
| `boxplot()` | Percentis por interpolação linear sobre amostra ordenada; outliers por regra 1.5 IQR | O(n) após ordenação |

O custo dominante do carregamento é a ordenação, necessária para percentis, boxplot, SSE por EDF/CDF e testes KS.

### Ajuste de distribuições

O critério de ranking usa erro quadrático entre CDF teórica e posição empírica de Hazen:

```text
SSE = sum_i (F(x_i) - p_i)^2
p_i = (i + 0.5) / n
```

Distribuições consideradas:

| Distribuição | Estimador/algoritmo implementado | Parâmetros produzidos |
| --- | --- | --- |
| Uniforme | Estimadores de extremos da amostra, equivalentes aos limites de suporte usados pelo MLE para uniforme contínua. | `a=min(x)`, `b=max(x)`. |
| Triangular | Método dos momentos com extremos amostrais. | `a=min(x)`, `b=max(x)`, `modo=3*mean-a-b`; a moda é limitada ao interior de `[a,b]` para manter CDF válida. |
| Normal | Estatísticas amostrais pré-computadas. | `mu=mean`, `sigma=stddev` com variância amostral de Welford. |
| Exponencial | MLE para a parametrização por escala/média. | `mean=mean(x)`; dados negativos ou média não positiva invalidam o ajuste. |
| Erlang | Método dos momentos. | `m=round(mean^2/variance)`, limitado a `m>=1`; `scale=mean/m`. A CDF usa a soma finita da Erlang inteira. |
| Beta escalada | Método dos momentos após normalizar `y=(x-min)/(max-min)` e limitar `y` a `(epsilon,1-epsilon)`. | `alpha=m((m(1-m)/v)-1)`, `beta=(1-m)((m(1-m)/v)-1)`, mais limites `min/max` originais. |
| Weibull | Casamento do coeficiente de variação com a expressão teórica, resolvido por bisseção. | Resolve `Gamma(1+2/k)/Gamma(1+1/k)^2 - 1 - CV^2 = 0` para a forma `k`; escala `lambda=mean/Gamma(1+1/k)`. |

Para beta escalada, a CDF é obtida por integração numérica de Simpson 1/3 da PDF beta normalizada, fornecida pelo solver preexistente reutilizado. Para Weibull, a CDF é fechada:

```text
F(x) = 1 - exp(-(x/lambda)^k), para x >= 0
```

`fitAllSummary()` executa todos os candidatos, descarta ajustes inválidos por `success=false`, ordena por SSE e retorna o melhor ajuste e o ranking completo.

Complexidade:

- cada avaliação de SSE custa O(n);
- `fitAllSummary()` usa número fixo de distribuições, então é O(n) após dataset carregado/ordenado;
- beta escalada e Weibull incluem passos numéricos adicionais: a CDF beta usa integração numérica e a forma da Weibull usa bisseção com limite fixo de iterações.

Como o número de distribuições e iterações é limitado por constantes, o comportamento é adequado para datasets de tamanho moderado em uso didático/simulação.

### Intervalos e testes paramétricos

`HypothesisTesterDefaultImpl` implementa inferência clássica:

| Método | Distribuição/estatística |
| --- | --- |
| IC de média | t-Student |
| IC de proporção | Normal aproximada |
| IC de proporção com população finita | Normal com correção finita |
| IC de variância | Chi-square |
| IC de diferença de médias | t pooled ou Welch |
| IC de diferença de proporções | Normal aproximada |
| IC de razão de variâncias | Fisher-Snedecor F |
| Teste de média | t-Student |
| Teste de proporção | Normal aproximada |
| Teste de variância | Chi-square |
| Testes de duas populações | t, normal ou F conforme parâmetro |

As estatísticas e erros-padrão usados são:

| Operação | Estatística/intervalo implementado |
| --- | --- |
| IC/teste de uma média | `t=(xbar-mu0)/(s/sqrt(n))`; IC `xbar +- t_(1-alpha/2,n-1)s/sqrt(n)`. |
| Planejamento amostral da média | `n=ceil((z_(1-alpha/2)s/e0)^2)`. |
| IC/teste de uma proporção | Aproximação normal: `z=(phat-p0)/sqrt(p0(1-p0)/n)` e IC `phat +- z_(1-alpha/2)sqrt(phat(1-phat)/n)`. |
| IC de proporção finita | Multiplica o erro-padrão por `sqrt((N-n)/(N-1))`. |
| IC/teste de uma variância | `chi2=(n-1)s^2/sigma0^2`; IC pelos quantis chi-square de `n-1` graus de liberdade. |
| Diferença de médias pooled | `t=(xbar1-xbar2)/sqrt(sp^2(1/n1+1/n2))`, com `sp^2=((n1-1)s1^2+(n2-1)s2^2)/(n1+n2-2)`. |
| Diferença de médias Welch | `t=(xbar1-xbar2)/sqrt(s1^2/n1+s2^2/n2)`; graus de liberdade pela fórmula de Welch-Satterthwaite. |
| Diferença de proporções | IC usa erros não combinados; teste usa proporção combinada `pbar=(x1+x2)/(n1+n2)`. |
| Razão/teste de variâncias | `F=s1^2/s2^2`, com graus de liberdade `n1-1` e `n2-1`. |

Nos testes paramétricos, hipóteses bilaterais usam `2*min(F_T(t), 1-F_T(t))` ou a CDF equivalente da estatística. Hipóteses unilaterais usam a cauda compatível com `LESS_THAN` ou `GREATER_THAN`.

Para diferença de médias, a política implementada é:

- usar t pooled quando a razão de variâncias indicar compatibilidade com 1;
- usar Welch quando as variâncias forem consideradas incompatíveis.

### Testes de aderência

O chi-square goodness-of-fit possui três formas principais:

- frequências observadas e esperadas diretamente;
- amostra crua, CDF e número automático de classes;
- amostra crua, CDF e limites de classes explícitos.

A partir de amostra crua:

1. as classes são definidas automaticamente ou por limites explícitos;
2. as frequências observadas são contadas;
3. as frequências esperadas são calculadas pela CDF informada;
4. classes adjacentes são agrupadas até atingir `minExpectedFrequency`;
5. os graus de liberdade são calculados por `df = effectiveClasses - 1 - estimatedParameters`.

A estatística de Pearson implementada é:

```text
X^2 = sum_i (O_i - E_i)^2 / E_i
```

Para dados crus, cada frequência esperada é calculada por:

```text
E_i = n * (F(b_i) - F(a_i)) / P(a_0 <= X <= b_k)
```

Essa normalização preserva a comparação mesmo quando os limites explícitos de classe não cobrem toda a cauda teórica. O agrupamento é sequencial: acumula classes adjacentes até atingir `minExpectedFrequency`; se a última classe ainda ficar abaixo do limiar, ela é fundida à anterior.

O KS ordena a amostra e calcula a maior diferença entre a CDF empírica e a CDF teórica informada:

```text
D+ = max_i (i/n - F(x_(i)))
D- = max_i (F(x_(i)) - (i-1)/n)
D  = max(D+, D-)
```

O limite crítico usado é a aproximação assintótica `sqrt(-0.5*ln(alpha/2)/n)`. O p-valor usa a série alternada clássica com correção finita `lambda=(sqrt(n)+0.12+0.11/sqrt(n))*D`, truncada quando o termo fica menor que `1e-12` ou após 100 termos.

Complexidade:

- chi-square por dados crus: O(n + k), onde `n` é o tamanho da amostra e `k` é o número de classes iniciais;
- KS: O(n log n) se a amostra ainda não estiver ordenada e O(n) para varrer e calcular a estatística.

Observação importante: o p-valor implementado é a aproximação clássica do KS para CDF completamente especificada. Quando os parâmetros da distribuição são estimados da própria amostra, o p-valor deve ser tratado como diagnóstico, pois não há correção de Lilliefors, bootstrap ou Monte Carlo.

### Probabilidade e quantis

`ProbabilityDistributionBase` fornece densidades/massas para distribuições conhecidas. `ProbabilityDistribution` fornece inversas/quantis usados em ICs e testes.

| Família | Função implementada | Base matemática |
| --- | --- | --- |
| Beta | `beta` | PDF beta por funções gamma. |
| Chi-square | `chi2` | PDF chi-square por potência, exponencial e gamma. |
| Erlang/Gamma | `erlang`, `gamma` | PDFs gamma com forma e escala; Erlang é o caso de forma inteira. |
| Exponencial | `exponential` | PDF exponencial parametrizada pela taxa usada pelo helper legado. |
| Fisher-Snedecor | `fisherSnedecor` | PDF F por função beta. |
| Normal/Lognormal | `normal`, `logNormal` | PDFs fechadas com exponencial quadrática. |
| Poisson | `poisson` | PMF `mean^x exp(-mean)/x!`. |
| Triangular/Uniforme | `triangular`, `uniform` | PDFs por partes. |
| t-Student | `tStudent` | PDF por gamma e graus de liberdade. |
| Weibull | `weibull` | PDF de forma/escala. |

Essas funções são usadas tanto diretamente pelo exemplo quanto indiretamente pelas integrações de CDF, cálculo de quantis e avaliação de ajustes.

## Comparação com a Apostila de Estatística

Esta seção compara os métodos estatísticos implementados com os procedimentos apresentados na apostila de Estatística usada como referência. A apostila cobre diretamente distribuições contínuas básicas, intervalos de confiança e testes de hipóteses clássicos. A ferramenta segue a mesma base teórica nos métodos de inferência e usa extensões práticas no fitting, pois a apostila apresenta as distribuições, mas não define um procedimento completo de ajuste automático e ranking de distribuições.

### Distribuições do fitter

| Método implementado | Relação com a apostila | Diferenças práticas da implementação |
| --- | --- | --- |
| `fitUniform` | Segue a distribuição uniforme contínua apresentada na apostila, com CDF linear no intervalo `[a,b]`, média `(a+b)/2` e variância `(b-a)^2/12`. | A apostila define a distribuição e suas propriedades. A ferramenta estima `a` e `b` diretamente pelos extremos amostrais (`min` e `max`) e avalia o ajuste por SSE entre CDF teórica e EDF. A apostila não propõe esse critério de ajuste. |
| `fitExpo` | Segue a parametrização da apostila em que o parâmetro da exponencial representa a média/escala `lambda`, com `E(X)=lambda`, `Var(X)=lambda^2` e `F(x)=1-exp(-x/lambda)`. | A ferramenta estima `lambda` pela média amostral, equivalente ao estimador natural/MLE nessa parametrização. Também rejeita dados negativos, pois o suporte da exponencial é `x >= 0`. |
| `fitNormal` | Segue a distribuição normal da apostila, com média `mu`, variância `sigma^2` e padronização por `Z=(X-mu)/sigma`. | A ferramenta estima `mu` pela média amostral e `sigma` pelo desvio padrão amostral calculado a partir da variância com divisor `n-1`. A CDF é calculada por `erfc`, em vez de consulta a tabela normal. |
| `fitTriangular` | Não há seção específica de distribuição triangular na apostila. | É uma extensão da ferramenta. Usa extremos amostrais como limites e estima a moda por método dos momentos: `mode = 3*mean - min - max`, limitada ao suporte para manter a CDF válida. |
| `fitErlang` | A apostila cobre distribuições discretas e contínuas básicas, mas não apresenta ajuste Erlang como método de fitting. | É uma extensão da ferramenta. Usa método dos momentos: `m ~= mean^2/variance`, arredondado para inteiro positivo, e escala `mean/m`. A CDF é calculada pela soma finita da Erlang inteira. |
| `fitBeta` | A apostila usada como referência não apresenta ajuste de beta escalada como procedimento de fitting. | É uma extensão da ferramenta. Normaliza a amostra para `[0,1]`, estima `alpha` e `beta` por método dos momentos e calcula a CDF por integração numérica da PDF beta. |
| `fitWeibull` | A apostila não apresenta Weibull como distribuição de ajuste. | É uma extensão da ferramenta. Estima o parâmetro de forma pelo coeficiente de variação, resolvido por bisseção, e calcula a escala por `lambda = mean/Gamma(1+1/k)`. |
| `fitAllSummary` | A apostila não define ranking automático de distribuições. | A ferramenta executa todos os ajustes, calcula SSE usando posições de Hazen e ordena os candidatos. Esse ranking é uma heurística de aderência, não um teste formal. |

Em resumo, para uniforme, exponencial e normal, a ferramenta segue a mesma formulação probabilística da apostila. A diferença principal é que a apostila ensina a distribuição e seus cálculos de probabilidade, enquanto a ferramenta também estima parâmetros e mede a qualidade do ajuste. Triangular, Erlang, beta e Weibull são funcionalidades adicionais ao material da apostila.

### Intervalos de confiança

| Método implementado | Relação com a apostila | Diferenças práticas da implementação |
| --- | --- | --- |
| `averageConfidenceInterval` | Segue o IC para média com variância populacional desconhecida: `xbar +- t_(alpha/2,n-1) * s/sqrt(n)`. | A ferramenta usa sempre t-Student para esse método, assumindo que o desvio padrão informado é amostral. A apostila também apresenta o caso com variância conhecida usando normal padrão, que não foi exposto como overload separado. |
| `estimateSampleSize` | Segue a fórmula de tamanho amostral para estimar média com erro máximo `E`: `n = (z*sigma/E)^2`. | A ferramenta usa aproximação normal e arredonda para cima. O parâmetro `avg` é mantido por compatibilidade, mas não influencia o cálculo, assim como na fórmula teórica. |
| `proportionConfidenceInterval` | Segue a abordagem otimista da apostila para proporção: `phat +- z * sqrt(phat(1-phat)/n)`. | A apostila também apresenta a abordagem conservadora usando `1/(4n)`. A ferramenta implementa a abordagem otimista, não a conservadora. |
| `proportionConfidenceInterval` com população finita | Segue a lógica da correção para população finita apresentada na apostila para estimação por amostragem sem reposição. | A ferramenta aplica essa correção ao IC de proporção quando `N` é informado. |
| `varianceConfidenceInterval` | Segue o IC para variância baseado em qui-quadrado: limites com `(n-1)S^2/chi-square`. | A implementação calcula os quantis numericamente em vez de usar tabela. A orientação dos limites segue a forma clássica com caudas `alpha/2` e `1-alpha/2`. |
| `averageDifferenceConfidenceInterval` | Segue os dois casos da apostila para diferença de médias com variâncias desconhecidas: t pooled quando as variâncias são assumidas iguais e Welch quando são diferentes. | A apostila separa os casos por hipótese assumida. A ferramenta escolhe automaticamente: usa pooled quando o IC da razão de variâncias contém 1; caso contrário usa Welch-Satterthwaite. |
| `proportionDifferenceConfidenceInterval` | Está na mesma linha da aproximação normal para proporções e da diferença de proporções usada nos testes da apostila. | A apostila enfatiza o teste de igualdade de duas proporções. A ferramenta também expõe IC para `p1-p2`, usando erro padrão não combinado. |
| `varianceRatioConfidenceInterval` | Segue a inferência por distribuição F-Snedecor para razão de variâncias apresentada na apostila. | A implementação retorna intervalo para `sigma1^2/sigma2^2`, coerente com o exemplo da apostila. Os quantis F são calculados numericamente, não consultados em tabela. |

### Testes de hipóteses

| Método implementado | Relação com a apostila | Diferenças práticas da implementação |
| --- | --- | --- |
| `testAverage` para uma população | Segue o procedimento geral da apostila: definir H0/H1, calcular estatística, comparar com região crítica e calcular nível descritivo. Para variância desconhecida, usa `T=(xbar-mu0)/(s/sqrt(n))`. | A ferramenta usa t-Student para o método principal, pois recebe desvio padrão amostral. A apostila também mostra o caso normal `Z` quando a variância populacional é conhecida. |
| `testProportion` para uma população | Segue o teste para proporção populacional em amostras grandes: `Z=(phat-p0)/sqrt(p0(1-p0)/n)`. | A ferramenta calcula p-valor para alternativas unilateral esquerda, unilateral direita e bilateral. A apostila apresenta a mesma lógica por região crítica e nível descritivo. |
| `testVariance` para uma população | Segue o teste qui-quadrado para variância: `W=(n-1)S^2/sigma0^2`. | A implementação calcula limites críticos e p-valor por CDF/quantis numéricos. A apostila usa tabela de qui-quadrado. |
| `testAverage` para duas populações | Segue a apostila para diferença de médias independentes: t pooled para variâncias iguais e Welch para variâncias diferentes. | A ferramenta automatiza a escolha pooled/Welch usando a compatibilidade da razão de variâncias com 1. A apostila apresenta primeiro a decisão sobre homogeneidade e depois escolhe o teste. |
| `testProportion` para duas populações | Segue o teste de igualdade de duas proporções para amostras grandes, com proporção combinada `pbar=(x+y)/(n+m)` sob H0. | A implementação aceita alternativas unilateral e bilateral e retorna p-valor. A apostila apresenta a mesma estatística normal e exemplifica a decisão por região crítica. |
| `testVariance` para duas populações | Segue o teste F para igualdade de variâncias: `F=S1^2/S2^2`. | A implementação calcula p-valor e limites críticos com a CDF F numérica, enquanto a apostila trabalha com valores tabelados. |
| `chiSquareGoodnessOfFit` | A apostila apresenta a distribuição qui-quadrado e menciona testes de aderência em contexto diagnóstico, mas não desenvolve um procedimento completo de goodness-of-fit para fitting de distribuições. | É uma extensão da ferramenta. Implementa Pearson chi-square com frequências esperadas por CDF, agrupamento de classes com baixa frequência esperada e graus de liberdade `classes efetivas - 1 - parâmetros estimados`. |
| `kolmogorovSmirnov` | A apostila menciona testes de normalidade/aderência como instrumentos possíveis, mas não detalha o algoritmo KS. | É uma extensão da ferramenta. Implementa KS de uma amostra por `D=max(D+,D-)` e p-valor assintótico. Quando parâmetros são estimados da própria amostra, o p-valor é documentado como diagnóstico. |

De forma geral, os métodos de inferência seguem a mesma linha de raciocínio da apostila: estatística de teste padronizada, distribuição de referência, região crítica, nível de significância e p-valor. A diferença principal é operacional: a apostila usa tabelas estatísticas e resolução manual, enquanto a ferramenta calcula CDFs, p-valores e quantis numericamente.

## Makefile de Conveniência

O `Makefile` foi criado como camada de conveniência para terminal. Ele não substitui CMake/QtCreator; apenas encapsula comandos frequentes.

| Alvo | Uso |
| --- | --- |
| `gui` | Configura e compila a GUI. |
| `run-gui` | Compila e executa a GUI. |
| `terminal` | Compila a aplicação terminal. |
| `run-terminal` | Compila e executa a aplicação terminal. |
| `unit-tests` | Compila testes unitários. |
| `run-unit-tests` | Compila e executa testes unitários. |
| `run-unit-tests PACKAGE=tools` | Executa somente testes unitários do pacote tools. |
| `integration-tests` | Compila testes de integração. |
| `run-integration-tests` | Executa testes de integração. |
| `run-integration-tests PACKAGE=tools` | Executa somente testes de integração da ferramenta de análise. |
| `examples` | Compila exemplos. |
| `run-examples` | Executa os exemplos de análise. |
| `clean` | Remove builds gerados pelos alvos do Makefile. |

## Exemplos Executáveis

### Exemplo analítico

```text
examples/analysis_tools_example.cpp
```

Datasets usados:

```text
examples/data/sample_data.csv
examples/data/sample_group_a.csv
examples/data/sample_group_b.csv
```

O exemplo cobre:

- carregamento de arquivo;
- carregamento em memória;
- summary;
- histograma;
- boxplot;
- fitting individual;
- ranking completo por `fitAllSummary()`;
- helpers de probabilidade;
- ICs de uma população;
- testes de uma população;
- ICs e testes de duas populações;
- overloads baseados em arquivo;
- chi-square;
- KS;
- verificações determinísticas de regressão.

### Exemplo com modelagem, simulação e análise

```text
examples/simulation_analysis_example.cpp
```

Esse exemplo demonstra o uso da ferramenta de análise a partir de um resultado gerado pelo próprio GenESyS. O modelo representa um processo leve de amostragem de inspeção:

- entidades do tipo `Part` são criadas por um componente `Create`;
- cada chegada passa por um componente `Record`;
- o `Record` grava uma medição simulada de inspeção em arquivo de resultado do GenESyS;
- a entidade é enviada para `Dispose`;
- o arquivo produzido é lido por `SimulationResultsParser`;
- os valores observados são carregados em memória em `DataAnalyserDefaultImpl`;
- a fachada calcula resumo descritivo, fitting, IC da média, teste da média e diagnóstico KS.

O objetivo do exemplo é comprovar que a ferramenta de análise pode ser usada tanto com datasets externos quanto com saídas produzidas por um modelo de simulação. A dependência com o kernel e os plugins aparece apenas no executável de exemplo, isto é, como consumidor da ferramenta. O pacote `tools/analysis` continua independente de `source/kernel`.

## Documentação Produzida ou Atualizada

| Documento | Objetivo |
| --- | --- |
| `source/tools/analysis/README.md` | Documentação principal da ferramenta de análise: API, algoritmos, limitações, diagramas e testes. |
| `source/tools/README_tools.md` | Visão geral do pacote `source/tools` e destaque para `genesys_tools_analysis`. |
| `source/tools/ARCHITECTURE_tools.md` | Fronteiras arquiteturais, direção de dependências e contratos estáveis. |
| `source/tests/README.md` | Organização, objetivos e resultados dos testes unitários, integração e exemplo. |
| `documentation/developersCommunication/...` | Notas históricas atualizadas para apontar o backend consolidado atual. |
| `documentation/DoxyfileDeveloper2026` | Atualizado para incluir `source/tools` na documentação developer. |
| `documentation/tools/DCS_data_analysis_tool_report.md` | Este relatório de entrega. |

## Limitações Conhecidas

- `newDataSet`, `saveDataSet`, `sampler` e `experimenter` permanecem fora do escopo atual.
- `isNormalDistributed(...)` é uma heurística por SSE EDF/CDF, não um teste formal de normalidade.
- P-valores do KS são diagnósticos quando parâmetros são estimados da própria amostra.
- O fitting usa estimativas pragmáticas adequadas ao escopo didático/simulador, não um pacote estatístico completo de máxima verossimilhança para todas as famílias.
- A GUI Data Analyzer ainda precisa ser conectada completamente ao backend consolidado para substituir cálculos aproximados/protótipos antigos.

# Validação

## Estratégia de Validação

A validação foi organizada para cobrir os critérios do DCS relacionados à demonstração de requisitos, qualidade dos testes e comprovação de funcionamento do componente. Foram usados testes unitários, testes de integração e exemplos executáveis.

Os testes unitários verificam métodos e classes específicas. Os testes de integração exercitam o fluxo completo pela fachada pública. Os exemplos demonstram o uso esperado por um usuário C++ e também a integração da análise com um modelo de simulação do GenESyS.

## Testes Unitários

Local:

```text
source/tests/unit/tools/analysis
```

Arquivos:

| Arquivo | Objetivo |
| --- | --- |
| `test_tools_dataanalyser.cpp` | Testar fachada, defaults, propagação do dataset, summary, histogram, boxplot e caminhos fora de escopo. |
| `test_tools_dataset_loader.cpp` | Testar carregamento texto/binário, estatísticas, entradas inválidas e estado interno. |
| `test_tools_simulation_results_dataset.cpp` | Testar parser de raw numeric, record legacy/enriched e formato GUI tabular. |
| `test_fitter_distributions.cpp` | Testar ajuste das distribuições, falhas esperadas, ranking e `fitAllSummary()`. |
| `test_tools_hypothesistester.cpp` | Testar ICs, testes paramétricos, chi-square, KS, overloads de arquivo e valores de referência. |

Resultado registrado:

```text
Data: 2026-06-23
Atalho do Makefile: make run-unit-tests PACKAGE=tools
Resultado: 96/96 testes aprovados em 2026-06-23
```

## Testes de Integração

Local:

```text
source/tests/integration/tools/analysis
```

Arquivo:

```text
test_analysis_tool_integration.cpp
```

Os testes de integração validam:

- fluxo completo pela fachada: carrega `sample_data.csv` e valida integração entre resumo estatístico, histograma, boxplot, ajuste de distribuições, intervalo/teste de média e testes de aderência chi-square e KS;
- consistência entre entradas e inferência por arquivo: verifica que o mesmo dataset carregado de arquivo ou memória produz estatísticas e ajuste equivalentes, e que overloads de testes por arquivo retornam decisões coerentes para média, variância e proporção entre amostras.

Resultado registrado:

```text
Data: 2026-06-23
Atalho do Makefile: make run-integration-tests PACKAGE=tools
Resultado: 2/2 testes aprovados
```

## Relação entre Testes e Requisitos

| Requisito / caso de uso | Evidência principal |
| --- | --- |
| Carregar dataset de arquivo | `DatasetLoaderTest`, `DataAnalyserDefaultImplTest`, teste de integração |
| Carregar dataset em memória | `DataAnalyserDefaultImplTest`, teste de integração |
| Summary | `DataAnalyserDefaultImplTest`, teste de integração |
| Histogram | `DataAnalyserDefaultImplTest`, teste de integração |
| Boxplot | `DataAnalyserDefaultImplTest`, teste de integração |
| Fitting | `FitterTest`, exemplo, teste de integração |
| Ranking de fitting | `FitterTest.FitAllSummary...`, exemplo, teste de integração |
| ICs | `HypothesisTesterDefaultImplTest`, exemplo, teste de integração |
| Testes paramétricos | `HypothesisTesterDefaultImplTest`, exemplo, teste de integração |
| Chi-square | `HypothesisTesterDefaultImplTest`, exemplo, teste de integração |
| KS | `HypothesisTesterDefaultImplTest`, exemplo, teste de integração |
| Overloads baseados em arquivo | `HypothesisTesterDefaultImplTest.FileBased...`, teste de integração |
| Independência do kernel | Alvo `genesys_tools_analysis` e diagrama de fronteiras |

## Evidências de Funcionamento dos Exemplos

Resultado do exemplo analítico:

```text
Data: 2026-06-23
Atalho do Makefile: make run-examples
Resultado: Regression result: ALL CHECKS PASSED
```

Resultado do exemplo com modelagem, simulação e análise:

```text
Data: 2026-06-23
Atalho do Makefile: make run-examples
Resultado: Simulation analysis example: SUCCESS
```

Esses resultados demonstram que o componente pode ser usado tanto sobre datasets externos quanto sobre resultados gerados por um modelo de simulação do próprio GenESyS.

# Conclusão

A entrega consolida a ferramenta de análise de dados como componente C++ vinculado ao GenESyS. A solução atende ao escopo de uma funcionalidade típica de simulador de sistemas, com separação arquitetural adequada, testes unitários, testes de integração, exemplos executáveis, documentação e diagramas.

Os principais resultados alcançados foram:

- criação de uma ferramenta de análise independente do kernel;
- implementação de uma fachada de uso direto;
- suporte a entrada por arquivo e por memória;
- estatísticas descritivas, histograma e boxplot numéricos;
- ajuste e ranking de distribuições;
- inferência paramétrica e testes de aderência;
- integração com exemplos e fluxo de simulação;
- validação por testes unitários e de integração;
- documentação técnica consolidada para uso, arquitetura, testes e entrega.

Do ponto de vista dos critérios do DCS, os principais artefatos estão presentes: código-fonte organizado e comentado, casos de uso cobertos por testes, documentação de arquitetura, instruções de uso, evidências de execução e relatório estruturado.

Como próximos passos, recomenda-se conectar completamente a GUI Data Analyzer ao backend consolidado, implementar os pontos de extensão `sampler` e `experimenter` quando entrarem no escopo da ferramenta, e avaliar correções específicas para p-values de KS quando os parâmetros da distribuição forem estimados da própria amostra.

# Referências

BANKS, Jerry; CARSON, John S.; NELSON, Barry L.; NICOL, David M. **Discrete-Event System Simulation**. 5. ed. Upper Saddle River: Pearson, 2010.

CANCHO, Vicente Garibay. **Noções de Estatística e Probabilidade**. Ouro Preto: Universidade Federal de Ouro Preto, 2004.

CANCIAN, Rafael Luiz. **GenESyS: Generic and Expansible System Simulator**. GitHub, [s. d.]. Disponível em: <https://github.com/rlcancian/Genesys-Simulator>. Acesso em: 23 jun. 2026.

GOOGLE. **GoogleTest: Google Testing and Mocking Framework**. GitHub, [s. d.]. Disponível em: <https://github.com/google/googletest>. Acesso em: 23 jun. 2026.

LAW, Averill M. **Simulation Modeling and Analysis**. 5. ed. New York: McGraw-Hill Education, 2015.

MONTGOMERY, Douglas C.; RUNGER, George C. **Applied Statistics and Probability for Engineers**. 7. ed. Hoboken: Wiley, 2020.

NIST/SEMATECH. **e-Handbook of Statistical Methods**. Gaithersburg: National Institute of Standards and Technology, [s. d.]. Disponível em: <https://www.itl.nist.gov/div898/handbook/>. Acesso em: 23 jun. 2026.
