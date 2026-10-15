# Portable distribution validation

Validation date: 2026-10-15. Host: Ubuntu 24.04, Linux x86_64, CMake 3.28.3,
Ninja 1.11.1, GCC/G++ 13.3.0, Qt 6.4.2. Temporary HOME and installation
paths were used for lifecycle tests. The archive itself is intentionally not
versioned.

| Test | Status | Evidence |
|---|---|---|
| T01 Build | PASS | Combined CMake/Ninja build completed with 503/503 targets; the final incremental build reported no work. |
| T02 Package creation | PASS | Final archive generated, both JSON manifests parse, `sha256sum -c` passes, and no temporary `elf-queue` entry is present. |
| T03 Install without sudo | PASS | `test-portable.sh` installs twice in a temporary HOME and prefix without root or network. |
| T04 Build-environment independence | PASS | Lifecycle test clears `LD_LIBRARY_PATH`, `QT_PLUGIN_PATH`, CMake variables and restricts `PATH` to `/usr/bin:/bin`. |
| T05 Main GUI | PASS | Main GUI remains alive for the 8-second Xvfb timeout with the packaged Qt platform plugin and no startup error. |
| T06 Shell | PASS | Shell loads and shows `ExampleBasicOrderShipping.gen`, then exits deterministically. |
| T07 Worker | PASS | Worker starts with ephemeral port `0`, remains alive, and is stopped by the test; no service is installed. |
| T08 Independent GUIs | PASS | HTTP Worker, Data Analyser, Optimizer and AI Assistant GUIs each remain alive under Xvfb timeout. |
| T09 Relocatability | PASS | Installation and execution pass from temporary prefixes, including a prefix containing spaces and an arbitrary working directory. |
| T10 Reinstallation | PASS | Two consecutive installations into the same prefix pass without duplicate desktop entries or data loss. |
| T11 Upgrade/rollback | PASS (fixture) | Same-version replacement passes; a checksum-corrupted replacement is rejected while the prior marker/install remains. No second real binary version was available. |
| T12 Uninstall | PASS | Runtime and marked desktop entry are removed; user data is preserved by the lifecycle and desktop tests. |
| T13 Security | UNKNOWN | Corrupt package, unsafe system prefix and MCP diagnostic were tested. Root/non-owned-prefix rejection and adversarial archive-entry fixtures were not independently exercised in this non-root run. |
| T14 Regression | FAIL | Launcher tests pass 34/34 and smoke passes 3/3; `tests-unit` and `tests-kernel-unit` each reproduce one existing `BacteriaColonyTwoSignalDemoLoadsRunsAndPersists` failure, with 1898/1899 executed tests passing and 4 disabled. |
| T15 Debian | UNKNOWN | Debian packaging files were not changed, but a complete `dpkg-buildpackage`/lintian validation was not run in this cycle. |
| T16 UFSC laboratory | UNKNOWN | No physical UFSC laboratory host was available; the procedure is documented in `README.md`. |

## Final artifact

- Archive: `/home/rafaelcancian/Repositories/Genesys-Simulator/packaging/installer/dist/genesys-2026.10.15.2351-linux-x86_64-portable.tar.gz`
- Size: 71946540 bytes
- SHA-256: `7034b9caf4d394c3833e4f33558f4b85d85d58d862d575165db4aa7291fb007e`
- Build commit embedded in the archive: `14016456ab5b2d800f964f4493018e23bbd6cf68`

The package is ready for controlled review and laboratory staging, not for a
claim of universal Linux compatibility or release publication.
