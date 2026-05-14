#pragma once

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




#include <memory>
#include <vector>

#include <QSize>

#include "plugins/components/ModalModel/CellularAutomata/Boundary_Closed.h"
#include "plugins/components/ModalModel/CellularAutomata/Cell.h"
#include "plugins/components/ModalModel/CellularAutomata/CellularAutomata_Classic.h"
#include "plugins/components/ModalModel/CellularAutomata/Lattice.h"
#include "plugins/components/ModalModel/CellularAutomata/LocalRule.h"
#include "plugins/components/ModalModel/CellularAutomata/LocalRule_GameOfLife.h"
#include "plugins/components/ModalModel/CellularAutomata/LocalRule_FlorestalFire.h"
#include "plugins/components/ModalModel/CellularAutomata/LocalRule_Growty.h"
#include "plugins/components/ModalModel/CellularAutomata/Neighborhood_Moore.h"
#include "plugins/components/ModalModel/CellularAutomata/Neighborhood_VonNeumann.h"
#include "plugins/components/ModalModel/CellularAutomata/StateSet_Enumerable.h"

enum class CellularAutomataRulePreset {
	GameOfLife = 0,
	Growty = 1,
	Identity = 2,
	ForestFire = 3
};

enum class CellularAutomataNeighborhoodPreset {
	Moore = 0,
	VonNeumann = 1
};

enum class CellularAutomataStatePreset {
	Binary = 0,
	Enumerated = 1,
	Numeric = 2
};

struct CellularAutomataDemoSettings {
	QSize latticeSize = QSize{32, 24};
	CellularAutomataRulePreset rulePreset = CellularAutomataRulePreset::GameOfLife;
	CellularAutomataNeighborhoodPreset neighborhoodPreset = CellularAutomataNeighborhoodPreset::Moore;
	CellularAutomataStatePreset statePreset = CellularAutomataStatePreset::Binary;
};

QString cellularAutomataRulePresetText(CellularAutomataRulePreset preset);
QString cellularAutomataNeighborhoodPresetText(CellularAutomataNeighborhoodPreset preset);
QString cellularAutomataStatePresetText(CellularAutomataStatePreset preset);

class LocalRule_CopyCurrentState final : public LocalRule {
public:
	explicit LocalRule_CopyCurrentState(CellularAutomataBase* parentCellularAutomata)
		: LocalRule(parentCellularAutomata) {
	}

	void applyRule(Cell* cell) override {
		if (cell != nullptr) {
			cell->setNextState(cell->getCurrentState());
		}
	}
};

struct CellularAutomataDemoModel {
	explicit CellularAutomataDemoModel(CellularAutomataDemoSettings settings = {});

	CellularAutomataDemoSettings settings;
	std::vector<State> ownedStates;
	std::vector<State*> statePointers;
	std::unique_ptr<CellularAutomata_Classic> automaton;
	std::unique_ptr<Lattice> lattice;
	std::unique_ptr<StateSet_Enumerable> stateSet;
	std::unique_ptr<Boundary_Closed> boundary;
	std::unique_ptr<Neighborhood> neighborhood;
	std::unique_ptr<LocalRule> localRule;
	std::unique_ptr<LocalRule_CopyCurrentState> identityLocalRule;

	int columns() const;
	int rows() const;
	bool isValidPosition(int x, int y) const;
	Cell* cellAt(int x, int y) const;
	long stateAt(int x, int y) const;
	bool setStateAt(int x, int y, long value);
	void fill(long value);
	void seedDefaultPattern();
	std::vector<long> availablePaintStates() const;
};

class CellularAutomataDemoBuilder {
public:
	static std::unique_ptr<CellularAutomataDemoModel> buildDemo(CellularAutomataDemoSettings settings = {});
};
