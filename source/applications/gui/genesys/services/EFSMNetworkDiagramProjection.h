#ifndef EFSMNETWORKDIAGRAMPROJECTION_H
#define EFSMNETWORKDIAGRAMPROJECTION_H

class ModelGraphicsScene;
class Simulator;

namespace EFSMNetworkDiagramProjection {

/**
 * @brief Rebuilds the read-only EFSM relation layer for the scene's context.
 *
 * States are represented by the existing non-owning data-definition items;
 * only formal transition graphics are owned by the scene. An empty or
 * non-EFSM context clears the relation layer without changing the model.
 */
void synchronize(Simulator* simulator, ModelGraphicsScene* scene);

} // namespace EFSMNetworkDiagramProjection

#endif
