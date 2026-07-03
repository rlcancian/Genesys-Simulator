# GenESyS Debian Packaging — Phase 0

## Scope

Initial Debian/Ubuntu packaging for the GenESyS Qt GUI and web application.

Base branch: `WiP20261`
Packaging branch: `packaging/debian-phase0`
Primary target: Ubuntu 24.04 LTS `noble` on `amd64`

This phase defines naming, versioning and packaging boundaries only. It intentionally avoids CMake installation rules, Debian control files, package repository configuration and Snap metadata.

## Source package

Debian source package name:

- `genesys-simulator`

Rationale: the source package represents the whole GenESyS simulator repository, not only one executable.

## Binary packages

### `genesys-gui`

Purpose:

- installs the Qt desktop GUI.

Public command planned for installation:

- `/usr/bin/genesys-gui`

Current internal CMake executable target:

- `genesys_qt_gui_application`

Current aggregate build target:

- `genesys_gui`

Observed build dependency implication:

- the GUI target links Qt Core, Gui, Widgets, PrintSupport and Network;
- the GUI target also links `genesys_web_core`, `genesys_tools`, kernel, parser and plugin static libraries;
- therefore the initial GUI package build is expected to configure both GUI and web support.

### `genesys-web`

Purpose:

- installs the web/webhook executable.

Public command planned for installation:

- `/usr/bin/genesys-web`

Current internal CMake executable target:

- `genesys_web_app`

Current compatibility alias:

- `genesys_webhook`

Deferred runtime decision:

- whether this package should install only a command-line executable or also a `systemd` service is intentionally postponed.

## Versioning policy

Upstream tags should use semantic project releases:

- `v2026.1.0`
- `v2026.1.1`

Debian package versions should use:

- `2026.1.0-1`
- `2026.1.1-1`

Ubuntu/PPA builds should append the Ubuntu target release:

- `2026.1.0-1~ubuntu24.04.1`

Snapshot builds may use date and commit identifiers:

- `2026.1.0~gitYYYYMMDD.<shortsha>-1`

## Initial build configuration

The initial Debian package build should use CMake with Ninja.

Required CMake configuration options for the first packaging iteration:

- `-DGENESYS_BUILD_GUI_APPLICATION=ON`
- `-DGENESYS_BUILD_WEB_APPLICATION=ON`
- `-DGENESYS_BUILD_TERMINAL_APPLICATION=OFF`
- `-DGENESYS_BUILD_TESTS=ON`
- `-DCMAKE_BUILD_TYPE=Release`
- `-DCMAKE_INSTALL_PREFIX=/usr`

The terminal application is intentionally excluded from the first Debian packaging iteration to keep the package scope narrow and reviewable.

## Planned installation paths

The first installable package iteration should aim for the following public paths:

- `/usr/bin/genesys-gui`
- `/usr/bin/genesys-web`
- `/usr/share/applications/io.github.rlcancian.genesys.desktop`
- `/usr/share/metainfo/io.github.rlcancian.genesys.metainfo.xml`
- `/usr/share/icons/hicolor/scalable/apps/io.github.rlcancian.genesys.svg`
- `/usr/share/doc/genesys-gui/`
- `/usr/share/doc/genesys-web/`

These paths are not implemented in Phase 0. They are target contracts for the next implementation phases.

## Debian package strategy

The first Debian package should be generated locally with `dpkg-buildpackage` from repository packaging metadata.

The first successful outcome is a pair of installable local packages:

- `genesys-gui_<version>_amd64.deb`
- `genesys-web_<version>_amd64.deb`

APT repository publishing, PPA publishing and Snap packaging are out of scope for the first `.deb` package, but the names and versions above must remain compatible with those future distribution channels.

## Deferred decisions

The following points are intentionally deferred:

- whether `genesys-web` should install a `systemd` unit;
- whether bundled models/examples should be shipped in a separate package;
- whether terminal execution should become a separate `genesys-cli` package;
- whether plugins should become runtime-loaded shared libraries and separate packages;
- whether a future development package such as `libgenesys-kernel-dev` is justified;
- whether the project should publish through PPA, a self-hosted APT repository, Snap Store, or a combination of these.

## Exit criteria for Phase 0

Phase 0 is complete when this document exists in the packaging branch and no functional source or build files have been changed.

The next phase should add minimal CMake installation rules for the GUI and web executables, without adding Debian packaging metadata yet.
