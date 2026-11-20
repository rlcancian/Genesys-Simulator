#include "../../GuiExtensionContracts.h"
#include "../../GuiExtensionPluginCatalog.h"
#include "../../../graphicals/GraphicalModelDataDefinition.h"
#include "../../../graphicals/ModelGraphicsScene.h"

#include "plugins/data/BiochemicalSimulation/BioReaction.h"
#include "plugins/data/BiochemicalSimulation/BioSpecies.h"
#include "kernel/simulator/model/Model.h"

#include <QAction>
#include <QGraphicsItem>
#include <QMenu>
#include <QObject>

#include <utility>
#include <vector>

namespace {

bool collectBioReactionSelection(ModelGraphicsScene* scene,
                                 BioReaction*& reaction,
                                 std::vector<BioSpecies*>& speciesDefinitions) {
    reaction = nullptr;
    speciesDefinitions.clear();
    if (scene == nullptr) {
        return false;
    }

    const QList<QGraphicsItem*> selectedItems = scene->selectedItems();
    if (selectedItems.size() < 2) {
        return false;
    }

    for (QGraphicsItem* item : selectedItems) {
        auto* graphicalDefinition = dynamic_cast<GraphicalModelDataDefinition*>(item);
        if (graphicalDefinition == nullptr) {
            return false;
        }

        ModelDataDefinition* dataDefinition = graphicalDefinition->getDataDefinition();
        if (auto* candidateReaction = dynamic_cast<BioReaction*>(dataDefinition)) {
            if (reaction != nullptr) {
                return false;
            }
            reaction = candidateReaction;
            continue;
        }

        auto* species = dynamic_cast<BioSpecies*>(dataDefinition);
        if (species == nullptr) {
            return false;
        }
        speciesDefinitions.push_back(species);
    }

    return reaction != nullptr && !speciesDefinitions.empty();
}

bool reactionAlreadyContainsReactant(BioReaction* reaction, const std::string& speciesName) {
    for (const auto& term : reaction->getReactants()) {
        if (term.speciesName == speciesName) {
            return true;
        }
    }
    return false;
}

bool reactionAlreadyContainsProduct(BioReaction* reaction, const std::string& speciesName) {
    for (const auto& term : reaction->getProducts()) {
        if (term.speciesName == speciesName) {
            return true;
        }
    }
    return false;
}

bool reactionAlreadyContainsModifier(BioReaction* reaction, const std::string& speciesName) {
    for (const std::string& modifierName : reaction->getModifiers()) {
        if (modifierName == speciesName) {
            return true;
        }
    }
    return false;
}


void populateBioReactionLinkMenu(QMenu* menu, ModelGraphicsScene* scene) {
    if (menu == nullptr || scene == nullptr) {
        return;
    }

    BioReaction* reaction = nullptr;
    std::vector<BioSpecies*> selectedSpecies;
    if (!collectBioReactionSelection(scene, reaction, selectedSpecies)) {
        return;
    }

    QMenu* relationMenu = menu->addMenu("Biochemical Relation");
    QAction* addReactantsAction = relationMenu->addAction("Add Selected BioSpecies as Reactants");
    QAction* addProductsAction = relationMenu->addAction("Add Selected BioSpecies as Products");
    QAction* addModifiersAction = relationMenu->addAction("Add Selected BioSpecies as Modifiers");

    // All three actions mutate the same BioReaction and rely on the generic scene sync to rebuild diagram links.
    auto refreshAfterChange = [scene, reaction]() {
        reaction->setHasChanged(true);
        if (reaction->getParentModel() != nullptr) {
            reaction->getParentModel()->setHasChanged(true);
        }
        scene->requestGraphicalDataDefinitionsSync();
        scene->update();
    };

    QObject::connect(addReactantsAction, &QAction::triggered, relationMenu, [reaction, selectedSpecies, refreshAfterChange]() {
        bool changed = false;
        for (BioSpecies* species : selectedSpecies) {
            if (species == nullptr || reactionAlreadyContainsReactant(reaction, species->getName())) {
                continue;
            }
            reaction->addReactant(species->getName(), 1.0);
            changed = true;
        }
        if (changed) {
            refreshAfterChange();
        }
    });

    QObject::connect(addProductsAction, &QAction::triggered, relationMenu, [reaction, selectedSpecies, refreshAfterChange]() {
        bool changed = false;
        for (BioSpecies* species : selectedSpecies) {
            if (species == nullptr || reactionAlreadyContainsProduct(reaction, species->getName())) {
                continue;
            }
            reaction->addProduct(species->getName(), 1.0);
            changed = true;
        }
        if (changed) {
            refreshAfterChange();
        }
    });

    QObject::connect(addModifiersAction, &QAction::triggered, relationMenu, [reaction, selectedSpecies, refreshAfterChange]() {
        bool changed = false;
        for (BioSpecies* species : selectedSpecies) {
            if (species == nullptr || reactionAlreadyContainsModifier(reaction, species->getName())) {
                continue;
            }
            reaction->addModifier(species->getName());
            changed = true;
        }
        if (changed) {
            refreshAfterChange();
        }
    });
}

} // namespace

// The existing GUI contribution registry invokes this code only when the
// biochemical model plugins are available. The generic context-menu controller
// does not know any BioReaction/BioSpecies concrete types.
class BioReactionContextMenuGuiExtensionPlugin final : public GuiExtensionPlugin {
public:
    std::string extensionId() const override {
        return "gui.extensions.bio.reaction.context";
    }

    std::string displayName() const override {
        return "BioReaction Context Relations";
    }

    std::string version() const override {
        return "1.0.0";
    }

    std::vector<std::string> requiredModelPlugins() const override {
        return {"BioReaction", "BioSpecies"};
    }

    void registerContributions(GuiExtensionRegistry* registry) const override {
        if (registry == nullptr) return;

        GuiContextMenuContribution contextMenu;
        contextMenu.id = "gui.bio.reaction.link-context";
        contextMenu.isVisible = [](const GuiExtensionRuntimeContext& context, QGraphicsItem*) {
            BioReaction* reaction = nullptr;
            std::vector<BioSpecies*> selectedSpecies;
            return collectBioReactionSelection(context.graphicsScene, reaction, selectedSpecies);
        };
        contextMenu.populate = [](QMenu* menu, const GuiExtensionRuntimeContext& context, QGraphicsItem*) {
            populateBioReactionLinkMenu(menu, context.graphicsScene);
        };
        registry->addContextMenu(std::move(contextMenu));
    }
};

REGISTER_GUI_EXTENSION_PLUGIN(BioReactionContextMenuGuiExtensionPlugin);
