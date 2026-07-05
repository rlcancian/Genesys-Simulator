# Developer documentation

This directory contains developer-facing documentation configuration for GenESyS.

Current Doxygen entrypoint:

    doxygen docs/developers/DoxyfileDeveloper

Doxygen working files are generated under:

    build/doxygen/developers/

Only the final PDF should be versioned here:

    docs/developers/GenESyS-Developer-Documentation.pdf

Man pages are generated under:

    build/doxygen/developers/man/

Those man pages are intended for Debian/PPA packaging workflows and should be collected from the build tree, not committed as ordinary documentation.
