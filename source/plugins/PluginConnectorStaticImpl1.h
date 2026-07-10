#ifndef PLUGINCONNECTORSTATICIMPL1_H
#define PLUGINCONNECTORSTATICIMPL1_H

#include "kernel/simulator/PluginConnector_if.h"

/*!
 * \brief Static plugin connector stub used by the kernel interface.
 *
 * This implementation currently provides a no-op connector that satisfies the
 * plugin connector contract without loading dynamic libraries.
 */
class PluginConnectorStaticImpl1 : public PluginConnector_if {
public:
    PluginConnectorStaticImpl1() = default;
    virtual ~PluginConnectorStaticImpl1() = default;

    /*!
     * \brief Checks whether a plugin library can be connected.
     * \param dynamicLibraryFilename Path to the plugin library.
     * \return Always \c nullptr in the current static stub implementation.
     */
    Plugin* check(const std::string dynamicLibraryFilename) override;
    /*!
     * \brief Connects a plugin library.
     * \param dynamicLibraryFilename Path to the plugin library.
     * \return Always \c nullptr in the current static stub implementation.
     */
    Plugin* connect(const std::string dynamicLibraryFilename) override;
    /*!
     * \brief Lists the available plugin libraries.
     * \return Always \c nullptr in the current static stub implementation.
     */
    List<std::string>* find() override;
    /*!
     * \brief Disconnects a plugin by library filename.
     * \param dynamicLibraryFilename Path to the plugin library.
     * \return Always \c true in the current static stub implementation.
     */
    bool disconnect(const std::string dynamicLibraryFilename) override;
    /*!
     * \brief Disconnects a plugin instance.
     * \param plugin Plugin instance to disconnect.
     * \return Always \c true in the current static stub implementation.
     */
    bool disconnect(Plugin* plugin) override;
};

#endif
