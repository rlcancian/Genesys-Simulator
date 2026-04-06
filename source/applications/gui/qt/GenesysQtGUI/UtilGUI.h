#ifndef UTILGUI_H
#define UTILGUI_H
#include <QColor>
#include <cstdint>
#include <sstream>
#include <string>

namespace UtilGUI {

inline QColor rgbaFromPacked(uint64_t color) {
	uint8_t r = (color & 0xFF000000) >> 24;
	uint8_t g = (color & 0x00FF0000) >> 16;
	uint8_t b = (color & 0x0000FF00) >> 8;
	uint8_t a = (color & 0x000000FF);
	return QColor(r, g, b, a);
}

inline std::string dotColorFromPacked(uint64_t color) {
	std::stringstream stream;
	stream << std::hex << "#" << color;
	return stream.str();
}

}

#endif // UTILGUI_H
