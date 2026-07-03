# User documentation

This directory contains user-facing documentation configuration for GenESyS.

## Generate Doxygen documentation

Run from the repository root:

```bash
doxygen docs/users/DoxyfileUser2022
```

Generated files are written under:

```text
docs/users/generated/user2022/
```

`DoxyfileUser2022.legacy` preserves the historical full Doxygen configuration. `DoxyfileUser2022` is a small wrapper that includes the legacy configuration and overrides repository-relative paths for the current `docs/` layout.
