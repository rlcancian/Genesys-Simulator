#pragma once

#include <QColor>

class CellularAutomataStateColorMap {
public:
	static QColor colorForState(long state) {
		if (state <= 0) {
			return QColor(248, 248, 248);
		}
		if (state == 1) {
			return QColor(28, 30, 36);
		}

		const int hue = static_cast<int>((state * 47) % 360);
		return QColor::fromHsv(hue, 180, 215);
	}

	static QColor backgroundColor() {
		return QColor(238, 239, 242);
	}

	static QColor gridLineColor() {
		return QColor(190, 194, 201);
	}

	static QColor emptyTextColor() {
		return QColor(110, 115, 124);
	}
};
