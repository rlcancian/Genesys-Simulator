# Tema 8.1 - EDOs (2026-06-30)

> Este documento é um resumo técnico de apoio (evidências de build/teste) para o relatório
> completo do trabalho, que é entregue separadamente no Moodle.

## Objetivo do trabalho

Adicionar suporte a sistemas de tempo contínuo (equações diferenciais ordinárias) ao GenESyS
através de um novo componente de modelo (`ContinuousSystemComponent`) e uma nova definição de
dados (`ODESolver`), além de corrigir um bug numérico no integrador RK4 já existente do
componente `LSODE`.

## O que foi implementado/alterado

### Novo — componente de sistema contínuo
- [`source/plugins/components/Continuous/ContinuousSystemComponent.h`](../source/plugins/components/Continuous/ContinuousSystemComponent.h) / `.cpp`
  — `ModelComponent` que dispara a integração numérica de um `ODESolver` a cada chegada de
  entidade na porta de entrada 0, escreve o estado resolvido de volta no `ODESolver` e encaminha
  a entidade pela porta de saída 0.

### Novo — definição de dados do solver
- [`source/plugins/data/Continuous/ODESolver.h`](../source/plugins/data/Continuous/ODESolver.h) / `.cpp`
  — `ModelDataDefinition` que encapsula estado (`_stateValues`), equações (`_equationExpressions`
  em função de nomes de variáveis de estado), passo, precisão, `maxSteps` e delega a integração ao
  `RungeKutta4OdeSolver` (`source/tools/`).

### Corrigido — `LSODE::_doStep()`
- [`source/plugins/components/Continuous/LSODE.cpp`](../source/plugins/components/Continuous/LSODE.cpp)
  — o RK4 manual (baseado em expressões simbólicas via `Model::parseExpression`) tinha dois bugs:
  1. **k4 usava meio-passo em vez do passo completo** (`valVar[i] + k3[i] * halfStep` em vez de
     `_step`), quebrando a ordem de convergência do método.
  2. **O resultado final somava a partir do valor *atual* da variável** (já mutado durante as
     avaliações de k2/k3), em vez do estado em t0 (`valVar`) — erro acumulado a cada passo.

### Ajustado — `DiffEquations`
- [`source/plugins/components/Continuous/DiffEquations.h`](../source/plugins/components/Continuous/DiffEquations.h) / `.cpp`
  — assinatura do callback de biblioteca compartilhada trocada de
  `onDispatchEvent_t(Simulator*, Model*, Entity*)` para
  `diffEquationsOnDispatchEvent_t(double* stateVars, int n, double t0, double tTarget, double step)`,
  e adicionado suporte a nomes de variáveis de estado (`getStateVariableNames()`), desacoplando o
  plugin de código externo do restante do simulador.

### Integração com o GenESyS (registro de plugins)
- [`source/kernel/simulator/PluginManager.h`](../source/kernel/simulator/PluginManager.h) / `.cpp`
  — novo método `insertStaticPlugin(Plugin*)` para inserir plugins estáticos diretamente
  (usado pelos testes de integração, sem passar pelo carregamento dinâmico de `.so`).
- [`source/plugins/PluginConnectorDummyImpl1.cpp`](../source/plugins/PluginConnectorDummyImpl1.cpp)
  — `ContinuousSystemComponent` e `ODESolver` registrados no conector estático de plugins
  (`continuoussystemcomponent.so`, `odesolver.so`), tornando-os descobríveis pelo
  `PluginManager` da mesma forma que os demais componentes/definições de dados do simulador.

### Comentários/robustez
- [`source/tools/RungeKutta4OdeSolver.h`](../source/tools/RungeKutta4OdeSolver.h) — comentários
  explicando cada estágio (k1..k4) do RK4 clássico; sem mudança de lógica.

## Testes

| Arquivo | Depende do GenESyS? | O que valida |
|---|---|---|
| [`source/tests/test_lsode.cpp`](../source/tests/test_lsode.cpp) | Sim (kernel + plugins) | `LSODE` dentro do simulador, contra a solução analítica do oscilador harmônico (`cos(t)`/`-sin(t)`) — exercita diretamente o bug corrigido em `_doStep()`. |
| [`source/tests/test_continuous_system.cpp`](../source/tests/test_continuous_system.cpp) | Sim (kernel + plugins) | `ContinuousSystemComponent` + `ODESolver` rodando dentro de um modelo completo (`Simulator`/`Model`), também contra a solução analítica. |
| [`source/tests/test_ode_solver_numerical_validation.cpp`](../source/tests/test_ode_solver_numerical_validation.cpp) | **Não** — só usa `source/tools/OdeSolver_if.h`, `OdeSystem_if.h`, `RungeKutta4OdeSolver.h` | Prova real isolada: valida a matemática do `RungeKutta4OdeSolver` sozinho, sem nenhuma dependência do kernel/plugins do simulador. Não está integrado ao CMake (compilado/rodado manualmente). |

Os dois primeiros estão registrados como testes de smoke no CMake
([`source/tests/smoke/CMakeLists.txt`](../source/tests/smoke/CMakeLists.txt)), rodando via `ctest`.

## Evidência de execução

Ambiente: WSL2 Ubuntu 26.04, `cmake 4.2.3`, `g++ 15.2.0` (Unix Makefiles), build `Debug`.

```
$ ctest -R 'test_continuous_system|test_lsode' --output-on-failure
Test project .../build
    Start 19: test_continuous_system
1/2 Test #19: test_continuous_system ...........   Passed    0.27 sec
    Start 20: test_lsode
2/2 Test #20: test_lsode .......................   Passed    0.32 sec

100% tests passed, 0 tests failed out of 2
```

### `test_lsode` — saída
```
t final LSODE = 6.2800000000
x final = 0.9999949269, esperado = 0.9999949269, erro = 0.0000000000
v final = 0.0031853023, esperado = 0.0031853018, erro = 0.0000000005
variacao maxima de energia = 0.0000000009%
pontos coletados = 65
Teste PASSOU: LSODE acompanha cos(t) e -sin(t)
```

### `test_continuous_system` — saída (resumo)
```
Valores finais:
  x_final = 0.999999999996
  v_final = 4.99375492347e-10
Erros (vs. solução exata em t=2*pi):
  err_x = 4.13280520917e-12
  err_v = 4.99375247417e-10
variação percentual máxima de energia = 8.26561041833e-10%

✓ Teste PASSOU: integração contínua numericamente estável em t=2*pi
```
