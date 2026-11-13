# Read — InputOutput ModelComponent

Status: implementation in progress. Verify CI and GUI behavior before claiming support.

## Supported behavior

`Read` is a simulation component with one conventional input and one output. It is intentionally independent of the existing `Write` component. Its properties are `ReadFromType` (`FILE`, `KEYBOARD`), `Filename`, and an ordered `Destinations` list. List entries expose `Destination` and `AttributeNotVariable` (true: current entity attribute, false: model-global variable) in the existing property-list editor.

The persisted component type is `Read`, its registration alias is `read.so`, and its category is `InputOutput`. Runtime file cursor, open streams, and keyboard callback are not serialized.

## FILE mode

The component reads exactly one nonempty, noncomment line per arriving entity. It ignores blank lines and lines whose first non-whitespace character is `#`. Each record contains **exactly as many whitespace-separated, finite floating-point numbers as configured destinations**. Examples with three destinations:

```text
# processing time, load, priority
10 25 3
12 30 4
15 28 2
```

The format uses the C decimal point (`.`); no locale-specific separators, quoted fields, CSV semantics, or inline comments are supported. Fields are converted to `double`, as required by the current Attribute/Variable APIs.

The first eligible record is consumed at the first arriving entity; subsequent arrivals consume subsequent records. The file is opened lazily and left open during a replication. It is closed and the cursor is reset at the start of **every replication**. The file path is not rewritten by `Read`; relative paths resolve under the simulator process's current working directory. Neither this component nor the `File` data definition provides a shared file cursor or recordset.

## KEYBOARD mode

The host application may supply a non-persistent `Read::KeyboardInputProvider` callback, which returns one complete record using the same parsing rules as FILE mode. Automated tests inject this provider. If no provider exists, a POSIX terminal running with an interactive stdin TTY can read one line from `std::cin`. Without a TTY or provider, input fails without blocking on a pipe.

The Qt6 GUI binds the provider via a GUI extension at simulation start; it opens a modal text prompt on the GUI thread. Qt is not used by the simulation component or kernel. A non-GUI-thread invocation, a cancelled dialog, or a closing window rejects input. The synchronous prompt uses the Qt dialog event loop; it **does not** implement asynchronous event suspension or cross-thread marshalling. This is intentionally a limited interaction contract.

## Destination semantics

Destinations may be scalar names (`ProcessingTime`) or names suffixed by an existing index representation (`Load[3]`). Attribute names address the entity entering Read; Variable names address the model-wide data definition. The ordered list editor supports creating, removing, changing kind/name, and reordering entries. Like `Assign`, the component declares these definitions if they are not present while preparing model data. `Read` does not evaluate arbitrary expressions in incoming text.

Before applying a record, `Read` parses all fields, checks their count, and resolves each destination. It then assigns each field in order. Errors (bad/missing file, EOF, invalid/incomplete record, inaccessible keyboard, cancellation, or unresolved destination) are traced and request simulation stop. **The arriving entity is not forwarded** and the fields from a malformed record are not applied. This does not roll back prior entities or previously consumed records.

## Persistence and validation

Serialized fields: `readFromType`, `filename`, `readDestinations` and pairs `readDest{index}` / `readIsAttrib{index}`. No changes to existing `Write` or `File` persistence records. Errors and source compatibility should be validated by the focused `genesys_test_read_component` suite, ordinary and kernel presets, smoke tests, and GUI build.

## Known limits

- Only finite numeric `double` values; no string values, dates, or typed records.
- No CSV, binary, SQL, ADO, Excel, XML, or complex Arena records.
- Only one logical reader cursor per component instance. Two `Read` components pointed at the same file each start from the beginning.
- No retry/skip/alternate output on a malformed input; the simulation is stopped.
- GUI input requires user intervention at the event and must be checked interactively on the supported desktop. Headless CI verifies the injection seam but cannot prove a real user's dialog experience.
- Input is inherently nondeterministic if entered manually. To reproduce a run, record the input sequence externally; the values are not persisted in a simulation model.
