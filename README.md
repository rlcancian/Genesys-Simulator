# DCS — Solver de EDO (Factory + Dormand-Prince 5(4)) e extensão de difusão N-D

Extensão do simulador GenESyS (branch `2026-1`) para a disciplina de Modelagem e Simulação,
em duas partes coesas.

## Visualizar nosso vídeo:
Está presente no diretório raíz do projeto, como `video_entrega.mp4`

Alternativamente, é possível visualizá-lo no youtube por aqui: https://youtu.be/0pVlK125Oio

## Visualizar nosso Pull Request:
https://github.com/rlcancian/Genesys-Simulator/pull/425

## Relatório
Está presente no diretório raíz do projeto, como `relatorio.pdf`


## Parte 1 — Factory de solvers de EDO + Dormand-Prince 5(4)
A integração de EDOs no plugin `BioNetwork` estava presa ao Runge-Kutta 4 de passo fixo. Esta parte:
1. cria uma **Factory** (`OdeSolverFactory`) que produz solvers `OdeSolver_if` sob demanda;
2. implementa o método embutido e adaptativo **Dormand-Prince 5(4)** (`DormandPrince54OdeSolver`);
3. permite **escolher o solver em tempo de execução** no `BioNetwork` (campo persistido e validado).


## Parte 2 — Plugin de difusão N-D pelo Método das Linhas (extensão)
Um novo plugin (`DiffusionField`) resolve a equação de difusão `∂u/∂t = D∇²u` em **N dimensões
(parametrizável)** pelo **Método das Linhas**: diferenças finitas centrais no espaço transformam a
EDP num sistema de EDOs, integrado pelos **mesmos solvers**, via a mesma `OdeSolverFactory` — sem
alterar uma linha do componente da Parte 1.

**Resultado combinado:** 71 verificações automatizadas, 0 falhas. Ordens de convergência confirmadas
(RK4 ~4,0 no tempo; difusão O(h²) no espaço), conservação de massa Neumann a 1e-16, e o plugin de
difusão compilando contra o kernel real.


## Pré-requisitos

- **CMake ≥ 3.24**, gerador **Ninja** e um compilador com **C++23** (GCC 13+/Clang 16+).
- **GoogleTest** (resolvido pelo próprio build dos testes).


## Compilar e rodar os testes

Rodar **apenas os testes do DCS** (filtro por nome com `-R`):

```bash
# Parte 1 — Factory + Dormand-Prince + RK4
ctest --preset tests-kernel-unit -R "OdeSolverFactory|DormandPrince54|RungeKutta4|OdeSolverContract"

# Parte 2 — Método das Linhas N-D (núcleo)
ctest --preset tests-kernel-unit -R "DiffusionMol"

# Integração (plugins escolhendo solver)
ctest --preset tests-kernel-unit -R "Diffusion|BioNetwork"
```

Rodar todos os testes:
```bash
# 1. Configurar o preset de testes unitários do kernel
cmake --preset tests-kernel-unit

# 2. Compilar tudo (kernel, plugins, tools, testes e a demo)
cmake --build --preset tests-kernel-unit-run

# 3. Rodar TODOS os testes
ctest --preset tests-kernel-unit
```




Ou executar os binários diretamente (aceitam `--gtest_filter`):

```bash
BIN=build/tests-kernel-unit/source/tests/unit
$BIN/genesys_test_tools_ode_solver_factory
$BIN/genesys_test_tools_diffusion_mol
$BIN/genesys_test_simulator_runtime --gtest_filter='*Diffusion*'
```

## Rodar a demo (difusão 2D animada)

A demo já é compilada no passo de build acima. Para compilar só ela:

```bash
cmake --build --preset tests-kernel-unit-run --target genesys_diffusion_ascii_demo
```

Executar:

```bash
./build/tests-kernel-unit/source/tests/genesys_diffusion_ascii_demo
```

Mostra uma malha **2D 41×41**, condição inicial **Gaussiana**, contorno **Neumann**, solver
**Dormand-Prince 5(4)**, integrando de `t=0` a `t=1.5`. A cada passo, redesenha o campo com blocos
Unicode e imprime o tempo, a **massa total** (que permanece constante — checagem de
conservação) e o valor máximo. Requer terminal UTF-8.


## Escolher o solver de EDO (Parte 1)

Tanto `BioNetwork` quanto `DiffusionField` expõem o controle **`OdeSolver`**. Trocar entre
`"RungeKutta4"` e `"DormandPrince54"` muda o método de integração **sem alterar nenhuma linha de
lógica** — a `OdeSolverFactory` resolve o nome em tempo de execução. Se o nome for desconhecido, automaticamente se usa Runge-Kutta-4.
```cpp
field.setOdeSolver("DormandPrince54");  // adaptativo, mais estável em malha fina
field.setOdeSolver("RungeKutta4");      // passo fixo, mais leve
```

