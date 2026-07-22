#!/usr/bin/env python3
"""Validate the governed docs/ai_assistants documentation structure."""

from __future__ import annotations

import argparse
import datetime as dt
import re
import subprocess
import sys
from pathlib import Path
from urllib.parse import unquote, urlsplit

REPO_ROOT = Path(__file__).resolve().parents[1]
AI_ROOT = REPO_ROOT / "docs" / "ai_assistants"
OLDIES_ROOT = AI_ROOT / "oldies"

TOP_LEVEL_ALLOWLIST = {
    "README.md",
    "GOVERNANCE.md",
    "ARCHITECTURE.md",
    "STATUS.md",
    "BACKLOG_AUTONOMOUS.md",
    "BACKLOG_HUMAN.md",
}

REQUIRED_PATHS = {
    AI_ROOT / "README.md",
    AI_ROOT / "GOVERNANCE.md",
    AI_ROOT / "ARCHITECTURE.md",
    AI_ROOT / "STATUS.md",
    AI_ROOT / "BACKLOG_AUTONOMOUS.md",
    AI_ROOT / "BACKLOG_HUMAN.md",
    AI_ROOT / "runbooks" / "AUTONOMOUS_AGENT.md",
    AI_ROOT / "runbooks" / "LOCAL_AGENT.md",
    AI_ROOT / "runbooks" / "GITHUB_AGENT.md",
    AI_ROOT / "reference" / "README.md",
    AI_ROOT / "reference" / "BUILD_TEST_PACKAGING.md",
    AI_ROOT / "reference" / "KERNEL_PARSER_OWNERSHIP.md",
    AI_ROOT / "reference" / "PLUGINS.md",
    AI_ROOT / "reference" / "SCIENTIFIC_DOMAINS.md",
    AI_ROOT / "reference" / "APPLICATIONS_TOOLS_MODELS.md",
    AI_ROOT / "reference" / "API_INTEGRATIONS.md",
    AI_ROOT / "history" / "CHANGELOG_AI.md",
    AI_ROOT / "history" / "evidence" / "README.md",
    AI_ROOT / "history" / "evidence" / "2026" / "07" / "VALIDATION_LEDGER.md",
    AI_ROOT / "archive" / "README.md",
    AI_ROOT / "archive" / "OLDIES_REVIEW.md",
}

REQUIRED_FRONT_MATTER_KEYS = {"document_type", "authority", "owner", "status"}
BACKLOG_FILES = [AI_ROOT / "BACKLOG_AUTONOMOUS.md", AI_ROOT / "BACKLOG_HUMAN.md"]
BACKLOG_ID_RE = re.compile(r"^###\s+([A-Z]+-[A-Z]+-\d{3})\b", re.MULTILINE)
MARKDOWN_LINK_RE = re.compile(r"!?\[[^\]]*\]\(([^)]+)\)")
FENCED_BLOCK_RE = re.compile(r"```.*?```", re.DOTALL)
DATED_EVIDENCE_RE = re.compile(r".*_evidence_\d{8}\.md$")
RUN_ID_RE = re.compile(r"(?<!\d)\d{11}(?!\d)")
OLDIES_GATE = dt.date(2026, 11, 1)


def rel(path: Path) -> str:
    return path.relative_to(REPO_ROOT).as_posix()


def read_text(path: Path) -> str:
    return path.read_text(encoding="utf-8")


def parse_front_matter(path: Path, errors: list[str]) -> dict[str, str]:
    lines = read_text(path).splitlines()
    if not lines or lines[0].strip() != "---":
        errors.append(f"{rel(path)}: missing YAML front matter")
        return {}

    try:
        end = next(i for i, line in enumerate(lines[1:], start=1) if line.strip() == "---")
    except StopIteration:
        errors.append(f"{rel(path)}: unterminated YAML front matter")
        return {}

    metadata: dict[str, str] = {}
    for line in lines[1:end]:
        if not line.strip() or line.lstrip().startswith("#") or ":" not in line:
            continue
        key, value = line.split(":", 1)
        metadata[key.strip()] = value.strip()

    missing = sorted(REQUIRED_FRONT_MATTER_KEYS - metadata.keys())
    if missing:
        errors.append(f"{rel(path)}: missing front-matter keys: {', '.join(missing)}")
    return metadata


def validate_top_level(errors: list[str]) -> None:
    actual = {path.name for path in AI_ROOT.glob("*.md")}
    missing = sorted(TOP_LEVEL_ALLOWLIST - actual)
    unexpected = sorted(actual - TOP_LEVEL_ALLOWLIST)
    if missing:
        errors.append(f"top-level canonical Markdown missing: {', '.join(missing)}")
    if unexpected:
        errors.append(f"unexpected top-level Markdown: {', '.join(unexpected)}")

    if (AI_ROOT / "plugins").exists():
        errors.append("legacy docs/ai_assistants/plugins/ directory must not exist")

    for removed in ("oldies_inventory.md", "oldies_review_status.md", "consolidation_map.md"):
        if (AI_ROOT / removed).exists():
            errors.append(f"superseded tracker still exists: docs/ai_assistants/{removed}")


def validate_required_paths(errors: list[str]) -> None:
    for path in sorted(REQUIRED_PATHS):
        if not path.is_file():
            errors.append(f"required documentation file missing: {rel(path)}")


def active_markdown_files() -> list[Path]:
    return sorted(
        path
        for path in AI_ROOT.rglob("*.md")
        if OLDIES_ROOT not in path.parents
    )


def validate_front_matter(errors: list[str]) -> None:
    for path in active_markdown_files():
        parse_front_matter(path, errors)


def normalize_link_target(raw_target: str) -> str:
    target = raw_target.strip()
    if target.startswith("<") and ">" in target:
        return target[1 : target.index(">")]
    if " " in target:
        target = target.split(None, 1)[0]
    return target


def validate_links(errors: list[str]) -> None:
    paths = [REPO_ROOT / "README.md", *active_markdown_files()]
    for path in paths:
        text = FENCED_BLOCK_RE.sub("", read_text(path))
        for match in MARKDOWN_LINK_RE.finditer(text):
            raw = normalize_link_target(match.group(1))
            if not raw or raw.startswith("#"):
                continue

            parsed = urlsplit(raw)
            if parsed.scheme or raw.startswith("//"):
                continue

            target_text = unquote(parsed.path)
            if not target_text:
                continue

            if target_text.startswith("/"):
                target = REPO_ROOT / target_text.lstrip("/")
            else:
                target = path.parent / target_text

            target = target.resolve()
            try:
                target.relative_to(REPO_ROOT)
            except ValueError:
                errors.append(f"{rel(path)}: relative link escapes repository: {raw}")
                continue

            if OLDIES_ROOT in target.parents or target == OLDIES_ROOT:
                errors.append(f"{rel(path)}: active documentation must not depend on oldies: {raw}")
            elif not target.exists():
                errors.append(f"{rel(path)}: broken relative link: {raw}")


def validate_backlogs(errors: list[str]) -> None:
    seen: dict[str, str] = {}
    for path in BACKLOG_FILES:
        text = read_text(path)
        matches = list(BACKLOG_ID_RE.finditer(text))
        for index, match in enumerate(matches):
            task_id = match.group(1)
            section_end = matches[index + 1].start() if index + 1 < len(matches) else len(text)
            section = text[match.start() : section_end]

            if task_id in seen:
                errors.append(f"duplicate backlog ID {task_id}: {seen[task_id]} and {rel(path)}")
            else:
                seen[task_id] = rel(path)

            for required_field in ("- Priority:", "- Status:"):
                if required_field not in section:
                    errors.append(f"{rel(path)}: {task_id} missing required field {required_field}")

    if not seen:
        errors.append("no backlog IDs found")


def validate_normative_documents(errors: list[str]) -> None:
    for name in ("GOVERNANCE.md", "ARCHITECTURE.md"):
        path = AI_ROOT / name
        matches = RUN_ID_RE.findall(read_text(path))
        if matches:
            errors.append(f"{rel(path)}: normative document contains 11-digit workflow/run IDs: {', '.join(sorted(set(matches)))}")


def validate_evidence_placement(errors: list[str]) -> None:
    evidence_root = AI_ROOT / "history" / "evidence"
    for path in AI_ROOT.rglob("*.md"):
        if DATED_EVIDENCE_RE.fullmatch(path.name) and evidence_root not in path.parents:
            errors.append(f"dated evidence file outside history/evidence: {rel(path)}")


def validate_oldies_tracker(errors: list[str]) -> None:
    tracker = AI_ROOT / "archive" / "OLDIES_REVIEW.md"
    oldies_files = sorted(path.name for path in OLDIES_ROOT.glob("*.md"))
    if len(oldies_files) != 25:
        errors.append(f"expected 25 retained oldies Markdown files, found {len(oldies_files)}")

    tracker_text = read_text(tracker)
    tracked = sorted(set(re.findall(r"`(old_[^`]+\.md)`", tracker_text)))
    if tracked != oldies_files:
        missing = sorted(set(oldies_files) - set(tracked))
        extra = sorted(set(tracked) - set(oldies_files))
        if missing:
            errors.append(f"oldies tracker missing files: {', '.join(missing)}")
        if extra:
            errors.append(f"oldies tracker references absent files: {', '.join(extra)}")

    classified_rows = re.findall(
        r"^\|\s*\d+\s*\|.*\|\s*`retained-review-pending`\s*\|\s*no\s*\|$",
        tracker_text,
        flags=re.MULTILINE,
    )
    if len(classified_rows) != 25:
        errors.append(f"expected 25 retained-review-pending/not-ready rows, found {len(classified_rows)}")

    if "retention_gate: 2026-11-01" not in tracker_text or "2026-11-01" not in tracker_text:
        errors.append("oldies tracker does not preserve the 2026-11-01 retention gate")


def validate_oldies_git_diff(base_ref: str | None, errors: list[str]) -> None:
    if not base_ref:
        return

    command = [
        "git",
        "diff",
        "--name-status",
        f"{base_ref}...HEAD",
        "--",
        "docs/ai_assistants/oldies",
    ]
    result = subprocess.run(command, cwd=REPO_ROOT, text=True, capture_output=True, check=False)
    if result.returncode != 0:
        errors.append(f"unable to inspect oldies diff against {base_ref}: {result.stderr.strip()}")
        return

    today = dt.datetime.now(dt.timezone.utc).date()
    allow_deletion = bool(int(__import__("os").environ.get("GENESYS_ALLOW_OLDIES_DELETION", "0")))
    for line in result.stdout.splitlines():
        fields = line.split("\t")
        status = fields[0]
        if status.startswith("D") or status.startswith("R"):
            if today <= OLDIES_GATE and not allow_deletion:
                errors.append(
                    f"oldies deletion/rename before retention gate {OLDIES_GATE.isoformat()}: {line}"
                )


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--base-ref", help="base commit/ref used to inspect oldies deletions")
    args = parser.parse_args()

    errors: list[str] = []
    if not AI_ROOT.is_dir():
        print("ERROR: docs/ai_assistants directory is missing", file=sys.stderr)
        return 1

    validate_top_level(errors)
    validate_required_paths(errors)
    validate_front_matter(errors)
    validate_links(errors)
    validate_backlogs(errors)
    validate_normative_documents(errors)
    validate_evidence_placement(errors)
    validate_oldies_tracker(errors)
    validate_oldies_git_diff(args.base_ref, errors)

    if errors:
        print(f"AI documentation governance validation failed with {len(errors)} error(s):", file=sys.stderr)
        for error in errors:
            print(f"- {error}", file=sys.stderr)
        return 1

    print("AI documentation governance validation passed.")
    print(f"- top-level Markdown: {len(TOP_LEVEL_ALLOWLIST)} canonical files")
    print(f"- active Markdown checked: {len(active_markdown_files())}")
    print("- retained oldies checked: 25 files, none deletion-ready")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
