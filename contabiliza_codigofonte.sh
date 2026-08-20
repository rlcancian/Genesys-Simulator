#!/usr/bin/env bash
#
# contabiliza_codigofonte.sh
#
# Estatísticas quantitativas do código-fonte C++ do Genesys-Simulator.
#
# O script:
#   - lê recursivamente arquivos C++ em src/ ou source/;
#   - NÃO modifica qualquer arquivo-fonte;
#   - mostra somente indicadores agregados;
#   - opcionalmente grava a mesma saída em:
#         build/contabiliza_codigofonte.dat
#
# Uso:
#   ./contabiliza_codigofonte.sh
#   ./contabiliza_codigofonte.sh --save
#   ./contabiliza_codigofonte.sh --source-dir src
#   ./contabiliza_codigofonte.sh --source-dir source
#

set -Eeuo pipefail
IFS=$'\n\t'

SOURCE_ARG=""
SAVE_REPORT=0

usage() {
    cat <<'__USAGE__'
Uso:
  ./contabiliza_codigofonte.sh [opções]

Opções:
  --source-dir DIR   Diretório de fontes.
                     Padrão: src/; se não existir, source/.
  --save             Salva também em build/contabiliza_codigofonte.dat
  -h, --help         Mostra esta ajuda.
__USAGE__
}

while (($# > 0)); do
    case "$1" in
        --source-dir)
            [[ $# -ge 2 ]] || {
                echo "[ERROR] --source-dir requer um argumento." >&2
                exit 2
            }
            SOURCE_ARG="$2"
            shift 2
            ;;
        --save)
            SAVE_REPORT=1
            shift
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            echo "[ERROR] Opção desconhecida: $1" >&2
            exit 2
            ;;
    esac
done

command -v python3 >/dev/null 2>&1 || {
    echo "[ERROR] python3 não encontrado." >&2
    exit 1
}

REPO_ROOT="$(
    cd -- "$(dirname -- "${BASH_SOURCE[0]}")"
    pwd -P
)"

if [[ -z "$SOURCE_ARG" ]]; then
    if [[ -d "$REPO_ROOT/src" ]]; then
        SOURCE_ARG="src"
    elif [[ -d "$REPO_ROOT/source" ]]; then
        SOURCE_ARG="source"
    else
        echo "[ERROR] Nem src/ nem source/ foram encontrados." >&2
        exit 1
    fi
fi

if [[ "$SOURCE_ARG" = /* ]]; then
    SOURCE_DIR="$SOURCE_ARG"
else
    SOURCE_DIR="$REPO_ROOT/$SOURCE_ARG"
fi

[[ -d "$SOURCE_DIR" ]] || {
    echo "[ERROR] Diretório inexistente: $SOURCE_DIR" >&2
    exit 1
}

SOURCE_DIR="$(
    cd -- "$SOURCE_DIR"
    pwd -P
)"

case "$SOURCE_DIR/" in
    "$REPO_ROOT/"*) ;;
    *)
        echo "[ERROR] O diretório precisa estar dentro do repositório." >&2
        exit 1
        ;;
esac

SAVE_PATH="$REPO_ROOT/build/contabiliza_codigofonte.dat"

if ((SAVE_REPORT)); then
    [[ -d "$REPO_ROOT/build" ]] || {
        echo "[ERROR] build/ não existe." >&2
        exit 1
    }
fi

python3 - "$SOURCE_DIR" "$REPO_ROOT" "$SAVE_PATH" "$SAVE_REPORT" <<'__PYTHON__'
from __future__ import annotations

import os
import re
import sys
from pathlib import Path
from collections import Counter, defaultdict


SOURCE_ROOT = Path(sys.argv[1]).resolve()
REPO_ROOT = Path(sys.argv[2]).resolve()
SAVE_PATH = Path(sys.argv[3])
SAVE_REPORT = bool(int(sys.argv[4]))


HEADER_EXTS = {
    ".h", ".hh", ".hpp", ".hxx",
    ".inl", ".ipp", ".tpp", ".inc", ".cuh",
}

IMPLEMENTATION_EXTS = {
    ".cpp", ".cc", ".cxx", ".C",
    ".cu", ".ixx", ".cppm",
}

ALL_EXTS = HEADER_EXTS | IMPLEMENTATION_EXTS


FIELDS = (
    "files",
    "headers",
    "implementations",
    "bytes",
    "loc",
    "sloc",
    "blank",
    "comment_only",
    "mixed",
    "classes",
    "interfaces",
    "abstract",
    "concrete",
    "methods",
    "pure_virtual",
    "override",
    "member_definitions",
    "enums",
    "templates",
    "includes",
    "todos",
)


def stats():
    return Counter({field: 0 for field in FIELDS})


def add(dst, src):
    for field in FIELDS:
        dst[field] += src.get(field, 0)


def ext_of(path: Path):
    suffix = path.suffix
    return ".C" if suffix == ".C" else suffix.lower()


def is_cpp(path: Path):
    return ext_of(path) in ALL_EXTS


def relative_to_source(path: Path):
    rel = path.relative_to(SOURCE_ROOT)
    text = rel.as_posix()
    return "." if text == "." else text


def remove_comments_and_strings(text: str):
    """
    Produz uma versão lexical do código mantendo quebras de linha e
    calcula linhas contendo código/comentários.
    """

    lines = text.splitlines()
    line_count = len(lines)

    has_code = [False] * line_count
    has_comment = [False] * line_count

    output = []

    state = "normal"
    quote = None
    escaped = False

    i = 0
    line_no = 0
    n = len(text)

    while i < n:
        c = text[i]
        nxt = text[i + 1] if i + 1 < n else ""

        if state == "normal":

            if c == "\n":
                output.append("\n")
                line_no += 1
                i += 1
                continue

            if c == "/" and nxt == "/":
                if line_no < line_count:
                    has_comment[line_no] = True
                output.extend((" ", " "))
                i += 2
                state = "line_comment"
                continue

            if c == "/" and nxt == "*":
                if line_no < line_count:
                    has_comment[line_no] = True
                output.extend((" ", " "))
                i += 2
                state = "block_comment"
                continue

            if c in ("'", '"'):
                if line_no < line_count:
                    has_code[line_no] = True
                quote = c
                escaped = False
                output.append(" ")
                state = "string"
                i += 1
                continue

            output.append(c)

            if not c.isspace() and line_no < line_count:
                has_code[line_no] = True

            i += 1
            continue

        if state == "line_comment":

            if line_no < line_count:
                has_comment[line_no] = True

            if c == "\n":
                output.append("\n")
                line_no += 1
                state = "normal"
            else:
                output.append(" ")

            i += 1
            continue

        if state == "block_comment":

            if line_no < line_count:
                has_comment[line_no] = True

            if c == "*" and nxt == "/":
                output.extend((" ", " "))
                i += 2
                state = "normal"
                continue

            if c == "\n":
                output.append("\n")
                line_no += 1
            else:
                output.append(" ")

            i += 1
            continue

        if state == "string":

            if line_no < line_count:
                has_code[line_no] = True

            if c == "\n":
                output.append("\n")
                line_no += 1
                escaped = False
                i += 1
                continue

            output.append(" ")

            if escaped:
                escaped = False
            elif c == "\\":
                escaped = True
            elif c == quote:
                state = "normal"
                quote = None

            i += 1

    blank = sum(1 for line in lines if not line.strip())

    sloc = sum(1 for value in has_code if value)

    comment_only = sum(
        1
        for code, comment in zip(has_code, has_comment)
        if comment and not code
    )

    mixed = sum(
        1
        for code, comment in zip(has_code, has_comment)
        if code and comment
    )

    return "".join(output), {
        "loc": line_count,
        "sloc": sloc,
        "blank": blank,
        "comment_only": comment_only,
        "mixed": mixed,
    }


CLASS_RE = re.compile(
    r"""
    \b(?P<kind>class|struct)\s+
    (?:
        [A-Z_][A-Z0-9_]*\s+
    )*
    (?P<name>[A-Za-z_]\w*)
    \s*
    (?:
        final\s*
    )?
    (?:
        :
        (?P<bases>[^;{}]+)
    )?
    \s*
    \{
    """,
    re.VERBOSE | re.MULTILINE,
)


METHOD_RE = re.compile(
    r"""
    (?P<name>
        operator\s*(?:\[\]|\(\)|[^\s(]+)
        |
        ~?[A-Za-z_]\w*
    )
    \s*
    \(
        (?P<params>
            (?:
                [^(){};]
                |
                \([^(){};]*\)
            )*
        )
    \)
    (?P<post>[^;{}]*?)
    (?:
        =
        \s*
        (?P<special>0|default|delete)
        \s*
    )?
    (?P<end>;|\{)
    """,
    re.VERBOSE | re.DOTALL,
)


MEMBER_DEFINITION_RE = re.compile(
    r"""
    \b
    [A-Za-z_]\w*
    (?:
        \s*::\s*[A-Za-z_]\w*
    )*
    \s*::\s*
    (?P<method>
        operator\s*(?:\[\]|\(\)|[^\s(]+)
        |
        ~?[A-Za-z_]\w*
    )
    \s*
    \(
    """,
    re.VERBOSE,
)


CONTROL_NAMES = {
    "if",
    "for",
    "while",
    "switch",
    "catch",
    "sizeof",
    "alignof",
    "decltype",
    "static_assert",
}


def matching_brace(text: str, opening: int):
    depth = 0

    for i in range(opening, len(text)):
        if text[i] == "{":
            depth += 1
        elif text[i] == "}":
            depth -= 1
            if depth == 0:
                return i

    return None


def brace_depth(body: str):
    depth = 0
    result = [0] * (len(body) + 1)

    for i, c in enumerate(body):
        result[i] = depth

        if c == "{":
            depth += 1
        elif c == "}":
            depth = max(0, depth - 1)

    return result


def has_data_members(body: str, depths):
    start = 0

    for i, char in enumerate(body):

        if char != ";" or depths[i] != 0:
            continue

        segment = body[start:i + 1]
        start = i + 1

        segment = re.sub(
            r"\b(?:public|private|protected)\s*:",
            " ",
            segment,
        ).strip()

        if not segment:
            continue

        if "(" in segment:
            continue

        if re.match(
            r"^(?:using|typedef|friend|static_assert|class|struct|enum|template)\b",
            segment,
        ):
            continue

        if segment.startswith("#"):
            continue

        if re.search(r"[A-Za-z_]\w*", segment):
            return True

    return False


def parse_classes(code: str, path: Path):

    result = []

    for match in CLASS_RE.finditer(code):

        prefix = code[max(0, match.start() - 20):match.start()]

        if re.search(r"\benum\s*$", prefix):
            continue

        opening = code.find("{", match.start(), match.end() + 1)

        if opening < 0:
            continue

        closing = matching_brace(code, opening)

        if closing is None:
            continue

        body = code[opening + 1:closing]
        name = match.group("name")

        depths = brace_depth(body)

        methods = []
        pure = 0
        overrides = 0

        for method in METHOD_RE.finditer(body):

            if depths[method.start()] != 0:
                continue

            method_name = re.sub(
                r"\s+",
                "",
                method.group("name"),
            )

            if method_name in CONTROL_NAMES:
                continue

            if re.fullmatch(r"[A-Z_][A-Z0-9_]*", method_name):
                continue

            methods.append(method_name)

            if method.group("special") == "0":
                pure += 1

            if re.search(
                r"\boverride\b",
                method.group("post") or "",
            ):
                overrides += 1

        behavioral_methods = [
            method
            for method in methods
            if method not in (name, "~" + name)
        ]

        filename_interface = bool(
            re.search(r"_if$", path.stem, re.IGNORECASE)
        )

        classname_interface = bool(
            re.search(r"_if$", name, re.IGNORECASE)
        )

        structural_interface = (
            pure > 0
            and behavioral_methods
            and pure >= len(behavioral_methods)
            and not has_data_members(body, depths)
        )

        if (
            filename_interface
            or classname_interface
            or structural_interface
        ):
            category = "interface"

        elif pure > 0:
            category = "abstract"

        else:
            category = "concrete"

        result.append({
            "category": category,
            "methods": len(methods),
            "pure": pure,
            "override": overrides,
        })

    return result


def analyze(path: Path):

    raw = path.read_bytes()

    try:
        text = raw.decode("utf-8-sig")
    except UnicodeDecodeError:
        text = raw.decode("utf-8", errors="replace")

    code, line_stats = remove_comments_and_strings(text)

    classes = parse_classes(code, path)

    st = stats()

    st["files"] = 1
    st["bytes"] = len(raw)

    ext = ext_of(path)

    if ext in HEADER_EXTS:
        st["headers"] = 1
    else:
        st["implementations"] = 1

    for key, value in line_stats.items():
        st[key] = value

    st["classes"] = len(classes)

    st["interfaces"] = sum(
        1 for cls in classes
        if cls["category"] == "interface"
    )

    st["abstract"] = sum(
        1 for cls in classes
        if cls["category"] == "abstract"
    )

    st["concrete"] = sum(
        1 for cls in classes
        if cls["category"] == "concrete"
    )

    st["methods"] = sum(
        cls["methods"]
        for cls in classes
    )

    st["pure_virtual"] = sum(
        cls["pure"]
        for cls in classes
    )

    st["override"] = sum(
        cls["override"]
        for cls in classes
    )

    st["member_definitions"] = len(
        MEMBER_DEFINITION_RE.findall(code)
    )

    st["enums"] = len(
        re.findall(
            r"\benum(?:\s+class)?\s+[A-Za-z_]\w*",
            code,
        )
    )

    st["templates"] = len(
        re.findall(
            r"\btemplate\s*<",
            code,
        )
    )

    st["includes"] = len(
        re.findall(
            r"(?m)^\s*#\s*include\b",
            code,
        )
    )

    st["todos"] = len(
        re.findall(
            r"\b(?:TODO|FIXME|XXX)\b",
            text,
            re.IGNORECASE,
        )
    )

    return st


directories = set()
files = []

for base, dirs, filenames in os.walk(
    SOURCE_ROOT,
    followlinks=False,
):

    base = Path(base)

    dirs[:] = sorted(
        d
        for d in dirs
        if not (base / d).is_symlink()
    )

    relative_dir = base.relative_to(SOURCE_ROOT).as_posix()

    if relative_dir == ".":
        relative_dir = ""

    directories.add(relative_dir)

    for filename in filenames:

        path = base / filename

        if path.is_symlink():
            continue

        if path.is_file() and is_cpp(path):
            files.append(path)


files.sort(key=lambda path: str(path).lower())

if not files:
    print(
        "[ERROR] Nenhum arquivo C++ encontrado.",
        file=sys.stderr,
    )
    sys.exit(1)


direct = defaultdict(stats)
recursive = defaultdict(stats)
total = stats()


for directory in directories:
    direct[directory]
    recursive[directory]


for path in files:

    st = analyze(path)

    add(total, st)

    parent = path.parent.relative_to(SOURCE_ROOT)
    parts = parent.parts

    direct_key = "/".join(parts)

    add(direct[direct_key], st)

    add(recursive[""], st)

    for level in range(1, len(parts) + 1):
        key = "/".join(parts[:level])
        add(recursive[key], st)


def decimal(value, places=2):
    return f"{value:.{places}f}".replace(".", ",")


def mean(num, den):
    if den == 0:
        return "0,00"
    return decimal(num / den)


def percent(num, den):
    if den == 0:
        return "0,00%"
    return decimal(100.0 * num / den) + "%"


def integer(value):
    return f"{value:,}".replace(",", ".")


# Consideramos como "pacotes/pastas de código" apenas diretórios
# que possuem diretamente pelo menos um arquivo C++.
code_directories = [
    key
    for key, st in direct.items()
    if st["files"] > 0
]


lines = []

lines.append("")
lines.append("=" * 132)
lines.append("GENESYS-SIMULATOR — ESTATÍSTICAS QUANTITATIVAS DO CÓDIGO-FONTE C++")
lines.append("=" * 132)
lines.append("")
lines.append(
    f"Diretório analisado: {SOURCE_ROOT.relative_to(REPO_ROOT)}"
)
lines.append("")


# ----------------------------------------------------------------------
# Por pasta: estatísticas dos arquivos pertencentes diretamente à pasta.
# ----------------------------------------------------------------------

headers = [
    "PASTA",
    "ARQ",
    "CLS",
    "IFC",
    "ABS",
    "CON",
    "MET",
    "LOC",
    "SLOC",
    "COM",
    "BRAN",
    "MET/CLS",
    "CLS/ARQ",
]

rows = []

for key in sorted(
    code_directories,
    key=lambda value: value.lower(),
):

    st = direct[key]

    folder = "." if key == "" else key

    rows.append([
        folder,
        str(st["files"]),
        str(st["classes"]),
        str(st["interfaces"]),
        str(st["abstract"]),
        str(st["concrete"]),
        str(st["methods"]),
        str(st["loc"]),
        str(st["sloc"]),
        str(st["comment_only"] + st["mixed"]),
        str(st["blank"]),
        mean(st["methods"], st["classes"]),
        mean(st["classes"], st["files"]),
    ])


widths = []

for column in range(len(headers)):

    widths.append(
        max(
            len(headers[column]),
            max(
                (
                    len(row[column])
                    for row in rows
                ),
                default=0,
            ),
        )
    )


def format_row(row):

    values = []

    for i, value in enumerate(row):

        if i == 0:
            values.append(
                value.ljust(widths[i])
            )
        else:
            values.append(
                value.rjust(widths[i])
            )

    return "  ".join(values)


lines.append("ESTATÍSTICAS POR PASTA")
lines.append("-" * 132)
lines.append(format_row(headers))
lines.append(
    "  ".join(
        "-" * width
        for width in widths
    )
)

for row in rows:
    lines.append(format_row(row))


# ----------------------------------------------------------------------
# Estatísticas recursivas apenas para grandes agrupamentos de primeiro
# nível, úteis para kernel/, plugins/, tools/, etc.
# ----------------------------------------------------------------------

top_level = sorted(
    {
        key.split("/", 1)[0]
        for key in recursive
        if key
    },
    key=str.lower,
)

lines.append("")
lines.append("")
lines.append("ESTATÍSTICAS ACUMULADAS POR SUBÁRVORE PRINCIPAL")
lines.append("-" * 132)

sub_headers = [
    "SUBÁRVORE",
    "ARQ",
    "CLS",
    "IFC",
    "ABS",
    "CON",
    "MET",
    "LOC",
    "SLOC",
    "MET/CLS",
    "SLOC/ARQ",
]

sub_rows = []

for key in top_level:

    st = recursive[key]

    if st["files"] == 0:
        continue

    sub_rows.append([
        key,
        str(st["files"]),
        str(st["classes"]),
        str(st["interfaces"]),
        str(st["abstract"]),
        str(st["concrete"]),
        str(st["methods"]),
        str(st["loc"]),
        str(st["sloc"]),
        mean(st["methods"], st["classes"]),
        mean(st["sloc"], st["files"]),
    ])


sub_widths = []

for column in range(len(sub_headers)):

    sub_widths.append(
        max(
            len(sub_headers[column]),
            max(
                (
                    len(row[column])
                    for row in sub_rows
                ),
                default=0,
            ),
        )
    )


def format_sub_row(row):

    result = []

    for i, value in enumerate(row):

        if i == 0:
            result.append(
                value.ljust(sub_widths[i])
            )
        else:
            result.append(
                value.rjust(sub_widths[i])
            )

    return "  ".join(result)


lines.append(format_sub_row(sub_headers))
lines.append(
    "  ".join(
        "-" * width
        for width in sub_widths
    )
)

for row in sub_rows:
    lines.append(format_sub_row(row))


# ----------------------------------------------------------------------
# Totais gerais
# ----------------------------------------------------------------------

comment_lines = (
    total["comment_only"]
    + total["mixed"]
)

nonblank = (
    total["loc"]
    - total["blank"]
)

total_packages = len(code_directories)


lines.append("")
lines.append("")
lines.append("=" * 132)
lines.append("TOTAL GERAL DO GENESYS-SIMULATOR")
lines.append("=" * 132)
lines.append("")

lines.append("TAMANHO DO CÓDIGO")
lines.append(
    f"  Pastas/pacotes com código C++................. {integer(total_packages)}"
)
lines.append(
    f"  Arquivos C++.................................. {integer(total['files'])}"
)
lines.append(
    f"    Arquivos de cabeçalho....................... {integer(total['headers'])}"
)
lines.append(
    f"    Arquivos de implementação................... {integer(total['implementations'])}"
)
lines.append(
    f"  Tamanho total dos fontes...................... {integer(total['bytes'])} bytes"
)
lines.append("")

lines.append("CLASSES")
lines.append(
    f"  Classes/structs............................... {integer(total['classes'])}"
)
lines.append(
    f"    Interfaces.................................. {integer(total['interfaces'])}"
)
lines.append(
    f"    Classes abstratas........................... {integer(total['abstract'])}"
)
lines.append(
    f"    Classes concretas........................... {integer(total['concrete'])}"
)
lines.append(
    f"  Interfaces / classes.......................... {percent(total['interfaces'], total['classes'])}"
)
lines.append(
    f"  Classes abstratas / classes................... {percent(total['abstract'], total['classes'])}"
)
lines.append(
    f"  Classes concretas / classes................... {percent(total['concrete'], total['classes'])}"
)
lines.append("")

lines.append("MÉTODOS")
lines.append(
    f"  Métodos declarados/inline..................... {integer(total['methods'])}"
)
lines.append(
    f"  Definições Class::method externas............. {integer(total['member_definitions'])}"
)
lines.append(
    f"  Métodos pure virtual.......................... {integer(total['pure_virtual'])}"
)
lines.append(
    f"  Métodos override.............................. {integer(total['override'])}"
)
lines.append("")

lines.append("LINHAS")
lines.append(
    f"  LOC físico total.............................. {integer(total['loc'])}"
)
lines.append(
    f"  SLOC: linhas contendo código.................. {integer(total['sloc'])}"
)
lines.append(
    f"  Linhas contendo comentários................... {integer(comment_lines)}"
)
lines.append(
    f"    Somente comentário.......................... {integer(total['comment_only'])}"
)
lines.append(
    f"    Código + comentário......................... {integer(total['mixed'])}"
)
lines.append(
    f"  Linhas em branco.............................. {integer(total['blank'])}"
)
lines.append(
    f"  Linhas não vazias............................. {integer(nonblank)}"
)
lines.append("")

lines.append("OUTRAS ESTRUTURAS")
lines.append(
    f"  Enums......................................... {integer(total['enums'])}"
)
lines.append(
    f"  Templates..................................... {integer(total['templates'])}"
)
lines.append(
    f"  Diretivas #include............................ {integer(total['includes'])}"
)
lines.append(
    f"  TODO/FIXME/XXX................................ {integer(total['todos'])}"
)
lines.append("")

lines.append("MÉDIAS POR CLASSE")
lines.append(
    f"  Métodos por classe............................ {mean(total['methods'], total['classes'])}"
)
lines.append(
    f"  SLOC por classe............................... {mean(total['sloc'], total['classes'])}"
)
lines.append(
    f"  LOC por classe................................ {mean(total['loc'], total['classes'])}"
)
lines.append("")

lines.append("MÉDIAS POR ARQUIVO")
lines.append(
    f"  Classes por arquivo........................... {mean(total['classes'], total['files'])}"
)
lines.append(
    f"  Métodos por arquivo........................... {mean(total['methods'], total['files'])}"
)
lines.append(
    f"  SLOC por arquivo.............................. {mean(total['sloc'], total['files'])}"
)
lines.append(
    f"  LOC por arquivo............................... {mean(total['loc'], total['files'])}"
)
lines.append("")

lines.append("MÉDIAS POR PASTA/PACOTE")
lines.append(
    f"  Arquivos por pasta............................ {mean(total['files'], total_packages)}"
)
lines.append(
    f"  Classes por pasta............................. {mean(total['classes'], total_packages)}"
)
lines.append(
    f"  Métodos por pasta............................. {mean(total['methods'], total_packages)}"
)
lines.append(
    f"  SLOC por pasta................................ {mean(total['sloc'], total_packages)}"
)
lines.append("")

lines.append("DENSIDADES")
lines.append(
    f"  Código / LOC.................................. {percent(total['sloc'], total['loc'])}"
)
lines.append(
    f"  Comentários / LOC............................. {percent(comment_lines, total['loc'])}"
)
lines.append(
    f"  Linhas em branco / LOC....................... {percent(total['blank'], total['loc'])}"
)
lines.append(
    f"  Comentários / linhas não vazias............... {percent(comment_lines, nonblank)}"
)
lines.append(
    f"  Métodos pure virtual / métodos................ {percent(total['pure_virtual'], total['methods'])}"
)
lines.append(
    f"  Métodos override / métodos.................... {percent(total['override'], total['methods'])}"
)

lines.append("")
lines.append("=" * 132)

output = "\n".join(lines) + "\n"

sys.stdout.write(output)

if SAVE_REPORT:

    expected = (
        REPO_ROOT
        / "build"
        / "contabiliza_codigofonte.dat"
    ).resolve()

    if SAVE_PATH.resolve() != expected:
        print(
            "[ERROR] Caminho de saída inválido.",
            file=sys.stderr,
        )
        sys.exit(1)

    with SAVE_PATH.open(
        "w",
        encoding="utf-8",
        newline="\n",
    ) as handle:
        handle.write(output)

__PYTHON__
