# Third-Party Dependencies

This directory contains git submodules pinned to specific upstream commits.

## Policy

- `third_party/argparse`, `third_party/cnpy`, and `third_party/openFrameworks` are tracked as upstream submodules.
- The repository currently carries no local source patches inside these submodules.
- If a third-party change is ever required, record it explicitly in the main repository instead of leaving an unexplained dirty submodule worktree.

## Expected state

From the repository root, the third-party dependencies should be reproducible with:

```sh
git submodule update --init --recursive
```

To inspect the current pinned revisions:

```sh
git submodule status --recursive
```

To verify that the submodule worktrees are clean:

```sh
git -C third_party/argparse status --short
git -C third_party/cnpy status --short
git -C third_party/openFrameworks status --short
```

Each command above should produce no output in a clean checkout.
