# Análise técnica do parser (Bison/Flex)

## Audit Status (WiP20261)
- Branch audited: `WiP20261`
- Audit scope: reauditoria do lexer/gramática e ações semânticas em `source/parser/parserBisonFlex`
- Status legend: `DONE`, `PARTIAL`, `OPEN`, `UNCERTAIN`, `SUPERSEDED`
- Data desta reauditoria documental: `2026-04-10`

## Escopo
Arquivos revalidados:
- `source/parser/parserBisonFlex/bisonparser.yy`
- `source/parser/parserBisonFlex/lexerparser.ll`
- `source/parser/parserBisonFlex/bisonparser.report` (não encontrado no checkout atual)

## Registro histórico (pré-reauditoria)
O texto original deste documento apontava conflitos LR elevados, erros de tokenização e lacunas semânticas. Parte desse diagnóstico foi superada por mudanças posteriores; parte ainda permanece relevante.

## Reclassificação dos achados históricos

### Conversão hexadecimal
### Audit status
`DONE`

### Evidence
- O lexer converte hex com `std::strtoll(yytext, &endPtr, 16)` e emite `NUMH`.

### Remaining gaps
- Sem gap direto neste item.

### Tokenização de `min` / `max`
### Audit status
`DONE`

### Evidence
- `min` retorna `mathMIN` e `max` retorna `mathMAX`.

### Remaining gaps
- Sem gap direto neste item.

### Regex de `EntitiesWIP`
### Audit status
`DONE`

### Evidence
- Regra atual usa padrão de palavra completa `[eE][nN][tT][iI][tT][iI][eE][sS][wW][iI][pP]`.

### Remaining gaps
- Sem gap direto neste item.

### Regra isolada `T` no lexer
### Audit status
`SUPERSEDED`

### Evidence
- A regra isolada não aparece mais no lexer atual.

### Remaining gaps
- Sem gap direto neste item.

### Token `ILLEGAL` antes inútil
### Audit status
`DONE`

### Evidence
- A gramática atual inclui `expression: ... | illegal { $$.valor = -1; }`, tornando o caminho alcançável.

### Remaining gaps
- Avaliar qualidade de recuperação/diagnóstico de erro além da alcançabilidade.

### Estrutura de precedência/associatividade da gramática
### Audit status
`DONE`

### Evidence
- Estrutura por níveis explícitos: `logicalOr -> logicalXor -> logicalAnd -> logicalNot -> relational -> additive -> multiplicative -> power -> unary -> primary`.
- Potência em `power: unary POWER power`, indicando associatividade à direita.

### Remaining gaps
- Não foi possível revalidar numericamente conflitos LR no estado atual sem `bisonparser.report`.

## Pontos ainda pendentes (ou parcialmente mitigados)

### Funções semânticas não implementadas totalmente (`fDISC`, `fLASTINQ`, `fRESSEIZES`)
### Audit status
`OPEN`

### Evidence
- `fDISC` em `probFunction` contém TODO e retorno placeholder (`sampleDiscrete(0,0)`).
- `fLASTINQ` possui ação vazia com comentário de não implementado.
- `fRESSEIZES` também está com comentário de não implementado.

### Remaining gaps
- Implementar semântica real dessas funções e cobrir com testes de expressão.

### Avaliação de `FORM`
### Audit status
`OPEN`

### Evidence
- Regras de `FORM` continuam comentando problema sério de reentrada e retornam `0.0`.

### Remaining gaps
- Resolver avaliação de fórmula (reentrância/arquitetura de avaliação) para evitar retorno silencioso inválido.

### Risco de null-deref em atribuições/leituras ligadas a evento atual
### Audit status
`PARTIAL`

### Evidence
- Há guardas em alguns caminhos (`if (getCurrentEvent() != nullptr && getCurrentEvent()->getEntity() != nullptr)`), mas ainda existem acessos diretos como `fIDENT` e outras ações usando `getCurrentEvent()->getEntity()` sem guarda uniforme.

### Remaining gaps
- Padronizar guarda robusta em todos os pontos de acesso a evento/entidade corrente.

### Conflitos LR atuais
### Audit status
`UNCERTAIN`

### Evidence
- `source/parser/parserBisonFlex/bisonparser.report` não estava disponível no checkout atual, então não houve recontagem atual de conflitos shift/reduce e reduce/reduce.

### Remaining gaps
- Gerar novo report do Bison e atualizar números concretos de conflitos.

## Recomendações arquiteturais que seguem válidas
1. Formalizar tratamento de funções não implementadas com erro explícito (em vez de `0`/placeholder silencioso).
2. Consolidar estratégia de null-safety para contexto de evento corrente.
3. Reintroduzir rotina de auditoria de conflitos LR no pipeline (geração e versionamento de report ou artefato equivalente).
4. Fortalecer testes de regressão de parser para operadores, funções de plugin e cenários de erro.

## Remaining Work
- `OPEN` — Implementar semântica real de `fDISC`.
- `OPEN` — Implementar semântica real de `fLASTINQ`.
- `OPEN` — Implementar semântica real de `fRESSEIZES`.
- `OPEN` — Resolver avaliação de `FORM` sem retorno neutro silencioso.
- `PARTIAL` — Eliminar caminhos restantes de null-deref em `getCurrentEvent()->getEntity()`.
- `UNCERTAIN` — Recontar conflitos LR com novo `bisonparser.report`.
