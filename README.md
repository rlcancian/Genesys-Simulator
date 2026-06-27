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

- **CMake ≥ 3.24**, compilador com **C++23** (GCC 13+/Clang 16+) e um gerador: **Ninja** (usado pelos presets) **ou Make**.
- **GoogleTest** (resolvido pelo próprio build dos testes).


## Compilar e rodar os testes

> Compilar os testes compila também as libs do kernel (a lib `genesys_tools` depende delas),
> mas **não** precisa de Qt nem das aplicações terminal/GUI.

### 1. Configurar

```bash
# Opção A — presets do projeto (requer Ninja)
cmake --preset tests-kernel-unit

# Opção B — manual, sem Ninja (usa Make); gera em build/tests-unit
cmake -S . -B build/tests-unit -G "Unix Makefiles" \
  -DGENESYS_BUILD_TESTS=ON \
  -DGENESYS_BUILD_TERMINAL_APPLICATION=OFF \
  -DGENESYS_BUILD_GUI_APPLICATION=OFF
```

### 2. Compilar

```bash
# com preset
cmake --build --preset tests-kernel-unit-run
# ou, com a build manual (Make)
cmake --build build/tests-unit -j
```

### 3. Rodar

Use `--preset tests-kernel-unit` (se configurou por preset) ou `--test-dir build/tests-unit`
(se configurou manual):

```bash
# todos os testes
ctest --test-dir build/tests-unit

# apenas os testes do DCS (filtro por nome -R)
ctest --test-dir build/tests-unit -R "OdeSolverFactory|DormandPrince54|RungeKutta4|OdeSolverContract"  # Parte 1
ctest --test-dir build/tests-unit -R "DiffusionMol"                                                     # Parte 2
ctest --test-dir build/tests-unit -R "Diffusion|BioNetwork"                                             # integração
```




Ou executar os binários diretamente (aceitam `--gtest_filter`):

```bash
BIN=build/tests-unit/source/tests/unit   # ou build/tests-kernel-unit/... se usou o preset
$BIN/genesys_test_tools_ode_solver_factory
$BIN/genesys_test_tools_diffusion_mol
$BIN/genesys_test_simulator_runtime --gtest_filter='*Diffusion*'
```

## Rodar a demo (difusão 2D animada)

A demo é o alvo `genesys_diffusion_ascii_demo` (fonte: `diffusion_demo.cpp`). Compilar só ela e executar:

```bash
# build manual (Make)
cmake --build build/tests-unit --target genesys_diffusion_ascii_demo -j
./build/tests-unit/source/tests/genesys_diffusion_ascii_demo

# (com preset:
#   cmake --build --preset tests-kernel-unit-run --target genesys_diffusion_ascii_demo
#   ./build/tests-kernel-unit/source/tests/genesys_diffusion_ascii_demo )
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

