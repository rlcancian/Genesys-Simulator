#!/usr/bin/env python3
"""Render figure assets from FIGURE-SPEC blocks.

The manual source keeps a structural spec comment beside each figure placeholder.
This script turns those specs into compact vector PDFs under figs/generated/.
"""

from __future__ import annotations

import math
import re
import textwrap
from collections import defaultdict
from pathlib import Path

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib import patches
from matplotlib.backends.backend_pdf import PdfPages


ROOT = Path(__file__).resolve().parent
FIG_ROOT = ROOT / "figs" / "generated"


FIGURE_ENV_RE = re.compile(r"\\begin\{figure\}.*?\\end\{figure\}", re.S)
PLACEHOLDER_RE = re.compile(r"\\figureplaceholder\{([^}]+)\}")
CAPTION_RE = re.compile(r"\\caption\{([^}]*)\}", re.S)
LABEL_RE = re.compile(r"\\label\{([^}]*)\}")
FIELD_RE = re.compile(r"^([A-Za-z][A-Za-z0-9 /()&,-]+):\s*(.*)$")


def normalize_key(key: str) -> str:
    key = key.strip().lower()
    key = key.replace(" ", "_")
    key = key.replace("/", "_")
    key = key.replace("-", "_")
    key = key.replace("(", "")
    key = key.replace(")", "")
    key = key.replace("&", "and")
    key = re.sub(r"__+", "_", key)
    return key


def strip_comment(line: str) -> str:
    line = line.lstrip()
    if line.startswith("%"):
        line = line[1:]
    return line.lstrip()


def parse_spec(block: str) -> dict[str, list[str] | str]:
    fields: dict[str, list[str]] = defaultdict(list)
    current: str | None = None
    for raw in block.splitlines():
        line = strip_comment(raw).rstrip()
        if not line:
            continue
        if line in {"FIGURE-SPEC-BEGIN", "FIGURE-SPEC-END"}:
            continue
        match = FIELD_RE.match(line)
        if match:
            current = normalize_key(match.group(1))
            value = match.group(2).strip()
            if value:
                fields[current].append(value)
            continue
        if current is None:
            continue
        if line.startswith("- "):
            fields[current].append(line[2:].strip())
        elif line.startswith("* "):
            fields[current].append(line[2:].strip())
        else:
            fields[current].append(line.strip())
    return {key: value if len(value) > 1 else value[0] for key, value in fields.items()}


def collapse(value: object) -> list[str]:
    if not value:
        return []
    if isinstance(value, list):
        return [str(v).strip() for v in value if str(v).strip()]
    return [str(value).strip()] if str(value).strip() else []


def first_line(value: object, fallback: str = "") -> str:
    items = collapse(value)
    return items[0] if items else fallback


def humanize_identifier(identifier: str) -> str:
    identifier = identifier.replace("fig:", "")
    identifier = identifier.replace(":", " ")
    identifier = identifier.replace("-", " ")
    identifier = identifier.replace("_", " ")
    identifier = re.sub(r"\s+", " ", identifier).strip()
    return identifier.title()


def wrap_lines(text: str, width: int = 42) -> list[str]:
    if not text:
        return []
    paragraphs = []
    for raw_paragraph in text.split("\n"):
        paragraph = raw_paragraph.strip()
        if not paragraph:
            paragraphs.append("")
            continue
        paragraphs.extend(textwrap.wrap(paragraph, width=width, break_long_words=False, break_on_hyphens=False))
    return paragraphs


def draw_text_block(ax, x: float, y: float, w: float, title: str, lines: list[str], color: str) -> float:
    line_height = 0.026
    header_h = 0.04
    content_h = max(0.08, header_h + max(1, len(lines)) * line_height + 0.02)
    y0 = y - content_h
    box = patches.FancyBboxPatch(
        (x, y0),
        w,
        content_h,
        boxstyle="round,pad=0.008,rounding_size=0.012",
        linewidth=1.0,
        edgecolor=color,
        facecolor="#ffffff",
    )
    ax.add_patch(box)
    ax.text(x + 0.015, y - 0.018, title, fontsize=9.2, fontweight="bold", color=color, va="top")
    text_y = y - header_h
    for line in lines[:12]:
        ax.text(x + 0.015, text_y, f"- {line}" if line else "", fontsize=7.0, color="#222222", va="top")
        text_y -= line_height
    if len(lines) > 12:
        ax.text(x + 0.015, text_y, f"- ... ({len(lines) - 12} more)", fontsize=7.0, color="#666666", va="top")
    return y0 - 0.02


def draw_arrow(ax, x1: float, y1: float, x2: float, y2: float, color: str = "#334155") -> None:
    arrow = patches.FancyArrowPatch(
        (x1, y1),
        (x2, y2),
        arrowstyle="-|>",
        mutation_scale=12,
        linewidth=1.2,
        color=color,
    )
    ax.add_patch(arrow)


def short_label(text: str, width: int = 18) -> str:
    text = re.sub(r"\s+", " ", text).strip()
    if len(text) <= width:
        return text
    return textwrap.shorten(text, width=width, placeholder="...")


def draw_flow(ax, title: str, items: list[str], accent: str) -> None:
    items = items or [title]
    n = len(items)
    x_start, x_end = 0.08, 0.92
    usable = x_end - x_start
    box_w = min(0.16, usable / max(n, 1) - 0.02)
    box_h = 0.12
    if n > 5:
        box_w = min(box_w, 0.13)
    y = 0.55
    gap = 0.04 if n < 6 else 0.02
    total_w = n * box_w + (n - 1) * gap
    x = 0.5 - total_w / 2
    for idx, item in enumerate(items):
        rect = patches.FancyBboxPatch(
            (x, y - box_h / 2),
            box_w,
            box_h,
            boxstyle="round,pad=0.01,rounding_size=0.02",
            linewidth=1.3,
            edgecolor=accent,
            facecolor="#f8fafc",
        )
        ax.add_patch(rect)
        ax.text(x + box_w / 2, y, short_label(item, 20), ha="center", va="center", fontsize=8.0)
        if idx < n - 1:
            draw_arrow(ax, x + box_w, y, x + box_w + gap, y, accent)
        x += box_w + gap
    ax.text(0.5, 0.80, "Process / flow", ha="center", va="center", fontsize=10, fontweight="bold", color=accent)


def draw_tree(ax, title: str, items: list[str], accent: str) -> None:
    items = items or [title]
    root = patches.FancyBboxPatch(
        (0.34, 0.67),
        0.32,
        0.14,
        boxstyle="round,pad=0.01,rounding_size=0.02",
        linewidth=1.3,
        edgecolor=accent,
        facecolor="#f8fafc",
    )
    ax.add_patch(root)
    ax.text(0.5, 0.74, short_label(title, 26), ha="center", va="center", fontsize=9.2, fontweight="bold")
    count = min(len(items), 6)
    xs = [0.08 + i * 0.82 / max(count - 1, 1) for i in range(count)]
    for idx, item in enumerate(items[:count]):
        bx = xs[idx] - 0.07
        by = 0.22
        rect = patches.FancyBboxPatch(
            (bx, by),
            0.14,
            0.12,
            boxstyle="round,pad=0.01,rounding_size=0.015",
            linewidth=1.1,
            edgecolor="#64748b",
            facecolor="#ffffff",
        )
        ax.add_patch(rect)
        ax.text(xs[idx], by + 0.06, short_label(item, 18), ha="center", va="center", fontsize=7.4)
        draw_arrow(ax, 0.5, 0.67, xs[idx], by + 0.12, accent)
    ax.text(0.5, 0.91, "Tree / family map", ha="center", va="center", fontsize=10, fontweight="bold", color=accent)


def draw_ladder(ax, title: str, items: list[str], accent: str) -> None:
    steps = items or [title]
    step_h = 0.10
    start_y = 0.20
    x0 = 0.12
    widths = [0.18 + i * 0.045 for i in range(min(len(steps), 6))]
    widths += [widths[-1]] * max(0, len(steps) - len(widths))
    for idx, item in enumerate(steps[:6]):
        w = widths[idx]
        x = x0 + idx * 0.08
        y = start_y + idx * 0.11
        rect = patches.FancyBboxPatch(
            (x, y),
            w,
            step_h,
            boxstyle="round,pad=0.01,rounding_size=0.01",
            linewidth=1.2,
            edgecolor=accent,
            facecolor="#f8fafc" if idx % 2 == 0 else "#eef2ff",
        )
        ax.add_patch(rect)
        ax.text(x + 0.012, y + step_h / 2, short_label(item, 22), ha="left", va="center", fontsize=7.5)
    ax.text(0.5, 0.82, "Evidence ladder", ha="center", va="center", fontsize=10, fontweight="bold", color=accent)


def draw_hub(ax, title: str, items: list[str], accent: str) -> None:
    center = patches.FancyBboxPatch(
        (0.38, 0.42),
        0.24,
        0.16,
        boxstyle="round,pad=0.01,rounding_size=0.02",
        linewidth=1.4,
        edgecolor=accent,
        facecolor="#f8fafc",
    )
    ax.add_patch(center)
    ax.text(0.5, 0.50, short_label(title, 24), ha="center", va="center", fontsize=8.6, fontweight="bold")
    slots = [
        (0.16, 0.70),
        (0.72, 0.70),
        (0.16, 0.22),
        (0.72, 0.22),
    ]
    for idx, item in enumerate(items[:4]):
        x, y = slots[idx]
        rect = patches.FancyBboxPatch(
            (x, y),
            0.18,
            0.12,
            boxstyle="round,pad=0.01,rounding_size=0.015",
            linewidth=1.0,
            edgecolor="#64748b",
            facecolor="#ffffff",
        )
        ax.add_patch(rect)
        ax.text(x + 0.09, y + 0.06, short_label(item, 18), ha="center", va="center", fontsize=7.4)
        draw_arrow(ax, 0.5, 0.42 if y < 0.4 else 0.58, x + 0.09, y + (0.12 if y < 0.4 else 0.0), accent)
    ax.text(0.5, 0.86, "Hub / overview", ha="center", va="center", fontsize=10, fontweight="bold", color=accent)


def draw_matrix(ax, title: str, items: list[str], accent: str) -> None:
    rows = items or [title]
    col1_x = 0.10
    col2_x = 0.48
    col_w = 0.34
    row_h = 0.11
    y = 0.75
    ax.text(0.5, 0.90, "Matrix / catalog", ha="center", va="center", fontsize=10, fontweight="bold", color=accent)
    headers = [("Item", col1_x), ("Note", col2_x)]
    for text, x in headers:
        ax.text(x + 0.02, y + row_h, text, fontsize=8.5, fontweight="bold", color=accent)
    for idx, item in enumerate(rows[:5]):
        yy = y - idx * 0.14
        for x in (col1_x, col2_x):
            rect = patches.Rectangle((x, yy - row_h), col_w, row_h, linewidth=1.0, edgecolor="#94a3b8", facecolor="#ffffff")
            ax.add_patch(rect)
        ax.text(col1_x + 0.02, yy - row_h / 2, short_label(item, 28), fontsize=7.4, va="center")
        ax.text(col2_x + 0.02, yy - row_h / 2, "See source spec", fontsize=7.2, va="center", color="#475569")


def render_diagram(ax, spec: dict[str, list[str] | str], caption: str, fig_id: str) -> None:
    type_text = first_line(spec.get("type"), "")
    lower = f"{caption} {fig_id} {type_text}".lower()
    required = collapse(spec.get("required_visual_content"))
    accent = "#0f766e"
    template = "hub"
    if any(k in lower for k in ["flow", "process", "pipeline", "sequence", "workflow"]):
        template = "flow"
        accent = "#1d4ed8"
    elif any(k in lower for k in ["tree", "family"]):
        template = "tree"
        accent = "#7c3aed"
    elif any(k in lower for k in ["ladder", "evidence"]):
        template = "ladder"
        accent = "#b45309"
    elif any(k in lower for k in ["matrix", "table", "catalog"]):
        template = "matrix"
        accent = "#be123c"
    elif any(k in lower for k in ["state", "cycle", "lifecycle"]):
        template = "flow"
        accent = "#0f766e"

    ax.set_xlim(0, 1)
    ax.set_ylim(0, 1)
    ax.axis("off")

    background = patches.FancyBboxPatch(
        (0.02, 0.04),
        0.96,
        0.92,
        boxstyle="round,pad=0.01,rounding_size=0.02",
        linewidth=1.2,
        edgecolor=accent,
        facecolor="#f8fafc",
    )
    ax.add_patch(background)

    if template == "flow":
        draw_flow(ax, caption, required[:6], accent)
    elif template == "tree":
        draw_tree(ax, caption, required[:6], accent)
    elif template == "ladder":
        draw_ladder(ax, caption, required[:6], accent)
    elif template == "matrix":
        draw_matrix(ax, caption, required[:6], accent)
    else:
        draw_hub(ax, caption, required[:6], accent)


def render_page(data: dict[str, object], out_path: Path) -> None:
    caption = str(data.get("caption", "")).strip() or humanize_identifier(str(data.get("id", out_path.stem)))
    fig_id = str(data.get("id", out_path.stem))
    spec = data.get("spec", {})
    assert isinstance(spec, dict)

    fig = plt.figure(figsize=(12.5, 8.3))
    gs = fig.add_gridspec(
        2,
        2,
        height_ratios=[0.14, 0.86],
        width_ratios=[0.58, 0.42],
        left=0.03,
        right=0.97,
        top=0.97,
        bottom=0.04,
        wspace=0.03,
        hspace=0.03,
    )
    ax_head = fig.add_subplot(gs[0, :])
    ax_left = fig.add_subplot(gs[1, 0])
    ax_right = fig.add_subplot(gs[1, 1])

    type_text = first_line(spec.get("type"), "figure")
    accent = "#0f766e"
    if any(k in (caption + " " + type_text + " " + fig_id).lower() for k in ["flow", "process", "pipeline", "sequence", "workflow"]):
        accent = "#1d4ed8"
    elif any(k in (caption + " " + type_text + " " + fig_id).lower() for k in ["tree", "family"]):
        accent = "#7c3aed"
    elif any(k in (caption + " " + type_text + " " + fig_id).lower() for k in ["ladder", "evidence"]):
        accent = "#b45309"
    elif any(k in (caption + " " + type_text + " " + fig_id).lower() for k in ["matrix", "table", "catalog"]):
        accent = "#be123c"

    ax_head.set_xlim(0, 1)
    ax_head.set_ylim(0, 1)
    ax_head.axis("off")
    header = patches.FancyBboxPatch(
        (0.0, 0.05),
        1.0,
        0.9,
        boxstyle="round,pad=0.01,rounding_size=0.02",
        linewidth=0,
        facecolor=accent,
    )
    ax_head.add_patch(header)
    ax_head.text(0.02, 0.68, caption, fontsize=16, fontweight="bold", color="white", va="center", ha="left")
    ax_head.text(0.02, 0.28, f"{fig_id}   |   {type_text}", fontsize=9.5, color="#e2e8f0", va="center", ha="left")
    ax_head.text(0.98, 0.28, "Generated from FIGURE-SPEC comments", fontsize=9.0, color="#e2e8f0", va="center", ha="right")

    render_diagram(ax_left, spec, caption, fig_id)

    ax_right.set_xlim(0, 1)
    ax_right.set_ylim(0, 1)
    ax_right.axis("off")
    y = 0.96

    sections: list[tuple[str, list[str]]] = []
    source = collapse(spec.get("source_of_truth") or spec.get("source_asset"))
    purpose = wrap_lines(first_line(spec.get("purpose"), ""), 42)
    required = collapse(spec.get("required_visual_content"))
    relationships = collapse(spec.get("relationships"))
    exclusions = collapse(spec.get("exclusions"))
    accessibility = wrap_lines(first_line(spec.get("accessibility_description") or spec.get("accessibility"), ""), 42)
    validation = collapse(spec.get("validation"))
    if source:
        sections.append(("Source", source))
    if purpose:
        sections.append(("Purpose", purpose))
    if required:
        sections.append(("Required content", required))
    if relationships:
        sections.append(("Relationships", relationships))
    if exclusions:
        sections.append(("Exclusions", exclusions))
    if accessibility:
        sections.append(("Accessibility", accessibility))
    if validation:
        sections.append(("Validation", validation))

    for title, lines in sections:
        y = draw_text_block(ax_right, 0.04, y, 0.92, title, lines, accent)
        if y < 0.12:
            break

    ax_right.text(
        0.04,
        0.05,
        "The source block remains the authoritative specification.\nThe rendered asset is a compact visual summary.",
        fontsize=7.0,
        color="#475569",
        va="bottom",
        ha="left",
    )

    FIG_ROOT.mkdir(parents=True, exist_ok=True)
    fig.savefig(out_path, format="pdf")
    plt.close(fig)


def parse_manual() -> list[dict[str, object]]:
    entries: list[dict[str, object]] = []
    for tex_path in sorted(ROOT.glob("*.tex")):
        text = tex_path.read_text(encoding="utf-8")
        for block in FIGURE_ENV_RE.findall(text):
            placeholder = PLACEHOLDER_RE.search(block)
            if not placeholder:
                continue
            placeholder_path = placeholder.group(1).strip()
            caption_match = CAPTION_RE.search(block)
            label_match = LABEL_RE.search(block)

            spec_match = re.search(r"% FIGURE-SPEC-BEGIN(.*?)% FIGURE-SPEC-END", block, re.S)
            spec = parse_spec(spec_match.group(1) if spec_match else "")
            fig_id = first_line(spec.get("id"), "")
            if not fig_id and label_match:
                fig_id = label_match.group(1)

            entries.append(
                {
                    "tex_path": tex_path,
                    "placeholder": placeholder_path,
                    "caption": caption_match.group(1).strip() if caption_match else "",
                    "label": label_match.group(1).strip() if label_match else "",
                    "id": fig_id,
                    "spec": spec,
                }
            )
    return entries


def main() -> int:
    entries = parse_manual()
    if not entries:
        print("No figure placeholders found.")
        return 0

    for entry in entries:
        placeholder = str(entry["placeholder"])
        relative = placeholder.replace("figs/generated/", "")
        out_path = FIG_ROOT / f"{relative}.pdf"
        out_path.parent.mkdir(parents=True, exist_ok=True)
        render_page(entry, out_path)
        print(f"Rendered {out_path.relative_to(ROOT)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
