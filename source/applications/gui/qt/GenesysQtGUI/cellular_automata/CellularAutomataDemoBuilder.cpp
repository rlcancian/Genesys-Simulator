#include "CellularAutomataDemoBuilder.h"

#include <algorithm>
#include <cstdlib>
#include <utility>

#include <QObject>
#include <QPoint>

/*

• Para que a GUI use um novo autômato celular com nova regra local,
  você mexe em 3 camadas:

  1) Implementar a regra no plugin

  - Crie a classe da regra em source/plugins/components/ModalModel/
    CellularAutomata/.
  - Normalmente isso significa algo como:
      - LocalRule_MyRule.h
      - opcionalmente LocalRule_MyRule.cpp
  - A regra precisa herdar de LocalRule e sobrescrever
    applyRule(Cell* cell).
  - Se a regra precisar de vizinhança específica, ela vai ler
    cell->getNeighbors().

  2) Registrar essa regra no builder da GUI

  - O ponto de montagem do demo fica em:
      - source/applications/gui/qt/GenesysQtGUI/cellular_automata/
        CellularAutomataDemoBuilder.h
      - source/applications/gui/qt/GenesysQtGUI/cellular_automata/
        CellularAutomataDemoBuilder.cpp
  - É aqui que a GUI escolhe qual regra concreta instanciar.
  - Você precisa:
      - incluir o header da nova regra;
      - adicionar um valor novo no enum CellularAutomataRulePreset;
      - estender cellularAutomataRulePresetText(...);
      - atualizar makeLocalRule(...) para retornar sua classe.

  3) Expor a escolha na janela

  - A UI de seleção fica em:
      - source/applications/gui/qt/GenesysQtGUI/cellular_automata/
        CellularAutomataViewerWindow.cpp
      - source/applications/gui/qt/GenesysQtGUI/cellular_automata/
        CellularAutomataViewerWindow.h
  - Você precisa adicionar o novo preset ao combo de regra em
    _syncControlsFromController().
  - Se quiser salvar/restaurar essa escolha no JSON, ajuste também:
      - source/applications/gui/qt/GenesysQtGUI/cellular_automata/
        CellularAutomataViewerController.cpp

  Se a nova regra exigir outro tipo de vizinhança ou estado

  - O ajuste continua na mesma área do builder:
      - CellularAutomataDemoBuilder.h/.cpp
  - Para vizinhança, hoje a GUI já suporta:
      - Moore
      - VonNeumann
  - Para tipo de estado, hoje ela expõe:
      - Binary
      - Enumerated
      - Numeric
  - Se sua regra precisar de um estado diferente, você precisa
    ampliar o preset de estado e o mapeamento de cores/menu.

  Fluxo mínimo para fazer funcionar

  1. Criar LocalRule_MyRule.
  2. Incluir essa classe em CellularAutomataDemoBuilder.h.
  3. Adicionar o preset no enum CellularAutomataRulePreset.
  4. Implementar a criação da regra em makeLocalRule(...).
  5. Adicionar o item no combo da janela em
     CellularAutomataViewerWindow.cpp.
  6. Atualizar serialização em CellularAutomataViewerController.cpp
     se quiser persistência.

  Arquivos que você provavelmente vai mexer

  - source/plugins/components/ModalModel/CellularAutomata/
    LocalRule_MyRule.h
  - source/plugins/components/ModalModel/CellularAutomata/
    LocalRule_MyRule.cpp se necessário
  - source/applications/gui/qt/GenesysQtGUI/cellular_automata/
    CellularAutomataDemoBuilder.h
  - source/applications/gui/qt/GenesysQtGUI/cellular_automata/
    CellularAutomataDemoBuilder.cpp
  - source/applications/gui/qt/GenesysQtGUI/cellular_automata/
    CellularAutomataViewerWindow.cpp
  - source/applications/gui/qt/GenesysQtGUI/cellular_automata/
    CellularAutomataViewerController.cpp

*/

namespace {
QSize normalizedSize(QSize size) {
	const int width = std::max(8, size.width());
	const int height = std::max(8, size.height());
	return QSize(width, height);
}

std::vector<State*> enumerableStates(CellularAutomataDemoModel& demo, int count) {
	demo.ownedStates.clear();
	demo.statePointers.clear();
	demo.ownedStates.reserve(static_cast<size_t>(count));
	demo.statePointers.reserve(static_cast<size_t>(count));
	for (int index = 0; index < count; ++index) {
		demo.ownedStates.emplace_back(static_cast<long>(index));
		demo.statePointers.push_back(&demo.ownedStates.back());
	}
	return demo.statePointers;
}

Neighborhood* makeNeighborhood(CellularAutomataDemoModel& demo) {
	switch (demo.settings.neighborhoodPreset) {
	case CellularAutomataNeighborhoodPreset::VonNeumann:
		return new Neighborhood_VonNeumann(demo.automaton.get(), 1, demo.boundary.get());
	case CellularAutomataNeighborhoodPreset::Moore:
	default:
		return new Neighborhood_Moore(demo.automaton.get(), 1, demo.boundary.get());
	}
}

BoundaryCondition* makeBoundary(CellularAutomataDemoModel& demo) {
	switch (demo.settings.boundaryPreset) {
	case CellularAutomataBoundaryPreset::Fixed:
		return new Boundary_Fixed();
	case CellularAutomataBoundaryPreset::Closed:
	default:
		return new Boundary_Closed();
	}
}

LocalRule* makeLocalRule(CellularAutomataDemoModel& demo) {
	switch (demo.settings.rulePreset) {
	case CellularAutomataRulePreset::Growty:
		return new LocalRule_Growty(demo.automaton.get());
	case CellularAutomataRulePreset::Identity:
		demo.identityLocalRule = std::make_unique<LocalRule_CopyCurrentState>(demo.automaton.get());
		return demo.identityLocalRule.get();
	case CellularAutomataRulePreset::ForestFire:
		return new LocalRule_FlorestalFire(demo.automaton.get());
	case CellularAutomataRulePreset::HppLatticeGas:
		return new LocalRule_HppLatticeGas(demo.automaton.get());
	case CellularAutomataRulePreset::GameOfLife:
	default:
		return new LocalRule_GameOfLife(demo.automaton.get());
	}
}

std::vector<long> paintStatesForPreset(CellularAutomataStatePreset preset) {
	switch (preset) {
	case CellularAutomataStatePreset::Enumerated:
		return {0, 1, 2, 3};
	case CellularAutomataStatePreset::Numeric:
		return {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
	case CellularAutomataStatePreset::HppLatticeGas:
		return {0, 1, 2, 4, 8, 5, 10};
	case CellularAutomataStatePreset::Binary:
	default:
		return {0, 1};
	}
}
} // namespace

QString cellularAutomataRulePresetText(CellularAutomataRulePreset preset) {
	switch (preset) {
	case CellularAutomataRulePreset::Growty:
		return QObject::tr("Growty");
	case CellularAutomataRulePreset::Identity:
		return QObject::tr("Identity");
	case CellularAutomataRulePreset::ForestFire:
		return QObject::tr("Forest Fire");
	case CellularAutomataRulePreset::HppLatticeGas:
		return QObject::tr("HPP Lattice Gas");
	case CellularAutomataRulePreset::GameOfLife:
	default:
		return QObject::tr("Game of Life");
	}
}

QString cellularAutomataNeighborhoodPresetText(CellularAutomataNeighborhoodPreset preset) {
	switch (preset) {
	case CellularAutomataNeighborhoodPreset::VonNeumann:
		return QObject::tr("Von Neumann");
	case CellularAutomataNeighborhoodPreset::Moore:
	default:
		return QObject::tr("Moore");
	}
}

QString cellularAutomataBoundaryPresetText(CellularAutomataBoundaryPreset preset) {
	switch (preset) {
	case CellularAutomataBoundaryPreset::Fixed:
		return QObject::tr("Fixed");
	case CellularAutomataBoundaryPreset::Closed:
	default:
		return QObject::tr("Closed / periodic");
	}
}

QString cellularAutomataStatePresetText(CellularAutomataStatePreset preset) {
	switch (preset) {
	case CellularAutomataStatePreset::Enumerated:
		return QObject::tr("Enumerated");
	case CellularAutomataStatePreset::Numeric:
		return QObject::tr("Numeric");
	case CellularAutomataStatePreset::HppLatticeGas:
		return QObject::tr("HPP lattice gas");
	case CellularAutomataStatePreset::Binary:
	default:
		return QObject::tr("Binary");
	}
}

CellularAutomataDemoModel::CellularAutomataDemoModel(CellularAutomataDemoSettings settings)
	: settings(std::move(settings)) {
}

int CellularAutomataDemoModel::columns() const {
	return settings.latticeSize.width();
}

int CellularAutomataDemoModel::rows() const {
	return settings.latticeSize.height();
}

bool CellularAutomataDemoModel::isValidPosition(int x, int y) const {
	return x >= 0 && y >= 0 && x < columns() && y < rows();
}

Cell* CellularAutomataDemoModel::cellAt(int x, int y) const {
	if (!isValidPosition(x, y) || lattice == nullptr) {
		return nullptr;
	}
	return lattice->getCell(std::vector<int>{x, y});
}

long CellularAutomataDemoModel::stateAt(int x, int y) const {
	Cell* cell = cellAt(x, y);
	return cell != nullptr ? cell->getCurrentState().getValue() : 0;
}

QString CellularAutomataDemoModel::stateTextAt(int x, int y) const {
	Cell* cell = cellAt(x, y);
	return cell != nullptr ? QString::fromStdString(cell->getCurrentState().toString()) : QStringLiteral("0");
}

bool CellularAutomataDemoModel::setStateAt(int x, int y, long value) {
	Cell* cell = cellAt(x, y);
	if (cell == nullptr) {
		return false;
	}
	if (settings.statePreset == CellularAutomataStatePreset::HppLatticeGas) {
		HppLatticeGasState state;
		state.setValue(value);
		return cell->setCurrentState(state) && cell->setNextState(state);
	}
	State state(value);
	return cell->setCurrentState(state) && cell->setNextState(state);
}

void CellularAutomataDemoModel::fill(long value) {
	for (int y = 0; y < rows(); ++y) {
		for (int x = 0; x < columns(); ++x) {
			setStateAt(x, y, value);
		}
	}
}

void CellularAutomataDemoModel::seedDefaultPattern() {
	fill(0);

	switch (settings.rulePreset) {
	case CellularAutomataRulePreset::Identity:
		setStateAt(columns() / 2, rows() / 2, 1);
		setStateAt(columns() / 2 + 1, rows() / 2, 2);
		return;
	case CellularAutomataRulePreset::Growty:
		setStateAt(columns() / 2, rows() / 2, 1);
		setStateAt(columns() / 2 - 1, rows() / 2, 1);
		setStateAt(columns() / 2 + 1, rows() / 2, 1);
		return;
	case CellularAutomataRulePreset::ForestFire:
		for (int y = 0; y < rows(); ++y) {
			for (int x = 0; x < columns(); ++x) {
				const int distanceX = std::abs(x - columns() / 2);
				const int distanceY = std::abs(y - rows() / 2);
				const bool inForestCore = distanceX < columns() / 3 && distanceY < rows() / 3;
				const bool inForestWing = distanceX + distanceY < (columns() + rows()) / 3;
				if (inForestCore || inForestWing) {
					setStateAt(x, y, 2);
				} else if ((x + y) % 11 == 0) {
					setStateAt(x, y, 1);
				}
			}
		}
		setStateAt(columns() / 2, rows() / 2, 3);
		setStateAt(columns() / 2 - 1, rows() / 2, 2);
		setStateAt(columns() / 2 + 1, rows() / 2, 2);
		setStateAt(columns() / 2, rows() / 2 - 1, 2);
		setStateAt(columns() / 2, rows() / 2 + 1, 2);
		return;
	case CellularAutomataRulePreset::HppLatticeGas:
		setStateAt(columns() / 2 - 1, rows() / 2, 2);
		setStateAt(columns() / 2 + 1, rows() / 2, 8);
		setStateAt(columns() / 2, rows() / 2 - 1, 4);
		setStateAt(columns() / 2, rows() / 2 + 1, 1);
		return;
	case CellularAutomataRulePreset::GameOfLife:
	default:
		break;
	}

	const int anchorX = columns() / 2;
	const int anchorY = rows() / 2;
	const std::vector<QPoint> glider = {
		QPoint(0, 1),
		QPoint(1, 2),
		QPoint(2, 0),
		QPoint(2, 1),
		QPoint(2, 2),
	};

	for (const QPoint& offset : glider) {
		const int x = anchorX + offset.x() - 1;
		const int y = anchorY + offset.y() - 1;
		if (isValidPosition(x, y)) {
			setStateAt(x, y, 1);
		}
	}
}

std::vector<long> CellularAutomataDemoModel::availablePaintStates() const {
	if (settings.rulePreset == CellularAutomataRulePreset::ForestFire) {
		return {0, 1, 2, 3};
	}
	if (settings.rulePreset == CellularAutomataRulePreset::HppLatticeGas) {
		return paintStatesForPreset(CellularAutomataStatePreset::HppLatticeGas);
	}
	return paintStatesForPreset(settings.statePreset);
}

std::unique_ptr<CellularAutomataDemoModel> CellularAutomataDemoBuilder::buildDemo(CellularAutomataDemoSettings settings) {
	settings.latticeSize = normalizedSize(settings.latticeSize);
	if (settings.rulePreset == CellularAutomataRulePreset::ForestFire) {
		settings.statePreset = CellularAutomataStatePreset::Enumerated;
	}
	if (settings.rulePreset == CellularAutomataRulePreset::HppLatticeGas) {
		settings.statePreset = CellularAutomataStatePreset::HppLatticeGas;
		settings.neighborhoodPreset = CellularAutomataNeighborhoodPreset::VonNeumann;
	}
	auto demo = std::make_unique<CellularAutomataDemoModel>(settings);

	demo->automaton = std::make_unique<CellularAutomata_Classic>();
	demo->lattice = std::make_unique<Lattice>(
		demo->automaton.get(),
		nullptr,
		std::vector<unsigned short>{static_cast<unsigned short>(demo->columns()),
		                            static_cast<unsigned short>(demo->rows())},
		LatticeType::RETICULAR);
	demo->boundary.reset(makeBoundary(*demo));
	demo->neighborhood.reset(makeNeighborhood(*demo));
	demo->localRule.reset(makeLocalRule(*demo));
	if (demo->settings.statePreset == CellularAutomataStatePreset::HppLatticeGas) {
		demo->stateSet = std::make_unique<HppLatticeGasStateSet>(demo->automaton.get());
	} else {
		const int stateCount = demo->settings.rulePreset == CellularAutomataRulePreset::ForestFire
			                           ? 4
			                           : std::max<int>(2, static_cast<int>(demo->availablePaintStates().size()));
		demo->stateSet = std::make_unique<StateSet_Enumerable>(
			demo->automaton.get(),
			enumerableStates(*demo, stateCount));
	}
	demo->localRule->setStateSet(demo->stateSet.get());

	demo->automaton->setLattice(demo->lattice.get());
	demo->automaton->setStateSet(demo->stateSet.get());
	demo->automaton->setNeighborhood(demo->neighborhood.get());
	demo->automaton->setLocalRule(demo->localRule.get());
	demo->boundary->setLattice(demo->lattice.get());

	demo->automaton->init();
	demo->fill(0);
	demo->seedDefaultPattern();
	return demo;
}
