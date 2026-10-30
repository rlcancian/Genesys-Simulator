#ifndef EFSMNETWORKDIAGRAMCODEC_H
#define EFSMNETWORKDIAGRAMCODEC_H

#include "EFSMNetworkDiagramLayout.h"

#include <QHash>
#include <QStringList>

class QTextStream;
class Simulator;
class Model;

/**
 * @brief Encodes/decodes the "Graphical EFSM Network Diagrams" `.gui` overlay section.
 *
 * This is a bounded, formalism-specific codec deliberately kept separate from
 * the generic `GraphicalModelSerializer` so the shared serializer stays
 * agnostic of concrete network formalisms (EFSM, Graph, DTMC, CPN). It writes
 * the smallest versioned record that can recover, deterministically: the
 * identity of the associated EFSMNetwork, the identity and position of its
 * states, and the validated sequence identity for its transitions (including
 * an exact empty-name comparison; see
 * `EFSMNetworkDiagramLayout`). It never reads or writes mathematical state,
 * and it never creates kernel objects -- malformed or unresolved entries are
 * skipped, not substituted.
 */
namespace EFSMNetworkDiagramCodec {

/** @brief Current supported schema version for the diagram record. */
constexpr int kSupportedSchemaVersion = 1;

/** @brief `.gui` section name, matching the existing `# <Section Name>` convention. */
constexpr const char* kSectionName = "Graphical EFSM Network Diagrams";

/**
 * @brief Writes one record block per network present in `layouts` whose
 * EFSMNetwork (and, per entry, states/transitions) still resolve against
 * `simulator`'s current model. Entries that no longer resolve are silently
 * skipped (not written), never causing a save failure.
 */
void writeSection(QTextStream& out, Simulator* simulator,
                   const QHash<unsigned long, EFSMNetworkDiagramLayout>& layouts);

/**
 * @brief Parses previously collected section lines (already stripped of the
 * leading "# " comment prefix) and resolves them against the just-loaded
 * `model`. Unknown schema versions, malformed lines and unresolved
 * state/transition references are skipped with a trace diagnostic; no kernel
 * object is ever created or mutated by this function.
 */
QHash<unsigned long, EFSMNetworkDiagramLayout> parseSection(const QStringList& lines, Model* model);

} // namespace EFSMNetworkDiagramCodec

#endif // EFSMNETWORKDIAGRAMCODEC_H
