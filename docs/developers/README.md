# Developer documentation

This directory contains developer-facing Doxygen configurations for GenESyS.

## Generate Doxygen documentation

Run from the repository root:

```bash
doxygen docs/developers/DoxyfileDeveloper2022
doxygen docs/developers/DoxyfileDeveloper2026
```

Generated files are written under:

```text
docs/developers/generated/developer2022/
docs/developers/generated/developer2026/
```

The `.legacy` files preserve the historical full Doxygen configurations. The main `DoxyfileDeveloper2022` and `DoxyfileDeveloper2026` files are small wrappers that include the legacy configurations and override repository-relative paths for the current `docs/` layout.
