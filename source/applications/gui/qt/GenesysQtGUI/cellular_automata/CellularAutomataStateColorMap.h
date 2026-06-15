#pragma once

#include <QColor>
#include <QString>
#include <functional>

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

	static QColor colorForStateText(const QString& stateText, long fallbackState) {
		if (stateText.isEmpty() || stateText == QString::number(fallbackState)) {
			return colorForState(fallbackState);
		}
		const size_t hash = std::hash<std::string>{}(stateText.toStdString());
		return QColor::fromHsv(static_cast<int>(hash % 360), 170, 220);
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
