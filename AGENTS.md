# AGENTS.md

This file defines collaboration rules for multiple Codex agents working in parallel on this repository.

## 1. Scope
- Repository: `project-page`
- Primary goal: build and maintain the research project page.
- Conversation language can be Japanese, but all page content and commit messages must be English.

## 2. Source of Truth
- Page configuration (single file): `src/content/site.yaml`
- Rendering entrypoint: `src/pages/index.astro`
- Styling: `src/styles/global.css`

## 3. Parallel Work Model
- One agent = one focused task (no mixed-purpose edits in the same commit).
- Prefer non-overlapping file ownership during active work.
- If overlap is unavoidable, coordinate by declaring a temporary lock in the task comment or PR description.

## 4. File Ownership (Default)
- Agent A (Content): `src/content/**`, `README.md`
- Agent B (UI): `src/components/**`, `src/styles/**`
- Agent C (Infra): `.github/**`, `astro.config.mjs`, `package.json`, tooling/config files
- Shared with caution: `src/pages/index.astro`

## 5. Locking Convention
- Before large edits, announce:
  - target files
  - expected duration
  - intent
- Suggested lock format:
  - `LOCK: src/styles/global.css (Agent B, ~30m, typography refactor)`
- Remove lock note after merge/push.

## 6. Branch Strategy
- Never work directly on `main`.
- Use short-lived branches:
  - `feat/<topic>`
  - `fix/<topic>`
  - `chore/<topic>`
- Rebase on latest `main` before opening PR.

## 7. Commit Rules
- Commit messages must be English.
- Use conventional style:
  - `feat: ...`
  - `fix: ...`
  - `chore: ...`
  - `docs: ...`
  - `refactor: ...`
- Keep each commit logically atomic.

## 8. Merge Conflict Minimization
- Keep changes minimal and local to task scope.
- Avoid broad formatting-only changes touching many files.
- Do not reorder unrelated YAML keys.
- Preserve existing content unless task explicitly requires replacement.

## 9. Validation Before Handoff
- Run:
  - `npm run build`
- If build cannot run, clearly report why and what was not validated.

## 10. Handoff Template
- Summary:
  - what changed
  - why
- Files:
  - absolute paths of changed files
- Validation:
  - commands run and result
- Follow-ups:
  - open risks / TODOs

## 11. Forbidden Actions
- Do not commit `assets/manuscript.pdf` or `assets/latex-src/**`.
- Do not force-push to shared branches without explicit agreement.
- Do not rewrite or revert another agent's changes without coordination.

## 12. Quick Start for New Agent
1. Read `README.md` and this file.
2. Pull latest `main`.
3. Pick a scoped task and declare lock.
4. Implement with minimal file touch.
5. Run `npm run build`.
6. Commit with English message and open PR.
