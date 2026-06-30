#ifndef DISTRIBUTION_IF_H
#define DISTRIBUTION_IF_H

#include <string>

/**
 * @brief Base abstraction for a theoretical probability distribution.
 */
class Distribution_if {
public:
	virtual ~Distribution_if() = default;
	virtual std::string name() const = 0;
};

#endif /* DISTRIBUTION_IF_H */
