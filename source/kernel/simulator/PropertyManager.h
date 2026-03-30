#ifndef PROPERTYMANAGER_H
#define PROPERTYMANAGER_H

// PropertyManager is currently a placeholder only.
// Do not include Property.h here, or this header may collide with the
// transitional PropertyBase alias used by the kernel-side control API.

/**
 * @brief PropertyManager is currently a placeholder for future application-side property coordination.
 *
 * The intended direction is to keep SimulationResponse/SimulationControl as
 * kernel-side abstractions for experiment/control access, while Property*
 * evolves toward user-facing parameter editing and property-grid integration
 * in the application layer.
 */
class PropertyManager {
public:
	PropertyManager();
public:
private:
};

#endif // PROPERTYMANAGER_H
