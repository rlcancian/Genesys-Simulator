#!/usr/bin/env bash
set -Eeuo pipefail
IFS=$'\n\t'

# Build direto e incremental do Genesys sem CMake/Ninja.
#
# Presets:
#   terminal-app
#   web-app
#   gui-app
#   smart:<Classe>
#   example:<caminho-relativo.cpp>
#
# Exemplos:
#   ./build-gcc.sh terminal-app
#   ./build-gcc.sh web-app
#   ./build-gcc.sh gui-app
#   ./build-gcc.sh smart:Smart_SeizeDelayRelease
#   ./build-gcc.sh example:smarts/Smart_SeizeDelayRelease.cpp
#   ./build-gcc.sh example:teaching/AnElectronicAssemblyAndTestSystem.cpp --class AnElectronicAssemblyAndTestSystem
#   ./build-gcc.sh --list
#   ./build-gcc.sh terminal-app --dry-run
#   ./build-gcc.sh gui-app --clean
#
# Saídas:
#   build-gcc/terminal-app/genesys_terminal_application
#   build-gcc/web-app/genesys_web_app
#   build-gcc/gui-app/genesys_qt_gui_application
#
# Requisitos mínimos:
#   - bash, find, grep, sed, sort, tr
#   - g++ com C++23
#
# Requisitos adicionais para gui-app:
#   Ubuntu 24.04:
#     sudo apt install g++ pkg-config qt6-base-dev qt6-base-dev-tools
#
# Observação:
#   A GUI Qt é best-effort fora do CMake, pois o CMake normalmente executa
#   AUTOMOC/AUTOUIC/AUTORCC. Este script replica isso diretamente com moc/uic/rcc.

usage() {
  cat <<'HELP'
Usage:
  ./build-gcc.sh <preset> [OPTIONS]

Presets:
  terminal-app
      Compila a aplicação terminal padrão: GenesysShell.

  web-app
      Compila a aplicação web: genesys_web_app.

  gui-app
      Compila a GUI Qt: genesys_qt_gui_application.
      Requer Qt5 ou Qt6 development tools.

  smart:<Classe>
      Compila um exemplo smart de source/applications/terminal/examples/smarts.
      Exemplo:
        ./build-gcc.sh smart:Smart_SeizeDelayRelease

  example:<relative.cpp>
      Compila um exemplo em source/applications/terminal/examples.
      Exemplo:
        ./build-gcc.sh example:smarts/Smart_SeizeDelayRelease.cpp
        ./build-gcc.sh example:teaching/AnElectronicAssemblyAndTestSystem.cpp --class AnElectronicAssemblyAndTestSystem

Options:
  --class NAME       Classe C++ do exemplo terminal. Default: nome-base do .cpp.
  --dry-run          Mostra comandos sem executar.
  --clean            Remove o diretório de build do preset e encerra.
  --list             Lista presets e exemplos smart conhecidos.
  -h, --help         Mostra esta ajuda.

Environment:
  CXX                Compilador. Default: g++.
                     Use CXX=gcc apenas se o gcc suportar C++ e link com libstdc++.
  BUILD_ROOT         Diretório raiz de build. Default: build-gcc.
  CXXFLAGS_EXTRA     Flags extras de compilação.
  LDFLAGS_EXTRA      Flags extras de linkedição.

Examples:
  ./build-gcc.sh terminal-app
  ./build-gcc.sh web-app
  ./build-gcc.sh gui-app
  ./build-gcc.sh smart:Smart_ModalModelFSM
  ./build-gcc.sh example:smarts/Smart_PythonForG.cpp --class Smart_PythonForG
HELP
}

log()  { printf '[INFO] %s\n' "$*" >&2; }
warn() { printf '[WARN] %s\n' "$*" >&2; }
err()  { printf '[ERROR] %s\n' "$*" >&2; exit 1; }

list_presets() {
  cat <<'HELP'
Presets:
  terminal-app
  web-app
  gui-app
  smart:Smart_ModalModelFSM
  smart:Smart_SeizeDelayRelease
  smart:Smart_SeizeDelayReleaseMany
  smart:Smart_SeizeDelayReleaseNoDataDefs
  smart:Smart_PythonForG
  smart:Smart_R_Simulator
  example:<relative.cpp>

Examples:
  ./build-gcc.sh terminal-app
  ./build-gcc.sh web-app
  ./build-gcc.sh gui-app
  ./build-gcc.sh smart:Smart_SeizeDelayRelease
  ./build-gcc.sh example:teaching/AnElectronicAssemblyAndTestSystem.cpp --class AnElectronicAssemblyAndTestSystem
HELP
}

PRESET=""
DRY_RUN=0
CLEAN=0
LIST_ONLY=0
EXAMPLE_CLASS=""

while [[ $# -gt 0 ]]; do
  case "$1" in
    --dry-run)
      DRY_RUN=1
      shift
      ;;
    --clean)
      CLEAN=1
      shift
      ;;
    --list)
      LIST_ONLY=1
      shift
      ;;
    --class)
      [[ $# -ge 2 ]] || err "--class requer argumento"
      EXAMPLE_CLASS="$2"
      shift 2
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    -*)
      err "Argumento desconhecido: $1"
      ;;
    *)
      if [[ -n "$PRESET" ]]; then
        err "Mais de um preset informado: '$PRESET' e '$1'"
      fi
      PRESET="$1"
      shift
      ;;
  esac
done

if [[ "$LIST_ONLY" -eq 1 ]]; then
  list_presets
  exit 0
fi

[[ -n "$PRESET" ]] || { usage; exit 2; }

CXX="${CXX:-g++}"
BUILD_ROOT="${BUILD_ROOT:-build-gcc}"

case "$BUILD_ROOT" in
  ""|"/"|"/."|"."|"..")
    err "BUILD_ROOT inseguro: '$BUILD_ROOT'"
    ;;
esac

command -v "$CXX" >/dev/null 2>&1 || err "Compilador não encontrado: $CXX"
command -v find >/dev/null 2>&1 || err "Comando obrigatório não encontrado: find"
command -v grep >/dev/null 2>&1 || err "Comando obrigatório não encontrado: grep"
command -v sed >/dev/null 2>&1 || err "Comando obrigatório não encontrado: sed"
command -v sort >/dev/null 2>&1 || err "Comando obrigatório não encontrado: sort"
command -v tr >/dev/null 2>&1 || err "Comando obrigatório não encontrado: tr"

[[ -f CMakeLists.txt ]] || err "Execute este script na raiz do repositório Genesys-Simulator."
[[ -d source ]] || err "Diretório 'source' não encontrado."

safe_name() {
  printf '%s' "$1" | sed 's#[^A-Za-z0-9_.-]#_#g'
}

BUILD_NAME="$(safe_name "$PRESET")"
BUILD_DIR="${BUILD_ROOT}/${BUILD_NAME}"
OBJ_DIR="${BUILD_DIR}/obj"
GEN_DIR="${BUILD_DIR}/generated"
META_DIR="${BUILD_DIR}/meta"

case "$PRESET" in
  terminal-app)
    OUT="${BUILD_DIR}/genesys_terminal_application"
    ;;
  web-app)
    OUT="${BUILD_DIR}/genesys_web_app"
    ;;
  gui-app)
    OUT="${BUILD_DIR}/genesys_qt_gui_application"
    ;;
  smart:*)
    OUT="${BUILD_DIR}/genesys_terminal_application"
    ;;
  example:*)
    OUT="${BUILD_DIR}/genesys_terminal_application"
    ;;
  *)
    err "Preset não suportado: $PRESET. Use --list."
    ;;
esac

if [[ "$CLEAN" -eq 1 ]]; then
  log "Removendo diretório de build do preset: $BUILD_DIR"
  if [[ "$DRY_RUN" -eq 1 ]]; then
    printf 'rm -rf -- %q\n' "$BUILD_DIR"
  else
    rm -rf -- "$BUILD_DIR"
  fi
  exit 0
fi

mkdir -p "$OBJ_DIR" "$GEN_DIR" "$META_DIR"

log "Preset: $PRESET"
log "Compiler: $("$CXX" --version | head -n 1)"
log "Build dir: $BUILD_DIR"
log "Output: $OUT"

COMMON_FLAGS=(
  -std=c++23
  -O2
  -pipe
  -MMD
  -MP
  -pthread
  -Isource
  -Isource/applications
  -Isource/applications/terminal
  -DGENESYS_HAS_PYTHON_INTEGRATION=0
)

LINK_FLAGS=(
  -pthread
)

case "$(basename "$CXX")" in
  gcc|gcc-*)
    LINK_FLAGS+=(-lstdc++)
    ;;
esac

if [[ -n "${CXXFLAGS_EXTRA:-}" ]]; then
  # shellcheck disable=SC2206
  EXTRA_COMPILE_FLAGS=(${CXXFLAGS_EXTRA})
  COMMON_FLAGS+=("${EXTRA_COMPILE_FLAGS[@]}")
fi

if [[ -n "${LDFLAGS_EXTRA:-}" ]]; then
  # shellcheck disable=SC2206
  EXTRA_LINK_FLAGS=(${LDFLAGS_EXTRA})
  LINK_FLAGS+=("${EXTRA_LINK_FLAGS[@]}")
fi

declare -A SEEN_SOURCES=()
SOURCES=()

add_source() {
  local src="$1"

  [[ -f "$src" ]] || err "Fonte obrigatório não encontrado: $src"

  if [[ -z "${SEEN_SOURCES[$src]:-}" ]]; then
    SOURCES+=("$src")
    SEEN_SOURCES["$src"]=1
  fi
}

add_optional_source() {
  local src="$1"

  if [[ -f "$src" ]]; then
    add_source "$src"
  fi
}

add_sources_from_find() {
  local dir="$1"
  shift

  [[ -d "$dir" ]] || err "Diretório obrigatório não encontrado: $dir"

  while IFS= read -r src; do
    add_source "$src"
  done < <(find "$dir" "$@" -type f -name '*.cpp' | sort)
}

add_sources_from_find_excluding_regex() {
  local dir="$1"
  local regex="$2"

  [[ -d "$dir" ]] || err "Diretório obrigatório não encontrado: $dir"

  while IFS= read -r src; do
    if [[ ! "$src" =~ $regex ]]; then
      add_source "$src"
    fi
  done < <(find "$dir" -type f -name '*.cpp' | sort)
}

quote_command() {
  printf '%q ' "$@"
  printf '\n'
}

# ----------------------------------------------------------------------
# Fonte comum: kernel, parser, plugins.
# ----------------------------------------------------------------------

add_core_sources() {
  # Kernel util.
  add_source "source/kernel/util/Util.cpp"

  # Kernel statistics.
  add_sources_from_find "source/kernel/statistics" -maxdepth 1

  # Kernel simulator support.
  add_source "source/kernel/simulator/LicenceManager.cpp"
  add_source "source/kernel/simulator/ParserManager.cpp"
  add_source "source/kernel/simulator/ParserChangesInformation.cpp"
  add_source "source/kernel/simulator/PluginInformation.cpp"
  add_source "source/kernel/simulator/SystemDependencyResolver.cpp"
  add_source "source/kernel/simulator/Plugin.cpp"
  add_source "source/kernel/simulator/TraceManager.cpp"
  add_source "source/kernel/simulator/OnEventManager.cpp"
  add_source "source/kernel/simulator/ConnectionManager.cpp"
  add_source "source/kernel/simulator/Event.cpp"
  add_source "source/kernel/simulator/ExperimentManager.cpp"
  add_source "source/kernel/simulator/SimulationExperiment.cpp"
  add_source "source/kernel/simulator/SimulationScenario.cpp"
  add_source "source/kernel/simulator/Persistence.cpp"

  add_sources_from_find "source/kernel/simulator/model"
  add_sources_from_find "source/kernel/simulator/essentialPlugins"
  add_sources_from_find "source/kernel/simulator/persistence"

  add_source "source/kernel/simulator/ParserDefaultImpl2.cpp"
  add_source "source/kernel/simulator/SimulationReporterDefaultImpl1.cpp"
  add_source "source/kernel/simulator/SourceModelComponent.cpp"
  add_source "source/kernel/simulator/SinkModelComponent.cpp"

  # Kernel simulator runtime.
  while IFS= read -r src; do
    case "$src" in
      source/kernel/simulator/PluginConnectorDummyBootstrap.cpp) continue ;;
      source/kernel/simulator/Persistence.cpp) continue ;;
      source/kernel/simulator/ParserDefaultImpl2.cpp) continue ;;
      source/kernel/simulator/SimulationReporterDefaultImpl1.cpp) continue ;;
      source/kernel/simulator/SourceModelComponent.cpp) continue ;;
      source/kernel/simulator/SinkModelComponent.cpp) continue ;;
    esac
    add_source "$src"
  done < <(find source/kernel/simulator -maxdepth 1 -type f -name '*.cpp' | sort)

  # Parser: usa fontes já geradas. Não regenera bison/flex.
  add_sources_from_find "source/parser" -maxdepth 1

  # Plugins mínimos estáticos.
  add_sources_from_find "source/plugins/data"
  add_sources_from_find "source/plugins/components"

  add_source "source/plugins/PluginConnectorStaticImpl1.cpp"
  add_source "source/plugins/PluginConnectorDummyImpl1.cpp"
}

add_terminal_application_sources() {
  local class_name="$1"
  local header_path="$2"
  local source_path="$3"

  add_source "source/applications/terminal/main.cpp"
  add_source "source/applications/BaseGenesysTerminalApplication.cpp"
  add_source "$source_path"

  COMMON_FLAGS+=("-DGENESYS_TERMINAL_APP_CLASS=${class_name}")
  COMMON_FLAGS+=("-DGENESYS_TERMINAL_APP_HEADER=\"${header_path}\"")
}

add_web_sources() {
  COMMON_FLAGS+=("-Isource/applications/web")

  add_source "source/applications/web/api/ApiRouter.cpp"
  add_source "source/applications/web/auth/TokenService.cpp"
  add_source "source/applications/web/http/SimpleHttpServer.cpp"
  add_source "source/applications/web/service/WebWorkerRuntime.cpp"
  add_source "source/applications/web/service/SimulatorSessionService.cpp"
  add_source "source/applications/web/session/SessionManager.cpp"
  add_source "source/applications/web/worker/WorkerJobManager.cpp"

  add_source "source/applications/web/main.cpp"
  add_source "source/applications/web/BaseGenesysWebApplication.cpp"
}

add_tools_sources() {
  add_source "source/tools/AIAssistant/AIAssistantDefaultImpl.cpp"
  add_source "source/tools/AIAssistant/AIAuditLog.cpp"
  add_source "source/tools/AIAssistant/AISecretStore.cpp"
  add_source "source/tools/AIAssistant/AnthropicProviderClientImpl.cpp"
  add_source "source/tools/AIAssistant/HttpProviderClientBase.cpp"
  add_source "source/tools/AIAssistant/LocalProviderClientImpl.cpp"
  add_source "source/tools/AIAssistant/OpenAIProviderClientImpl.cpp"
  add_source "source/tools/Statistics/FitterDummyImpl.cpp"
  add_source "source/tools/Statistics/HypothesisTesterDefaultImpl1.cpp"
  add_source "source/tools/Statistics/ProbabilityDistribution.cpp"
  add_source "source/tools/Statistics/ProbabilityDistributionBase.cpp"
  add_source "source/tools/Statistics/SimulationResultsDataset.cpp"
  add_source "source/tools/Optimization/OptimizerDefaultImpl1.cpp"
  add_source "source/tools/Continuous/SolverDefaultImpl1.cpp"
  add_source "source/tools/FactorialDesign/FactorialDesign.cpp"
}

# ----------------------------------------------------------------------
# Qt helpers para gui-app.
# ----------------------------------------------------------------------

QT_MAJOR=""
QT_PKGS=()
QT_CFLAGS=()
QT_LIBS=()
MOC=""
UIC=""
RCC=""

find_qt_tool() {
  local tool="$1"
  shift
  local candidate

  for candidate in "$@"; do
    if [[ -n "$candidate" && -x "$candidate" ]]; then
      printf '%s\n' "$candidate"
      return 0
    fi
    if [[ -n "$candidate" ]] && command -v "$candidate" >/dev/null 2>&1; then
      command -v "$candidate"
      return 0
    fi
  done

  return 1
}

qt_host_bins() {
  local query_bin="$1"
  local value=""

  if command -v "$query_bin" >/dev/null 2>&1; then
    value="$("$query_bin" -query QT_HOST_BINS 2>/dev/null || true)"
    if [[ -n "$value" && -d "$value" ]]; then
      printf '%s\n' "$value"
      return 0
    fi
  fi

  return 1
}

detect_qt() {
  command -v pkg-config >/dev/null 2>&1 || err "pkg-config não encontrado. Para gui-app instale: sudo apt install pkg-config qt6-base-dev qt6-base-dev-tools"

  if pkg-config --exists Qt6Core Qt6Gui Qt6Widgets Qt6PrintSupport Qt6Network; then
    QT_MAJOR="6"
    QT_PKGS=(Qt6Core Qt6Gui Qt6Widgets Qt6PrintSupport Qt6Network)
  elif pkg-config --exists Qt5Core Qt5Gui Qt5Widgets Qt5PrintSupport Qt5Network; then
    QT_MAJOR="5"
    QT_PKGS=(Qt5Core Qt5Gui Qt5Widgets Qt5PrintSupport Qt5Network)
  else
    err "Qt5/Qt6 development packages não encontrados via pkg-config. Em Ubuntu 24.04 tente: sudo apt install qt6-base-dev qt6-base-dev-tools"
  fi

  # shellcheck disable=SC2207
  QT_CFLAGS=($(pkg-config --cflags "${QT_PKGS[@]}"))
  # shellcheck disable=SC2207
  QT_LIBS=($(pkg-config --libs "${QT_PKGS[@]}"))

  local host_bins=""
  if [[ "$QT_MAJOR" == "6" ]]; then
    host_bins="$(qt_host_bins qmake6 || true)"
    [[ -z "$host_bins" ]] && host_bins="$(qt_host_bins qmake || true)"
    MOC="$(find_qt_tool moc "${host_bins:+$host_bins/moc}" moc6 moc-qt6 moc || true)"
    UIC="$(find_qt_tool uic "${host_bins:+$host_bins/uic}" uic6 uic-qt6 uic || true)"
    RCC="$(find_qt_tool rcc "${host_bins:+$host_bins/rcc}" rcc6 rcc-qt6 rcc || true)"
  else
    host_bins="$(qt_host_bins qmake || true)"
    MOC="$(find_qt_tool moc "${host_bins:+$host_bins/moc}" moc-qt5 moc || true)"
    UIC="$(find_qt_tool uic "${host_bins:+$host_bins/uic}" uic-qt5 uic || true)"
    RCC="$(find_qt_tool rcc "${host_bins:+$host_bins/rcc}" rcc-qt5 rcc || true)"
  fi

  [[ -n "$MOC" ]] || err "moc não encontrado. Instale qt6-base-dev-tools ou qtbase5-dev-tools."
  [[ -n "$UIC" ]] || err "uic não encontrado. Instale qt6-base-dev-tools ou qtbase5-dev-tools."
  [[ -n "$RCC" ]] || err "rcc não encontrado. Instale qt6-base-dev-tools ou qtbase5-dev-tools."

  COMMON_FLAGS+=("${QT_CFLAGS[@]}")
  LINK_FLAGS+=("${QT_LIBS[@]}")

  log "Qt: Qt${QT_MAJOR}"
  log "moc: $MOC"
  log "uic: $UIC"
  log "rcc: $RCC"
}

needs_regen() {
  local out="$1"
  shift

  [[ ! -f "$out" ]] && return 0

  local dep
  for dep in "$@"; do
    [[ -e "$dep" && "$dep" -nt "$out" ]] && return 0
  done

  return 1
}

run_or_print() {
  if [[ "$DRY_RUN" -eq 1 ]]; then
    quote_command "$@"
  else
    "$@"
  fi
}

generated_cpp_name_for() {
  local prefix="$1"
  local src="$2"
  local flat
  flat="$(printf '%s' "$src" | sed 's#[^A-Za-z0-9_.-]#_#g')"
  printf '%s/%s_%s.cpp' "$GEN_DIR" "$prefix" "$flat"
}

add_qt_generated_sources() {
  local gui_dir="source/applications/gui/qt/GenesysQtGUI"
  local src out stem

  mkdir -p "$GEN_DIR/ui"

  # AUTOUIC aproximado: gera ui_*.h para todos os .ui.
  while IFS= read -r src; do
    stem="$(basename "${src%.ui}")"
    out="${GEN_DIR}/ui/ui_${stem}.h"

    if needs_regen "$out" "$src"; then
      log "uic: $src"
      run_or_print "$UIC" "$src" -o "$out"
    else
      log "Atualizado: $out"
    fi
  done < <(find "$gui_dir" -type f -name '*.ui' | sort)

  COMMON_FLAGS+=("-I${GEN_DIR}/ui")

  # AUTOMOC aproximado: gera moc_*.cpp para headers com Q_OBJECT/Q_GADGET/Q_NAMESPACE.
  while IFS= read -r src; do
    out="$(generated_cpp_name_for moc "$src")"

    if needs_regen "$out" "$src"; then
      log "moc: $src"
      run_or_print "$MOC" "${COMMON_FLAGS[@]}" "$src" -o "$out"
    else
      log "Atualizado: $out"
    fi

    add_source "$out"
  done < <(
    grep -RIlE 'Q_OBJECT|Q_GADGET|Q_NAMESPACE' "$gui_dir" \
      --include='*.h' --include='*.hpp' \
      | sort
  )

  # AUTORCC aproximado: gera qrc_*.cpp para recursos.
  while IFS= read -r src; do
    stem="$(basename "${src%.qrc}")"
    out="${GEN_DIR}/qrc_${stem}.cpp"

    if needs_regen "$out" "$src"; then
      log "rcc: $src"
      run_or_print "$RCC" -name "$stem" "$src" -o "$out"
    else
      log "Atualizado: $out"
    fi

    add_source "$out"
  done < <(find "$gui_dir" -type f -name '*.qrc' | sort)
}

add_gui_sources() {
  local gui_dir="source/applications/gui/qt/GenesysQtGUI"

  detect_qt

  COMMON_FLAGS+=(
    "-I${gui_dir}"
    "-I${gui_dir}/codeeditor"
    "-I${gui_dir}/propertyeditor"
    "-I${gui_dir}/propertyeditor/qtpropertybrowser"
    "-I${gui_dir}/dialogs"
    "-Isource/applications/web"
    "-I${GEN_DIR}"
  )

  # Fontes da GUI: replica o GLOB_RECURSE do CMake, excluindo /build/ e qcustomplot.cpp.
  add_sources_from_find_excluding_regex "$gui_dir" '/build/|/qcustomplot\.cpp$'

  # Fallback terminal usado pela GUI no CMake.
  add_source "source/applications/BaseGenesysTerminalApplication.cpp"
  add_source "source/applications/terminal/GenesysShell/GenesysShell.cpp"

  # GUI também linka web_core e tools no CMake.
  add_web_core_only_sources
  add_tools_sources

  add_qt_generated_sources
}

add_web_core_only_sources() {
  COMMON_FLAGS+=("-Isource/applications/web")

  add_source "source/applications/web/api/ApiRouter.cpp"
  add_source "source/applications/web/auth/TokenService.cpp"
  add_source "source/applications/web/http/SimpleHttpServer.cpp"
  add_source "source/applications/web/service/WebWorkerRuntime.cpp"
  add_source "source/applications/web/service/SimulatorSessionService.cpp"
  add_source "source/applications/web/session/SessionManager.cpp"
  add_source "source/applications/web/worker/WorkerJobManager.cpp"
}

# ----------------------------------------------------------------------
# Seleção do preset.
# ----------------------------------------------------------------------

add_core_sources

case "$PRESET" in
  terminal-app)
    add_terminal_application_sources \
      "GenesysShell" \
      "GenesysShell/GenesysShell.h" \
      "source/applications/terminal/GenesysShell/GenesysShell.cpp"
    ;;

  web-app)
    add_web_sources
    ;;

  gui-app)
    add_gui_sources
    ;;

  smart:*)
    smart_class="${PRESET#smart:}"
    [[ -n "$smart_class" ]] || err "Preset smart sem classe."

    smart_source="source/applications/terminal/examples/smarts/${smart_class}.cpp"
    smart_header="examples/smarts/${smart_class}.h"

    [[ -f "$smart_source" ]] || err "Smart example não encontrado: $smart_source"

    add_terminal_application_sources \
      "$smart_class" \
      "$smart_header" \
      "$smart_source"
    ;;

  example:*)
    example_rel="${PRESET#example:}"
    [[ -n "$example_rel" ]] || err "Preset example sem caminho."

    case "$example_rel" in
      /*|*..*)
        err "example deve ser caminho relativo seguro dentro de source/applications/terminal/examples"
        ;;
    esac

    [[ "$example_rel" == *.cpp ]] || err "example deve apontar para arquivo .cpp"

    example_source="source/applications/terminal/examples/${example_rel}"
    example_header="examples/${example_rel%.cpp}.h"

    [[ -f "$example_source" ]] || err "Example source não encontrado: $example_source"
    [[ -f "source/applications/terminal/${example_header}" ]] || warn "Header esperado não encontrado: source/applications/terminal/${example_header}"

    if [[ -n "$EXAMPLE_CLASS" ]]; then
      example_class="$EXAMPLE_CLASS"
    else
      example_class="$(basename "${example_rel%.cpp}")"
    fi

    add_terminal_application_sources \
      "$example_class" \
      "$example_header" \
      "$example_source"
    ;;
esac

log "Fonte(s): ${#SOURCES[@]}"

# ----------------------------------------------------------------------
# Compilação incremental.
# ----------------------------------------------------------------------

object_for_source() {
  local src="$1"
  printf '%s/%s.o' "$OBJ_DIR" "${src%.cpp}"
}

dep_for_object() {
  local obj="$1"
  printf '%s.d' "$obj"
}

cmd_for_object() {
  local obj="$1"
  printf '%s.cmd' "$obj"
}

dependency_is_newer_than_object() {
  local depfile="$1"
  local obj="$2"

  [[ -f "$depfile" ]] || return 1

  while IFS= read -r dep; do
    [[ -z "$dep" ]] && continue
    [[ "$dep" == *: ]] && continue
    [[ -e "$dep" && "$dep" -nt "$obj" ]] && return 0
  done < <(
    sed \
      -e 's/\\$//' \
      -e 's/^[^:]*://' \
      "$depfile" \
      | tr ' ' '\n' \
      | sed '/^$/d'
  )

  return 1
}

needs_compile() {
  local src="$1"
  local obj="$2"
  local depfile="$3"
  local cmdfile="$4"
  shift 4
  local cmd_text

  [[ ! -f "$obj" ]] && return 0
  [[ "$src" -nt "$obj" ]] && return 0
  dependency_is_newer_than_object "$depfile" "$obj" && return 0

  cmd_text="$(quote_command "$@")"
  [[ ! -f "$cmdfile" ]] && return 0
  [[ "$(cat "$cmdfile")" != "$cmd_text" ]] && return 0

  return 1
}

compile_one() {
  local src="$1"
  local obj depfile cmdfile
  obj="$(object_for_source "$src")"
  depfile="$(dep_for_object "$obj")"
  cmdfile="$(cmd_for_object "$obj")"

  mkdir -p "$(dirname "$obj")" "$(dirname "$cmdfile")"

  local cmd=(
    "$CXX"
    "${COMMON_FLAGS[@]}"
    -c "$src"
    -o "$obj"
  )

  if needs_compile "$src" "$obj" "$depfile" "$cmdfile" "${cmd[@]}"; then
    log "Compilando: $src"
    if [[ "$DRY_RUN" -eq 1 ]]; then
      quote_command "${cmd[@]}"
    else
      "${cmd[@]}"
      quote_command "${cmd[@]}" > "$cmdfile"
    fi
  else
    log "Atualizado: $src"
  fi
}

OBJECTS=()

for src in "${SOURCES[@]}"; do
  obj="$(object_for_source "$src")"
  OBJECTS+=("$obj")
  compile_one "$src"
done

LINK_CMD_FILE="${META_DIR}/link.cmd"
LINK_CMD=(
  "$CXX"
  "${OBJECTS[@]}"
  -o "$OUT"
  "${LINK_FLAGS[@]}"
)

needs_link() {
  local cmd_text

  [[ ! -x "$OUT" ]] && return 0

  for obj in "${OBJECTS[@]}"; do
    [[ ! -f "$obj" ]] && return 0
    [[ "$obj" -nt "$OUT" ]] && return 0
  done

  cmd_text="$(quote_command "${LINK_CMD[@]}")"
  [[ ! -f "$LINK_CMD_FILE" ]] && return 0
  [[ "$(cat "$LINK_CMD_FILE")" != "$cmd_text" ]] && return 0

  return 1
}

if needs_link; then
  log "Linkando: $OUT"
  if [[ "$DRY_RUN" -eq 1 ]]; then
    quote_command "${LINK_CMD[@]}"
  else
    "${LINK_CMD[@]}"
    quote_command "${LINK_CMD[@]}" > "$LINK_CMD_FILE"
  fi
else
  log "Binário já atualizado: $OUT"
fi

if [[ "$DRY_RUN" -eq 0 ]]; then
  [[ -x "$OUT" ]] || err "Build terminou, mas o binário não foi gerado: $OUT"
  log "Build concluído."
  log "Execute: ./$OUT"
else
  log "Dry-run concluído."
fi
