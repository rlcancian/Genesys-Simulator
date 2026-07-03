# Matriz completa: plugins/components (classe x método)

Legenda de status: `implemented`, `todo`, `empty`, `missing`.

|Priority|Class|Header|CPP|GetPluginInformation|LoadInstance|NewInstance|_loadInstance|_saveInstance|_check|_createInternalAndAttachedData|_initBetweenReplications|_addProperty|
|---|---|---|---|---|---|---|---|---|---|---|---|---|
|P0|LSODE|source/plugins/components/LSODE.h|source/plugins/components/LSODE.cpp|implemented|implemented|implemented|todo|todo|todo|missing|missing|missing|
|P0|Access|source/plugins/components/Access.h|source/plugins/components/Access.cpp|implemented|implemented|implemented|todo|todo|todo|missing|empty|missing|
|P0|DropOff|source/plugins/components/DropOff.h|source/plugins/components/DropOff.cpp|implemented|implemented|implemented|todo|todo|todo|missing|empty|missing|
|P0|Exit|source/plugins/components/Exit.h|source/plugins/components/Exit.cpp|implemented|implemented|implemented|todo|todo|todo|missing|missing|missing|
|P0|MarkovChain|source/plugins/components/MarkovChain.h|source/plugins/components/MarkovChain.cpp|implemented|implemented|implemented|todo|todo|todo|missing|implemented|missing|
|P0|Signal|source/plugins/components/Signal.h|source/plugins/components/Signal.cpp|implemented|implemented|implemented|todo|todo|todo|implemented|empty|missing|
|P0|Start|source/plugins/components/Start.h|source/plugins/components/Start.cpp|implemented|implemented|implemented|todo|todo|todo|missing|missing|missing|
|P0|Stop|source/plugins/components/Stop.h|source/plugins/components/Stop.cpp|implemented|implemented|implemented|todo|todo|todo|missing|missing|missing|
|P0|Submodel|source/plugins/components/Submodel.h|source/plugins/components/Submodel.cpp|implemented|implemented|implemented|todo|todo|todo|missing|missing|missing|
|P0|Unstore|source/plugins/components/Unstore.h|source/plugins/components/Unstore.cpp|implemented|implemented|implemented|todo|todo|todo|missing|missing|missing|
|P0|CppForG|source/plugins/components/CppForG.h|source/plugins/components/CppForG.cpp|implemented|implemented|implemented|todo|todo|todo|implemented|empty|missing|
|P0|DiffEquations|source/plugins/components/DiffEquations.h|source/plugins/components/DiffEquations.cpp|empty|implemented|implemented|todo|todo|todo|implemented|implemented|empty|
|P0|SPICECircuit|source/plugins/components/SPICECircuit.h|source/plugins/components/SPICECircuit.cpp|implemented|implemented|implemented|todo|todo|missing|missing|missing|missing|
|P0|SPICENode|source/plugins/components/SPICENode.h|source/plugins/components/SPICENode.cpp|implemented|implemented|implemented|todo|todo|missing|missing|missing|missing|
|P0|Search|source/plugins/components/Search.h|source/plugins/components/Search.cpp|implemented|implemented|implemented|todo|todo|implemented|todo|missing|missing|
|P0|Remove|source/plugins/components/Remove.h|source/plugins/components/Remove.cpp|implemented|implemented|implemented|todo|todo|implemented|todo|missing|missing|
|P0|Buffer|source/plugins/components/Buffer.h|source/plugins/components/Buffer.cpp|implemented|implemented|implemented|todo|todo|implemented|implemented|implemented|empty|
|P0|CellularAutomataComp|source/plugins/components/CellularAutomataComp.h|source/plugins/components/CellularAutomataComp.cpp|implemented|implemented|implemented|todo|todo|implemented|missing|implemented|missing|
|P0|DefaultModalModel|source/plugins/components/DefaultModalModel.h|source/plugins/components/DefaultModalModel.cpp|empty|implemented|implemented|todo|todo|implemented|implemented|implemented|empty|
|P0|DefaultNode|source/plugins/components/network/DefaultNode.h|source/plugins/components/network/DefaultNode.cpp|empty|implemented|implemented|todo|todo|implemented|implemented|implemented|empty|
|P0|DummyComponent|source/plugins/components/DummyComponent.h|source/plugins/components/DummyComponent.cpp|empty|implemented|implemented|todo|todo|implemented|implemented|implemented|empty|
|P0|PickStation|source/plugins/components/PickStation.h|source/plugins/components/PickStation.cpp|implemented|implemented|implemented|todo|todo|implemented|implemented|empty|implemented|
|P0|Wait|source/plugins/components/Wait.h|source/plugins/components/Wait.cpp|implemented|implemented|implemented|todo|todo|implemented|implemented|empty|missing|
|P0|Match|source/plugins/components/Match.h|source/plugins/components/Match.cpp|implemented|implemented|implemented|todo|implemented|todo|implemented|missing|missing|
|P1|Assign|source/plugins/components/Assign.h|source/plugins/components/Assign.cpp|implemented|implemented|implemented|implemented|implemented|todo|implemented|empty|missing|
|P1|Create|source/plugins/components/Create.h|source/plugins/components/Create.cpp|implemented|implemented|implemented|implemented|implemented|todo|implemented|implemented|missing|
|P1|Process|source/plugins/components/Process.h|source/plugins/components/Process.cpp|implemented|implemented|implemented|implemented|todo|implemented|implemented|missing|missing|
|P1|Write|source/plugins/components/Write.h|source/plugins/components/Write.cpp|implemented|implemented|implemented|implemented|todo|implemented|missing|implemented|missing|
|P2|Seize|source/plugins/components/Seize.h|source/plugins/components/Seize.cpp|implemented|implemented|implemented|implemented|implemented|implemented|todo|todo|missing|
|P2|Delay|source/plugins/components/Delay.h|source/plugins/components/Delay.cpp|implemented|implemented|implemented|implemented|implemented|implemented|todo|missing|missing|
|P2|Batch|source/plugins/components/Batch.h|source/plugins/components/Batch.cpp|implemented|implemented|implemented|implemented|implemented|implemented|implemented|missing|missing|
|P2|Clone|source/plugins/components/Clone.h|source/plugins/components/Clone.cpp|implemented|implemented|implemented|implemented|implemented|implemented|implemented|empty|missing|
|P2|Decide|source/plugins/components/Decide.h|source/plugins/components/Decide.cpp|implemented|implemented|implemented|implemented|implemented|implemented|implemented|implemented|missing|
|P2|Dispose|source/plugins/components/Dispose.h|source/plugins/components/Dispose.cpp|implemented|implemented|implemented|implemented|implemented|implemented|implemented|implemented|missing|
|P2|Enter|source/plugins/components/Enter.h|source/plugins/components/Enter.cpp|implemented|implemented|implemented|implemented|implemented|implemented|implemented|empty|missing|
|P2|Leave|source/plugins/components/Leave.h|source/plugins/components/Leave.cpp|implemented|implemented|implemented|implemented|implemented|implemented|implemented|empty|missing|
|P2|Record|source/plugins/components/Record.h|source/plugins/components/Record.cpp|implemented|implemented|implemented|implemented|implemented|implemented|implemented|implemented|missing|
|P2|Release|source/plugins/components/Release.h|source/plugins/components/Release.cpp|implemented|implemented|implemented|implemented|implemented|implemented|implemented|empty|missing|
|P2|Route|source/plugins/components/Route.h|source/plugins/components/Route.cpp|implemented|implemented|implemented|implemented|implemented|implemented|implemented|empty|missing|
|P2|Separate|source/plugins/components/Separate.h|source/plugins/components/Separate.cpp|implemented|implemented|implemented|implemented|implemented|implemented|missing|missing|missing|
|P2|Store|source/plugins/components/Store.h|source/plugins/components/Store.cpp|implemented|implemented|implemented|implemented|implemented|implemented|missing|missing|missing|

## Priorização automática (execução incremental)

- Regra usada: P0 quando dois ou mais métodos críticos (`_loadInstance`, `_saveInstance`, `_check`) estão `missing/todo/empty`; P1 para um método crítico; P2 para nenhum.

- Ordem sugerida dentro de cada prioridade: maior número de lacunas críticas e TODOs.


### P0 (24 classes)
- LSODE (critical_gaps=3, todo_markers=7)
- Access (critical_gaps=3, todo_markers=6)
- DropOff (critical_gaps=3, todo_markers=6)
- Exit (critical_gaps=3, todo_markers=6)
- MarkovChain (critical_gaps=3, todo_markers=6)
- Signal (critical_gaps=3, todo_markers=6)
- Start (critical_gaps=3, todo_markers=6)
- Stop (critical_gaps=3, todo_markers=6)
- Submodel (critical_gaps=3, todo_markers=6)
- Unstore (critical_gaps=3, todo_markers=6)
- CppForG (critical_gaps=3, todo_markers=5)
- DiffEquations (critical_gaps=3, todo_markers=5)
- SPICECircuit (critical_gaps=3, todo_markers=4)
- SPICENode (critical_gaps=3, todo_markers=4)
- Search (critical_gaps=2, todo_markers=6)

### P1 (4 classes)
- Assign (critical_gaps=1, todo_markers=1)
- Create (critical_gaps=1, todo_markers=1)
- Process (critical_gaps=1, todo_markers=1)
- Write (critical_gaps=1, todo_markers=1)

### P2 (13 classes)
- Seize (critical_gaps=0, todo_markers=2)
- Delay (critical_gaps=0, todo_markers=1)
- Batch (critical_gaps=0, todo_markers=0)
- Clone (critical_gaps=0, todo_markers=0)
- Decide (critical_gaps=0, todo_markers=0)
- Dispose (critical_gaps=0, todo_markers=0)
- Enter (critical_gaps=0, todo_markers=0)
- Leave (critical_gaps=0, todo_markers=0)
- Record (critical_gaps=0, todo_markers=0)
- Release (critical_gaps=0, todo_markers=0)
- Route (critical_gaps=0, todo_markers=0)
- Separate (critical_gaps=0, todo_markers=0)
- Store (critical_gaps=0, todo_markers=0)