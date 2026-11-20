# GenESyS Launcher and Per-User Runtime Dispatcher

## Purpose

`source/applications/launcher` provides the stable invocation layer required for a future Debian installation to coexist with independently updated GenESyS runtimes in each user's home directory.

This directory builds two executables:

- `genesys-launcher`: interactive GUI entry point and the **only** process in this domain permitted to check for remote runtime updates;
- `genesys-dispatch`: non-interactive application dispatcher. It never checks for updates, never uses the network, never displays an update dialog, and never modifies the active runtime.

The launcher is intentionally independent of the simulator kernel, parser and plugin libraries. The focused `launcher-app` preset disables those subsystems.

## Architecture

```text
future Debian installation
  |
  +-- stable genesys-launcher / genesys-dispatch
  +-- stable system fallback runtime
  |     `-- <system-runtime-root>/bin/<application>
  `-- per-user runtime support
        |
        +-- $XDG_DATA_HOME/genesys/apps/<version>/
        `-- $XDG_DATA_HOME/genesys/current -> apps/<version>
```

A future package may expose public `/usr/bin/genesys-*` wrappers, but this implementation does **not** create or replace those wrappers. Debian package restructuring is a separate integration phase.

## CMake targets and preset

The root option is:

```text
GENESYS_BUILD_LAUNCHER_APPLICATION
```

Focused tests use:

```text
GENESYS_BUILD_LAUNCHER_TESTS
```

The application CMake targets are:

```text
genesys-launcher
genesys-dispatch
```

Shared internal libraries are:

```text
genesys_launcher_runtime   # Qt6::Core only
genesys_launcher_update    # runtime + Qt6::Network
```

The dedicated preset is:

```bash
cmake --preset launcher-app
cmake --build --preset launcher-app
ctest --preset launcher-app --output-on-failure
```

The preset builds the launcher, dispatcher and focused tests without building the GenESyS kernel, parser, plugins, worker or main GUI.

## Install contract for future packaging

CMake installs only the stable internal executables:

```text
${CMAKE_INSTALL_LIBEXECDIR}/genesys/genesys-launcher
${CMAKE_INSTALL_LIBEXECDIR}/genesys/genesys-dispatch
```

It deliberately does **not** install public `/usr/bin` wrappers in this change.

The configurable system fallback root defaults conceptually to:

```text
/usr/libexec/genesys/system/
  `-- bin/
      +-- genesys-gui
      +-- genesys-shell
      +-- genesys-worker
      `-- genesys-mcp
```

The path is centralized as the CMake cache variable `GENESYS_SYSTEM_RUNTIME_ROOT`; tests override it with temporary directories.

Other build-time integration variables are:

- `GENESYS_LAUNCHER_VERSION` — stable launcher contract version;
- `GENESYS_SYSTEM_PACKAGE_VERSION` — base installation version used for manifest compatibility checks;
- `GENESYS_SYSTEM_CONFIG_PATH` — defaults to `/etc/genesys/update.conf`;
- `GENESYS_UPDATE_KEYRING_PATH` — trusted OpenPGP keyring used by `gpgv`.

The repository currently has no canonical project-wide CMake version. Therefore these launcher/package version inputs default to `0.0.0` rather than creating a competing global version source. The future Debian/release integration must provide authoritative values at build time.

## XDG directories

User paths are resolved centrally by `RuntimePaths`.

When the corresponding XDG variable is an absolute path:

```text
configuration: $XDG_CONFIG_HOME/genesys/update.conf
cache:         $XDG_CACHE_HOME/genesys/
data:          $XDG_DATA_HOME/genesys/
```

Fallbacks when the XDG variable is absent are:

```text
~/.config/genesys/update.conf
~/.cache/genesys/
~/.local/share/genesys/
```

Runtime layout:

```text
~/.local/share/genesys/apps/<version>/
~/.local/share/genesys/current
```

Download cache:

```text
~/.cache/genesys/downloads/
```

Launcher log:

```text
~/.local/share/genesys/logs/launcher.log
```

If `HOME` is unavailable or unusable, per-user runtime selection/update is disabled and the dispatcher can use only the configured system fallback.

## Configuration and precedence

The launcher reads:

```text
/etc/genesys/update.conf
$XDG_CONFIG_HOME/genesys/update.conf
```

The user configuration is loaded first and explicitly supplied system values are applied last. Consequently the system policy wins for every key that it defines.

Supported `[updates]` keys are:

```ini
[updates]
enabled=true
channel=stable
allow_user_runtime=true
require_signature=true
check_on_gui_startup=true
fallback_to_system=true
minimum_check_interval_hours=12
max_user_versions=2
manifest_url=https://example.invalid/update-manifest.json
```

Remote updating is fail-safe by default: **no remote check occurs unless explicit effective policy exists and provides a valid HTTPS `manifest_url`.** A source-tree/development build therefore does not unexpectedly contact GitHub.

Invalid system security values are handled conservatively. In particular, invalid `enabled`, `allow_user_runtime`, or `require_signature` values cannot silently enable an unsafe path.

## Application selection

`genesys-dispatch` requires an explicit application:

```bash
genesys-dispatch --app genesys-shell -- <arguments>
genesys-dispatch --app genesys-worker -- <arguments>
genesys-dispatch --app genesys-mcp -- <arguments>
```

The dispatcher preserves the subsequent argument vector; it does not construct a shell command. On Linux, `ProcessLauncher` uses `execv()` so the stable dispatcher process is replaced by the selected application and the environment/current working directory are inherited.

Selection is deterministic:

1. read effective policy;
2. if user runtimes are permitted, inspect `current`;
3. use the requested user-runtime application only when the runtime and executable satisfy validation;
4. otherwise use the system application when fallback is permitted;
5. return a controlled error if neither exists.

The dispatcher never downloads, activates, cleans, or changes `current`.

## Active runtime validation

A user runtime is eligible only when:

- `current` is a symbolic link;
- it resolves to a direct child of the configured `apps` root;
- the target does not escape the authorized data tree;
- `VERSION` exists and parses as a numeric dotted version;
- `MANIFEST.json` exists and has schema version 1/product `genesys-simulator`;
- the internal manifest version matches `VERSION`;
- the requested application is declared by the internal manifest;
- the corresponding `bin/<application>` is a regular executable file;
- the executable is not a symlink and resolves inside the runtime `bin` directory.

Any failed condition causes a logged fallback rather than execution of an untrusted path.

## `genesys-worker` and legacy `genesys-web`

The current CMake worker target is `genesys_worker_app` and its output is `genesys-worker`. The existing Debian tree still contains `genesys-web` naming.

The canonical application name for the launcher contract is therefore `genesys-worker`.

For a request for `genesys-web`, selection is explicitly ordered as:

```text
1. genesys-web, when that executable is explicitly present and declared;
2. genesys-worker as the compatibility alias;
3. system fallback under the same ordered rule.
```

No other application receives an implicit alias.

## MCP

`source/applications/mcp` is an existing Python MCP server and publishes the entry point:

```text
genesys-mcp = genesys_mcp.server:main
```

The launcher does not rewrite it in C++ and does not install Python packages.

If `bin/genesys-mcp` is declared and present in an active runtime, the dispatcher executes it like any other application. Otherwise it tries the system fallback. If neither exists, `genesys-dispatch --app genesys-mcp` returns a controlled message directing the user to `genesys-gui` or to a distribution that includes MCP.

## Version rules

`Version` accepts dotted non-negative numeric components, including a leading `v` when parsing a release tag:

```text
2026.1.0
2026.1.2
v2026.2.0
```

Comparisons pad missing trailing components with zero (`2026.1 == 2026.1.0`). Non-numeric prerelease syntax is intentionally rejected because it is outside the current update contract.

A remote candidate older than the active/base version is rejected as a downgrade.

## Platform detection

On Linux the updater reads `/etc/os-release` and combines normalized `ID`, `VERSION_ID`, and the actual CPU architecture.

Example:

```text
ubuntu-24.04-x86_64
```

Linux is not assumed to mean Ubuntu. If platform identification fails or the exact key is absent from the manifest, no update is installed and normal runtime/system selection remains available.

## Remote manifest schema

The parser supports schema version 1 and requires:

```json
{
  "schema_version": 1,
  "product": "genesys-simulator",
  "channel": "stable",
  "version": "2026.2.1",
  "release_tag": "v2026.2.1",
  "published_at": "2026-08-20T00:00:00Z",
  "minimum_launcher_version": "2026.1.0",
  "minimum_system_package_version": "2026.1.0",
  "platforms": {
    "ubuntu-24.04-x86_64": {
      "kind": "user-runtime",
      "url": "https://example.invalid/genesys-runtime.tar.zst",
      "sha256": "<64 lowercase/uppercase hexadecimal characters>",
      "signature_url": "https://example.invalid/genesys-runtime.tar.zst.sig",
      "size": 123,
      "applications": [
        "genesys-gui",
        "genesys-shell",
        "genesys-worker",
        "genesys-mcp"
      ]
    }
  }
}
```

The parser rejects unsupported schema/product/channel/platform, invalid versions, downgrade, invalid/non-HTTPS URLs, invalid SHA-256, invalid sizes, duplicate/invalid/empty application lists, and unsatisfied minimum launcher/system-package versions. GitHub HTML is never scraped.

## Network behavior

Only the launcher update library links `Qt6::Network`.

`QtNetworkTransport` enforces:

- HTTPS-only requests for production update operations;
- bounded response sizes;
- finite transfer timeouts;
- a bounded redirect count;
- Qt's no-less-safe redirect policy;
- normal TLS validation (TLS errors are never ignored);
- cancellation;
- streaming downloads to a destination instead of concatenating shell commands or trusting a remote filename.

The manifest is limited to 1 MiB. Runtime bundle size is bounded by the signed manifest and currently capped at 2 GiB by the manifest parser. Signature downloads are limited to 1 MiB.

Tests use an injected `INetworkTransport`; they do not access GitHub or any public network.

## Integrity and signature verification

SHA-256 verification is mandatory for every runtime bundle. The expected digest is supplied by the validated remote manifest. Digest comparison is performed over decoded fixed-size digest bytes without early exit.

When `require_signature=true`, `GpgvSignatureVerifier` invokes `gpgv` directly with an argument vector:

```text
gpgv --keyring <trusted-keyring> <signature> <bundle>
```

No shell is involved. Signature-required mode fails closed when the trusted keyring, bundle/signature, `gpgv`, or a valid signature is unavailable.

This repository change does not choose or install the institutional/release signing key. Future Debian integration must install the public keyring and declare `gpgv`/archive runtime dependencies as appropriate. The code does not weaken signature-required policy while that deployment step is pending.

## Secure archive extraction

Runtime archives are expected to be `tar.zst`.

`TarZstdArchiveExtractor` invokes the system `tar` directly, never through a shell. Before extraction it inspects both path and verbose listings.

The current implementation intentionally accepts only:

- regular files;
- directories.

It rejects:

- absolute paths;
- `.`/`..` path components and traversal;
- escaped/backslash path forms in the archive listing;
- symlinks;
- hardlinks;
- device nodes;
- FIFOs and other special entries;
- setuid/setgid entries.

Extraction uses `--no-same-owner` and `--no-same-permissions`, targets a fresh `.partial` directory, and the resulting tree is scanned again for links/special entries/root escape before runtime validation.

Future Debian packaging must ensure compatible `tar` + zstd support is installed. Missing tooling causes a controlled update failure and leaves the existing runtime/system installation usable.

## Runtime validation and atomic activation

The installation flow is:

```text
download .part
  -> verify exact size
  -> verify SHA-256
  -> verify OpenPGP signature when required
  -> promote verified cache file
  -> extract to apps/<version>.partial
  -> validate extracted runtime
  -> rename to apps/<version>
  -> create temporary current symlink
  -> atomic rename over current
```

The extracted runtime must contain:

```text
VERSION
MANIFEST.json
bin/
```

The internal manifest must match the remote version and application set. The complete runtime contract currently requires at least:

```text
genesys-gui
genesys-shell
genesys-worker
genesys-mcp
```

Every declared application must exist as a non-symlink regular executable in `bin/`.

`current` is never changed before extraction and validation finish. On Unix the temporary symlink is atomically renamed over the previous `current` symlink. A non-symlink `current` entry is never overwritten.

## Concurrency and retention

`UserRuntimeManager` uses `QLockFile` under the GenESyS user data root. The launcher acquires the lock before download finalization and holds it through verification, installation, activation and cache cleanup. It is released before the selected GenESyS application is executed.

Runtime cleanup is best-effort and respects `max_user_versions`. It always preserves:

- the newly active runtime;
- the previously active runtime as immediate rollback.

Therefore rollback safety can retain two versions even when an administrative `max_user_versions` value of 1 is supplied.

## Logging

The default launcher log is:

```text
$XDG_DATA_HOME/genesys/logs/launcher.log
```

Each line contains a UTC timestamp, launcher version, event and bounded diagnostic detail. Relevant events include selection/fallback, update checks, downloads, verification, extraction/activation errors, activation and retention.

The implementation does not log configuration contents, tokens, credentials or secrets. The log rotates to `.1` when it reaches approximately 1 MiB.

## Launcher CLI

Supported operations are intentionally small:

```text
genesys-launcher --app genesys-gui --interactive-update
genesys-launcher --check-only
genesys-launcher --no-update
genesys-launcher --version
```

`--check-only` reports through console output and never installs a runtime. `--no-update` bypasses the update path and launches according to the selector. Normal `genesys-gui` startup checks only when `check_on_gui_startup=true` and all other effective update policy requirements permit it.

The launcher refuses remote update modes for non-GUI applications. Non-interactive applications must use `genesys-dispatch`.

## Failure and fallback behavior

Update failure is not application failure. Network errors, invalid manifests, checksum/signature errors, archive errors, disk/write errors, invalid runtime trees, lock contention and activation errors all leave the previously active `current` untouched.

After update handling, the launcher selects and starts the active user GUI or the system fallback. The dispatcher similarly selects user/system applications without attempting an update.

On Linux `ProcessLauncher` attempts the system fallback if execution of a selected user executable returns an execution error such as `EACCES` (including a home mounted `noexec`).

## Tests

Focused Google Test coverage uses only temporary roots and local test doubles. It covers:

- version parsing/comparison/downgrade;
- user/system configuration and fail-safe defaults;
- XDG defaults/overrides/missing HOME;
- remote manifest contract and minimum versions;
- runtime selection/fallback;
- `genesys-web` compatibility and MCP;
- SHA-256 and signature-required fail-closed paths;
- unsafe archive listings;
- `.partial` installation and atomic activation;
- preservation of the previous `current` on failures;
- retention/rollback;
- dispatcher argument forwarding through an injected process launcher;
- update-check interval behavior;
- a complete local fake manifest -> download -> checksum -> signature -> extract -> validate -> activate -> select cycle.

The focused GitHub Actions workflow additionally records dynamic-link evidence showing that `genesys-dispatch` does not link `Qt6::Network` or `Qt6::Widgets`.

## Known limitations and future Debian integration

This change intentionally does not:

- rewrite `debian/control` or `.install` files;
- replace `/usr/bin/genesys-gui` or create final wrappers;
- move current application binaries into the system fallback tree;
- install the OpenPGP public keyring;
- publish/sign release runtime bundles;
- convert the Python MCP server to C++;
- perform a real `dpkg` installation or laboratory test.

The next Debian phase must map the existing application outputs into the system fallback directory, install public wrappers that call this stable launcher/dispatcher, reconcile legacy `genesys-web` packaging with canonical `genesys-worker`, package the existing Python `genesys-mcp` when desired, install update policy/key material/tool dependencies, and validate the complete package lifecycle as a non-root laboratory user.

Real Debian install, user-without-sudo, `HOME` mounted `noexec`, and UFSC laboratory behavior remain `needs-local-validation` until that packaging phase exists.
