# AGENTS.md

Guidance for CLI coding agents working in this repository.

## Scope

- This repository contains a C++ solver core, tests, and an optional openFrameworks visualizer.
- Treat the solver as the primary product. The visualizer and experiment scripts are secondary and may have heavier or less portable dependencies.

## Repository map

- `main.cpp`: parallel solver entry point.
- `lacam/include`, `lacam/src`: core library code.
- `tests`: C++ test executables discovered by CMake.
- `visualizer`: optional openFrameworks-based viewer.
- `third_party`: vendored dependencies and submodules. Avoid editing unless the task is explicitly about dependency maintenance.

## Coordination rules for multiple agents

- Before editing, claim a narrow ownership area and avoid overlapping file edits with other agents.
- Use `dev` as the default integration branch for routine work unless the user explicitly asks to target another branch.
- Prefer splitting work by subsystem:
  - one agent for CLI and entry points
  - one agent for core library changes
- one agent for tests and CI
- one agent for docs
- If a task requires cross-cutting edits, sequence the work instead of editing the same files in parallel.
- When you touch shared interfaces, note the downstream files likely affected so another agent can verify them.

## Files to avoid by default

- Do not modify `third_party/**` unless the task is specifically about submodules or vendored patches.
- Treat `visualizer/**` as optional. Avoid pulling it into core build changes unless necessary.

## Build and test expectations

- Configure from the repository root:
  ```sh
  cmake -S . -B build
  ```
- Build:
  ```sh
  cmake --build build -j4
  ```
- Run tests:
  ```sh
  ctest --test-dir build --output-on-failure
  ```
- If configuration fails because submodules are missing, initialize:
  ```sh
  git submodule update --init --recursive third_party/argparse third_party/cnpy
  ```
- The visualizer has separate setup and should not be required for core validation.

## Editing conventions

- Keep changes minimal and localized.
- Prefer the smallest implementation that solves the task clearly. Preserve simplicity over premature abstraction or feature growth.
- Preserve the current CMake structure unless the task is to refactor build logic.
- Prefer reusable helpers over duplicating logic across CLI entry points and tools.
- Do not silently change solver behavior. If behavior changes, document the reason in the commit or summary.
- Keep ASCII unless a file already requires non-ASCII text.

## Validation guidance

- After code changes, verify that the project still compiles before finishing the task.
- For C++ changes, at minimum run the relevant build target or `ctest` when possible.
- For documentation-only changes, verify command examples against the current repository structure.
- For helper tooling changes, prefer making paths configurable rather than baking in local machine assumptions.

## Known repository risks

- The solver ships a single CLI entry point in `main.cpp`; update scripts and docs if its interface changes.
- The working tree may contain local submodule modifications. Inspect before attempting dependency cleanup.

## Good handoff notes

When finishing, report:

- which files you changed
- what behavior changed, if any
- what you verified locally
- what remains unverified because of missing dependencies or environment constraints
