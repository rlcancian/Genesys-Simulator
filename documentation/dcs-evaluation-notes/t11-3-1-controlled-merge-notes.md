# DCS Tema 11.3.1 — Notas de avaliação controlada

Estas notas registram decisões do professor/avaliador para a avaliação final do DCS Tema 11.3.1 — Ferramenta de Análise de Dados, com foco em diagnósticos adicionais e ajuste de curvas.

## PR original

- PR original dos alunos: #433 — `Grupo 11.3.1 - Gabriel Salmoria, Diego M. Meditsch e Lucas S. Vieira`
- Base: `2026-1`
- Head: branch `2026-1` do fork `gabriel-salmoria/Genesys-Simulator`

## Decisões de aceitação parcial

O PR original **não deve ser mergeado integralmente como está**.

### Entrega formal

O código-fonte não foi anexado ao Moodle. A justificativa de limite de 100 MB não é aceita, pois a entrega permitia múltiplos arquivos e o código poderia ter sido entregue separado de vídeo, artefatos de build e repositório completo.

### Alterações não aceitas

1. Arquivos gerados de build em `build-demo/` não serão aceitos.
2. Executáveis/binários gerados não serão aceitos.
3. A remoção de `todo.txt` não é aceita, pois o arquivo está fora do escopo funcional do Tema 11.3.1.

### Critério técnico para avaliação

A avaliação deve distinguir o que já existia no GenESyS — datasets, interfaces de fitting, `HypothesisTester`, infraestrutura probabilística e classes estatísticas — do que foi efetivamente criado pelo grupo: a fachada `DataAnalyzer`, diagnósticos adicionais, estruturas numéricas, testes adicionais, análise temporal e integração em CMake/testes.

Esta nota não representa aprovação para merge integral do PR original.