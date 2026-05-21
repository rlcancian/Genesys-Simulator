# tools/analysis

Módulo de análise estatística de dados do Genesys Simulator. Fornece as implementações concretas para ajuste de distribuições de probabilidade, carregamento de datasets e testes de hipóteses.

## Estrutura

| Arquivo | Descrição |
|---|---|
| `DatasetLoader` | Carrega dados numéricos de arquivos texto (delimitado ou espaços) e binários. Calcula estatísticas básicas (média, variância, min, max) via algoritmo de Welford. |
| `Fitter_if` / `FitterDefaultImpl` | Interface e implementação do ajustador de distribuições. Suporta Uniforme, Triangular, Normal, Exponencial, Erlang, Beta e Weibull. |
| `FitterDummyImpl` | Implementação vazia da interface `Fitter_if`, útil para injeção em contextos que não requerem ajuste real. |
| `DataAnalyser_if` / `DataAnalyserDefaultImpl` | Fachada de alto nível que orquestra o ciclo de vida do dataset e delega o ajuste ao `Fitter_if` injetado. |
| `HypothesisTester_if` / `HypothesisTesterDefaultImpl1` | Interface e implementação de testes de hipóteses (médias, proporções, variâncias). |
| `TraitsTools` | Template de traits que associa interfaces às suas implementações padrão. |
| `SimulationResultsDataset` / `SimulationResultsParser` | Estruturas e parser para datasets gerados por simulação. |

## Estimativa de parâmetros

Todos os métodos de ajuste utilizam o **Método dos Momentos (MOM)**, calculando os parâmetros diretamente das estatísticas da amostra (média, variância, extremos) sem agrupamento em classes.

| Distribuição | Parâmetros estimados |
|---|---|
| Uniforme | min = mínimo da amostra, max = máximo da amostra |
| Triangular | min, max (extremos da amostra), moda = 3μ − min − max |
| Normal | média amostral μ, desvio padrão amostral σ (denominador n−1) |
| Exponencial | média amostral μ |
| Erlang | média amostral μ, número de fases m = round(μ²/σ²) |
| Beta | α, β (MOM sobre dados escalados para [0,1]), limite inferior, limite superior |
| Weibull | forma α (via coeficiente de variação CV = σ/μ), escala = μ / Γ(1 + 1/α) |

## Erro Quadrático

O erro quadrático é calculado pela estatística de **Cramér-von Mises**, que compara a CDF teórica com a CDF empírica ponto a ponto sobre os dados ordenados:

```
SE = Σᵢ (F(xᵢ) − pᵢ)²
```

onde `F(xᵢ)` é a CDF teórica avaliada no i-ésimo valor ordenado e `pᵢ = (i + 0.5) / n` é a posição empírica de Hazen. A soma percorre todos os `n` pontos da amostra.

O `+ 0.5` na posição de Hazen evita os extremos 0 e 1, que causariam problemas numéricos em distribuições com suporte infinito como a Normal.

## Observações

### Diferenças em relação ao Arena Input Analyzer

- **Desvio padrão:** o Arena exibe o valor calculado com denominador `n` (estimador MLE), enquanto este módulo utiliza denominador `n − 1` (estimador amostral não-viesado). Para amostras grandes a diferença é desprezível; para amostras pequenas (ex.: n = 40) pode haver divergência visível (ex.: 9.95 vs. 9.83).

- **Arredondamentos:** o Arena Input Analyzer realiza arredondamentos intermediários na exibição dos resultados — média, desvio padrão e parâmetros ajustados são apresentados com precisão reduzida. Isso faz com que valores como média e desvio padrão pareçam diferentes dos obtidos pelo fitter do Genesys, mesmo que a diferença real seja numericamente insignificante.

- **Fórmula do erro quadrático:** este módulo utiliza o critério de Cramér-von Mises (comparação ponto a ponto na CDF empírica), enquanto o Arena Input Analyzer utiliza `χ²/n` por histograma de classes. As duas métricas não são numericamente comparáveis, o que explica a diferença nos valores de erro quadrático exibidos.
