#pragma once

#include <string>

/**
 * @brief Maps persisted plugin IDs retired by source-level class renames.
 *
 * These aliases are deliberately limited to the three names migrated in the
 * 2026-11-15 minor-changes task. They affect lookup while loading only; newly
 * created objects continue to use their current plugin IDs.
 */
inline std::string currentPluginTypeForPersistedType(const std::string& type) {
	if (type == "DummyComponent") return "TemplateComponent";
	if (type == "DummyElement") return "TemplateDataDefinition";
	if (type == "ModalModelDefault") return "ModalModel";
	return type;
}
