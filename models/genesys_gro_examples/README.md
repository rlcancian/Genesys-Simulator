# GenESyS Gro examples

These examples are original GenESyS fixtures written from scratch. They are
inspired by the supported GenESyS-native Gro subset and are not copies or
modified versions of files from the original University of Washington `gro`
repository.

The files are distributed under the GenESyS project license. They are
regression fixtures and teaching examples, not a claim of full compatibility
with the original Gro language or runtime.

The examples intentionally use only constructs covered by the current
GenESyS parser/compiler tests:

- `signal()` declarations with independent handles;
- `emit_signal()` and `get_signal()`;
- named programs and `ecoli()` placement;
- scalar assignments and conditional rules;
- `grow()`, `move()`, and `divide()` commands.

Run the fixture validation from the repository root with:

```bash
cmake --build build/tests-unit --target genesys_test_genesys_gro_fixtures
ctest --test-dir build/tests-unit -R GenesysGroFixtures --output-on-failure
```

The test validates parsing and compilation of every fixture. It does not claim
scientific equivalence to the original Gro implementation.
