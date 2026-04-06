#ifndef UTILGUI_H
#define UTILGUI_H
#include <QColor>
#include <cstdint>
#include <sstream>
#include <string>

namespace UtilGUI {

/**
 * @brief Converts a packed RGBA integer (0xRRGGBBAA) to QColor.
 * @param color Packed RGBA value.
 * @return QColor built from packed channels.
 *
 * @todo Audit TraitsGUI color constants to ensure all follow 0xRRGGBBAA convention.
 */
inline QColor rgbaFromPacked(uint64_t color) {
	uint8_t r = (color & 0xFF000000) >> 24;
	uint8_t g = (color & 0x00FF0000) >> 16;
	uint8_t b = (color & 0x0000FF00) >> 8;
	uint8_t a = (color & 0x000000FF);
	return QColor(r, g, b, a);
}

/**
 * @brief Converts a packed integer color into Graphviz hexadecimal color string.
 * @param color Packed color value.
 * @return String in format `#<hex>`.
 *
 * @todo Validate whether output should be normalized to 6 or 8 hex digits.
 */
inline std::string dotColorFromPacked(uint64_t color) {
	std::stringstream stream;
	stream << std::hex << "#" << color;
	return stream.str();
}

}

#endif // UTILGUI_H
