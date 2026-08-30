/*
 * File:   NetworkActivation.h
 * Author: Prof. Rafael Luiz Cancian, Dr. Eng.
 *
 * Created on 30 de Agosto de 2026
 */

#ifndef NETWORKACTIVATION_H
#define NETWORKACTIVATION_H

#include <vector>

/*!
 * \brief Presence/value pair carried by one network port during one
 * activation.
 *
 * See `docs/ai_assistants/reference/GENESYS_MODAL_MODEL_NETWORK_ARCHITECTURE.md`
 * §8/§12: an absent port is not equivalent to a present port whose value is
 * zero, and is not equivalent to reusing a previous value. `value` is only
 * meaningful when `present` is `true`.
 *
 * The value is numeric (`double`) in this first implementation, matching
 * the current GenESyS parser/expression contract (§20 of the architecture
 * reference); this does not prevent a future typed/vector/matrix port
 * value, which would extend this struct rather than replace its presence
 * semantics.
 */
struct NetworkPortValue {
	bool present = false;
	double value = 0.0;
};

/*!
 * \brief Input side of one `DefaultNetwork::activate()` call: the presence
 * and value of every declared input port for this activation only.
 *
 * A frame is a short-lived, per-activation value object. It is not
 * persistent network state (see `DefaultNetwork::getLastInputValue()` for
 * the separate, optional last-observed-value concept described in
 * architecture reference §13).
 *
 * The ordinary GenESyS `ModalModel` adapter normally marks only the input
 * port that received the arriving entity as present (architecture
 * reference §12); this class itself places no such restriction, so a
 * future multi-input activation remains representable.
 */
class NetworkActivationFrame {
public:
	explicit NetworkActivationFrame(unsigned int numInputs = 0);
public:
	/*! \brief Resizes the frame, clearing every input to absent. */
	void reset(unsigned int numInputs);
	unsigned int size() const;
	/*! \brief Marks input \p index present with \p value. No-op when \p index is out of range. */
	void setPresent(unsigned int index, double value);
	/*! \brief Marks input \p index absent (value reads back as 0.0). No-op when \p index is out of range. */
	void setAbsent(unsigned int index);
	bool isPresent(unsigned int index) const;
	/*! \brief Returns the input value, or 0.0 when absent or out of range. */
	double getValue(unsigned int index) const;
private:
	std::vector<NetworkPortValue> _inputs;
};

/*!
 * \brief Output side of one `DefaultNetwork::activate()` call: the
 * presence and value of every declared output port produced by that
 * activation.
 *
 * See architecture reference §15/§17: only present outputs cause the
 * `ModalModel` adapter to route/consume/clone the process entity. An
 * absent output is not interpreted as zero.
 */
class NetworkActivationResult {
public:
	explicit NetworkActivationResult(unsigned int numOutputs = 0);
public:
	/*! \brief Resizes the result, clearing every output to absent. */
	void reset(unsigned int numOutputs);
	unsigned int size() const;
	void setPresent(unsigned int index, double value);
	void setAbsent(unsigned int index);
	bool isPresent(unsigned int index) const;
	double getValue(unsigned int index) const;
	/*! \brief Returns how many output ports are currently present. */
	unsigned int countPresent() const;
private:
	std::vector<NetworkPortValue> _outputs;
};

#endif /* NETWORKACTIVATION_H */
