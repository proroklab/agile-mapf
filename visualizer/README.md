# Visualizer

This directory contains an optional openFrameworks-based viewer for solver output.

## Scope

- The visualizer is not required to build or use the core solver.
- The core solver CMake build does not depend on openFrameworks.
- This setup is intended only for users who specifically want visualization.
- The documented setup in this repository is macOS-only.
- The repository-root `./viz` wrapper is also macOS-only because it expects a `.app` bundle layout.
- Linux and other platforms may be possible through openFrameworks, but they are not documented or regularly tested here.

## Requirements

- `third_party/openFrameworks` initialized
- Platform-specific openFrameworks prerequisites
- A successful core solver run that produced a result file such as `build/result.txt`

Initialize the visualizer dependency only when needed:

```sh
git submodule update --init --recursive third_party/openFrameworks
```

## macOS

The steps below are the only supported flow documented in this repository.

Download the openFrameworks libraries:

```sh
bash third_party/openFrameworks/scripts/osx/download_libs.sh
```

Build the app:

```sh
cd visualizer
make -j4
cd ..
```

Run the wrapper script from the repository root:

```sh
./viz build/result.txt
```

## Notes

- The visualizer has its own build system under `visualizer/Makefile`.
- `./viz` should be treated as a macOS convenience wrapper, not a cross-platform launcher.
- Failures in visualizer setup should not block validation of the core solver.
- If you only need solver outputs, ignore this directory and the openFrameworks submodule entirely.
