#ifndef EFSMNETWORKDIAGRAMPROJECTION_H
#define EFSMNETWORKDIAGRAMPROJECTION_H

class ModelGraphicsScene;
class Simulator;
class EFSMNetwork;
struct EFSMNetworkDiagramLayout;

namespace EFSMNetworkDiagramProjection {

/**
 * @brief Rebuilds the read-only EFSM relation layer for the scene's context.
 *
 * States are represented by the existing non-owning data-definition items;
 * only formal transition graphics are owned by the scene. An empty or
 * non-EFSM context clears the relation layer without changing the model.
 */
void synchronize(Simulator* simulator, ModelGraphicsScene* scene);

/** @brief Captures the presentation state currently represented by a Network scene. */
bool captureLayout(EFSMNetwork* network, ModelGraphicsScene* scene, EFSMNetworkDiagramLayout* layout);

/** @brief Applies validated persisted state positions to a matching Network scene. */
void restoreLayout(Simulator* simulator, EFSMNetwork* network, ModelGraphicsScene* scene,
                   const EFSMNetworkDiagramLayout& layout);

} // namespace EFSMNetworkDiagramProjection

#endif
