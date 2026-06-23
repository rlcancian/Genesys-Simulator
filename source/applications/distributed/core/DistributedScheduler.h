#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace genesys::distributed {

/** @brief An execution target a batch can be assigned to (a remote worker or the local engine). */
struct SchedulerTarget {
    std::string endpoint;   // "host:port" for remote workers; descriptive label for local.
    bool isLocal = false;
};

/** @brief A contiguous share of replications assigned to a single target, with its own seed. */
struct ReplicationBatch {
    std::string targetEndpoint;
    bool isLocal = false;
    unsigned int numberOfReplications = 0;
    std::uint32_t seed = 0;
};

/**
 * @brief Partitions a total replication count across execution targets (round-robin even split).
 *
 * The split is deterministic: the first `N % M` targets receive one extra replication. Targets
 * that would receive zero replications are omitted. Each emitted batch gets a distinct seed
 * derived from the base seed so partial runs stay independent and reproducible. The sum of all
 * batch replications always equals the requested total (including any local target).
 */
class DistributedScheduler {
public:
    /** @brief Default base seed (matches the kernel default); overridden per call as needed. */
    static constexpr std::uint32_t kDefaultBaseSeed = 16021974u;

    std::vector<ReplicationBatch> partition(unsigned int totalReplications,
                                            const std::vector<SchedulerTarget>& targets,
                                            std::uint32_t baseSeed = kDefaultBaseSeed) const;
};

} // namespace genesys::distributed
