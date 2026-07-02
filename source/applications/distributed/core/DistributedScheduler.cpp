#include "DistributedScheduler.h"

namespace genesys::distributed {

std::vector<ReplicationBatch> DistributedScheduler::partition(
    unsigned int totalReplications,
    const std::vector<SchedulerTarget>& targets,
    std::uint32_t baseSeed) const {
    std::vector<ReplicationBatch> batches;
    const std::size_t targetCount = targets.size();
    if (totalReplications == 0 || targetCount == 0) {
        return batches;
    }

    const unsigned int base = totalReplications / static_cast<unsigned int>(targetCount);
    const unsigned int remainder = totalReplications % static_cast<unsigned int>(targetCount);

    for (std::size_t index = 0; index < targetCount; ++index) {
        // The first `remainder` targets absorb one extra replication so the split is exact.
        const unsigned int share = base + (index < remainder ? 1u : 0u);
        if (share == 0) {
            continue;  // Fewer replications than targets: skip empty assignments.
        }

        ReplicationBatch batch;
        batch.targetEndpoint = targets[index].endpoint;
        batch.isLocal = targets[index].isLocal;
        batch.numberOfReplications = share;
        // Distinct, reproducible seed per target so batches draw independent streams.
        batch.seed = baseSeed + static_cast<std::uint32_t>(index);
        batches.push_back(batch);
    }

    return batches;
}

} // namespace genesys::distributed
