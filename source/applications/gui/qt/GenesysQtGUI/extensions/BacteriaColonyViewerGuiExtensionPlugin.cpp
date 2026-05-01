#include "GuiExtensionPluginCatalog.h"

#include "kernel/simulator/ComponentManager.h"
#include "kernel/simulator/Model.h"
#include "kernel/simulator/ModelManager.h"
#include "kernel/simulator/Simulator.h"
#include "graphicals/GraphicalModelComponent.h"
#include "graphicals/ModelGraphicsScene.h"
#include "plugins/components/BiologicalModeling/BacteriaColony.h"
#include "plugins/data/BiologicalModeling/BacteriaSignalGrid.h"

#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QGraphicsItem>
#include <QHBoxLayout>
#include <QLabel>
#include <QMainWindow>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QPushButton>
#include <QStringList>
#include <QSpinBox>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <map>
#include <functional>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace {

struct BacteriumVisualState {
	unsigned int id = 0;
	unsigned int generation = 0;
	unsigned int gridX = 0;
	unsigned int gridY = 0;
	double positionX = 0.0;
	double positionY = 0.0;
	double directionRadians = 0.0;
	double volume = 1.0;
	double size = 1.0;
	double gfp = 0.0;
	double rfp = 0.0;
	double yfp = 0.0;
	double cfp = 0.0;
	double age = 0.0;
	QString programName;
	QString runtimeVariablesSummary;
};

struct ColonyVisualSnapshot {
	bool valid = false;
	QString colonyName;
	QString bioNetworkName;
	QString signalGridName;
	QString statusMessage;
	double colonyTime = 0.0;
	double minSignal = 0.0;
	double maxSignal = 0.0;
	unsigned int populationSize = 0;
	unsigned int internalBacteriaCount = 0;
	unsigned int gridWidth = 1;
	unsigned int gridHeight = 1;
	unsigned int maxBacteriaPerCell = 0;
	std::vector<double> signalValues;
	std::vector<BacteriumVisualState> bacteria;
};

struct BacteriumTrailSample {
	double positionX = 0.0;
	double positionY = 0.0;
	double colonyTime = 0.0;
};

struct RenderedBacteriumMarker {
	unsigned int id = 0;
	QPointF center;
	double radius = 0.0;
};

BacteriaColony* selectedBacteriaColonyFromCanvas(const GuiExtensionRuntimeContext& context) {
	if (context.graphicsScene == nullptr) {
		return nullptr;
	}
	const QList<QGraphicsItem*> selectedItems = context.graphicsScene->selectedItems();
	if (selectedItems.size() != 1) {
		return nullptr;
	}

	auto* graphicalComponent = dynamic_cast<GraphicalModelComponent*>(selectedItems.front());
	if (graphicalComponent == nullptr) {
		return nullptr;
	}
	return dynamic_cast<BacteriaColony*>(graphicalComponent->getComponent());
}

QColor signalColorForValue(double value, double minValue, double maxValue) {
	if (!std::isfinite(value)) {
		return QColor(96, 96, 96);
	}
	if (maxValue - minValue < 1e-12) {
		return value > 0.0 ? QColor(64, 191, 214, 160) : QColor(246, 248, 250);
	}
	const double normalized = std::clamp((value - minValue) / (maxValue - minValue), 0.0, 1.0);
	const int red = static_cast<int>(28.0 + normalized * 210.0);
	const int green = static_cast<int>(66.0 + normalized * 92.0);
	const int blue = static_cast<int>(112.0 + (1.0 - normalized) * 108.0);
	const int alpha = static_cast<int>(24.0 + normalized * 210.0);
	return QColor(red, green, blue, alpha);
}

QColor bacteriumColorForState(const BacteriumVisualState& bacterium) {
	const double intensity = std::clamp(0.02 * bacterium.volume + 0.015 * bacterium.size, 0.0, 1.0);
	const int red = std::clamp(static_cast<int>(70.0 + bacterium.rfp * 0.44 + bacterium.yfp * 0.28 + bacterium.generation * 8.0 + intensity * 28.0), 0, 255);
	const int green = std::clamp(static_cast<int>(82.0 + bacterium.gfp * 0.36 + bacterium.yfp * 0.34 + bacterium.cfp * 0.20 + intensity * 18.0), 0, 255);
	const int blue = std::clamp(static_cast<int>(78.0 + bacterium.cfp * 0.44 + bacterium.yfp * 0.20 + bacterium.gfp * 0.12 + intensity * 26.0), 0, 255);
	return QColor(red, green, blue);
}

class BacteriaColonyCanvas final : public QWidget {
public:
	using SelectionChangedCallback = std::function<void(std::optional<unsigned int>)>;

	explicit BacteriaColonyCanvas(QWidget* parent = nullptr) : QWidget(parent) {
		setMinimumSize(480, 360);
	}

	void setSelectedBacteriumId(std::optional<unsigned int> selectedBacteriumId) {
		_selectedBacteriumId = selectedBacteriumId;
		update();
	}

	void setSelectionChangedCallback(SelectionChangedCallback callback) {
		_selectionChangedCallback = std::move(callback);
	}

	void setSnapshot(ColonyVisualSnapshot snapshot,
	                 std::map<unsigned int, std::vector<BacteriumTrailSample>> trailsByBacteriumId) {
		_snapshot = std::move(snapshot);
		_trailsByBacteriumId = std::move(trailsByBacteriumId);
		update();
	}

protected:
	void paintEvent(QPaintEvent* event) override {
		Q_UNUSED(event)

		_renderedMarkers.clear();
		QPainter painter(this);
		painter.setRenderHint(QPainter::Antialiasing, true);
		painter.fillRect(rect(), QColor(246, 248, 250));

		if (!_snapshot.valid) {
			painter.setPen(QColor(70, 70, 70));
			painter.drawText(rect().adjusted(24, 24, -24, -24),
			                 Qt::AlignCenter | Qt::TextWordWrap,
			                 _snapshot.statusMessage.isEmpty()
			                     ? tr("No BacteriaColony available in current model.")
			                     : _snapshot.statusMessage);
			return;
		}

		const unsigned int gridWidth = std::max(1u, _snapshot.gridWidth);
		const unsigned int gridHeight = std::max(1u, _snapshot.gridHeight);
		const QRectF drawingRect = rect().adjusted(24, 24, -140, -24);
		const double axisLabelSpaceX = 26.0;
		const double axisLabelSpaceY = 20.0;
		const QRectF gridFrame = drawingRect.adjusted(axisLabelSpaceX, axisLabelSpaceY, -8.0, -8.0);
		const double cellWidth = gridFrame.width() / static_cast<double>(gridWidth);
		const double cellHeight = gridFrame.height() / static_cast<double>(gridHeight);
		const auto cellRectFor = [&gridFrame, cellWidth, cellHeight](unsigned int x, unsigned int y) {
			return QRectF(gridFrame.left() + static_cast<double>(x) * cellWidth,
			              gridFrame.top() + static_cast<double>(y) * cellHeight,
			              cellWidth,
			              cellHeight);
		};

		painter.setPen(Qt::NoPen);
		painter.setBrush(QColor(239, 244, 247));
		painter.drawRoundedRect(gridFrame.adjusted(-3.0, -3.0, 3.0, 3.0), 8.0, 8.0);

		for (unsigned int y = 0; y < gridHeight; ++y) {
			for (unsigned int x = 0; x < gridWidth; ++x) {
				const QRectF cellRect = cellRectFor(x, y);
				const std::size_t index = static_cast<std::size_t>(y) * static_cast<std::size_t>(gridWidth) + x;
				const double value = index < _snapshot.signalValues.size() ? _snapshot.signalValues[index] : 0.0;
				const QColor signalColor = signalColorForValue(value, _snapshot.minSignal, _snapshot.maxSignal);
				if (signalColor.alpha() > 0) {
					painter.fillRect(cellRect, signalColor);
				}
			}
		}

		QPen majorGridPen(QColor(15, 23, 42, 44));
		majorGridPen.setCosmetic(true);
		majorGridPen.setWidthF(0.95);
		QPen borderPen(QColor(15, 23, 42, 72));
		borderPen.setCosmetic(true);
		borderPen.setWidthF(1.15);

		const unsigned int xLabelStep = std::max(1u, gridWidth / 4u);
		const unsigned int yLabelStep = std::max(1u, gridHeight / 4u);
		painter.setBrush(Qt::NoBrush);
		for (unsigned int x = 0; x <= gridWidth; ++x) {
			if (x != 0 && x != gridWidth && (xLabelStep == 0 || x % xLabelStep != 0)) {
				continue;
			}
			painter.setPen(majorGridPen);
			const double px = gridFrame.left() + static_cast<double>(x) * cellWidth;
			painter.drawLine(QLineF(px, gridFrame.top(), px, gridFrame.bottom()));
		}
		for (unsigned int y = 0; y <= gridHeight; ++y) {
			if (y != 0 && y != gridHeight && (yLabelStep == 0 || y % yLabelStep != 0)) {
				continue;
			}
			painter.setPen(majorGridPen);
			const double py = gridFrame.top() + static_cast<double>(y) * cellHeight;
			painter.drawLine(QLineF(gridFrame.left(), py, gridFrame.right(), py));
		}
		painter.setPen(borderPen);
		painter.drawRect(gridFrame);

		// Match the original GRO viewer more closely by emphasizing the signal
		// area bounds instead of dense per-cell grid lines.
		const double armLength = std::min(18.0, std::min(cellWidth, cellHeight) * 1.6);
		const auto drawCornerCross = [&painter, armLength](const QPointF& corner, double horizontalDir, double verticalDir) {
			painter.drawLine(corner, QPointF(corner.x() + horizontalDir * armLength, corner.y()));
			painter.drawLine(corner, QPointF(corner.x(), corner.y() + verticalDir * armLength));
		};
		painter.setPen(QPen(QColor(15, 23, 42, 120), 1.2));
		drawCornerCross(gridFrame.topLeft(), 1.0, 1.0);
		drawCornerCross(gridFrame.topRight(), -1.0, 1.0);
		drawCornerCross(gridFrame.bottomLeft(), 1.0, -1.0);
		drawCornerCross(gridFrame.bottomRight(), -1.0, -1.0);

		QFont axisFont = painter.font();
		axisFont.setPointSizeF(std::max(8.0, axisFont.pointSizeF() > 0.0 ? axisFont.pointSizeF() - 1.0 : 8.0));
		painter.setFont(axisFont);
		painter.setPen(QColor(71, 85, 105));
		for (unsigned int x = 0; x < gridWidth; ++x) {
			if (x % xLabelStep != 0 && x + 1 != gridWidth) {
				continue;
			}
			const QRectF labelRect(gridFrame.left() + static_cast<double>(x) * cellWidth,
			                       drawingRect.top(),
			                       cellWidth,
			                       axisLabelSpaceY);
			painter.drawText(labelRect, Qt::AlignHCenter | Qt::AlignVCenter, QString::number(x));
		}
		for (unsigned int y = 0; y < gridHeight; ++y) {
			if (y % yLabelStep != 0 && y + 1 != gridHeight) {
				continue;
			}
			const QRectF labelRect(drawingRect.left(),
			                       gridFrame.top() + static_cast<double>(y) * cellHeight,
			                       axisLabelSpaceX - 4.0,
			                       cellHeight);
			painter.drawText(labelRect, Qt::AlignRight | Qt::AlignVCenter, QString::number(y));
		}

		const auto positionToPoint = [&gridFrame, cellWidth, cellHeight](double x, double y) {
			const double px = gridFrame.left() + (x + 0.5) * cellWidth;
			const double py = gridFrame.top() + (y + 0.5) * cellHeight;
			return QPointF(px, py);
		};
		constexpr double kPi = 3.14159265358979323846;

		// Draw short bacterium trails before the live markers so motion stays readable.
		for (const BacteriumVisualState& bacterium : _snapshot.bacteria) {
			auto trailIt = _trailsByBacteriumId.find(bacterium.id);
			if (trailIt == _trailsByBacteriumId.end() || trailIt->second.size() < 2) {
				continue;
			}
			const std::vector<BacteriumTrailSample>& samples = trailIt->second;
			for (std::size_t index = 1; index < samples.size(); ++index) {
				const BacteriumTrailSample& previous = samples[index - 1];
				const BacteriumTrailSample& current = samples[index];
				const bool selected = _selectedBacteriumId.has_value() && _selectedBacteriumId.value() == bacterium.id;
				const double alpha = selected
				                     ? 130.0 + 100.0 * (static_cast<double>(index) / static_cast<double>(samples.size()))
				                     : 45.0 + 145.0 * (static_cast<double>(index) / static_cast<double>(samples.size()));
				QPen trailPen(QColor(15, 23, 42, static_cast<int>(alpha)));
				trailPen.setWidthF(selected ? 2.8 : 1.6);
				painter.setPen(trailPen);
				painter.drawLine(positionToPoint(previous.positionX, previous.positionY),
				                 positionToPoint(current.positionX, current.positionY));
			}
		}

		std::vector<const BacteriumVisualState*> bacteriaInRenderOrder;
		bacteriaInRenderOrder.reserve(_snapshot.bacteria.size());
		for (const BacteriumVisualState& bacterium : _snapshot.bacteria) {
			bacteriaInRenderOrder.push_back(&bacterium);
		}
		std::sort(bacteriaInRenderOrder.begin(), bacteriaInRenderOrder.end(), [](const BacteriumVisualState* left, const BacteriumVisualState* right) {
			if (left == nullptr || right == nullptr) {
				return left != nullptr;
			}
			return left->volume > right->volume;
		});

		for (const BacteriumVisualState* bacterium : bacteriaInRenderOrder) {
			if (bacterium == nullptr) {
				continue;
			}
			const double bodyScale = std::max(0.85, std::sqrt(std::max(0.1, bacterium->size)));
			const double majorAxis = std::clamp(cellWidth * (0.56 + bodyScale * 0.22), 9.0, cellWidth * 1.8);
			const double minorAxis = std::clamp(majorAxis * 0.42, 4.5, majorAxis * 0.72);
			const QPointF center = positionToPoint(bacterium->positionX, bacterium->positionY);
			const QColor fillColor = bacteriumColorForState(*bacterium);
			const bool selected = _selectedBacteriumId.has_value() && _selectedBacteriumId.value() == bacterium->id;

			painter.save();
			painter.translate(center);
			painter.rotate(bacterium->directionRadians * 180.0 / kPi);
			QPainterPath shadowPath;
			shadowPath.addRoundedRect(QRectF(-majorAxis * 0.54, -minorAxis * 0.62, majorAxis * 1.08, minorAxis * 1.24),
			                          minorAxis * 0.55,
			                          minorAxis * 0.55);
			painter.setPen(Qt::NoPen);
			painter.setBrush(QColor(fillColor.red(), fillColor.green(), fillColor.blue(), 68));
			painter.drawPath(shadowPath);
			QPainterPath bodyPath;
			bodyPath.addRoundedRect(QRectF(-majorAxis * 0.5, -minorAxis * 0.5, majorAxis, minorAxis),
			                        minorAxis * 0.48,
			                        minorAxis * 0.48);
			painter.setPen(QPen(selected ? QColor(245, 158, 11) : QColor(248, 250, 252), selected ? 2.4 : 1.0));
			painter.setBrush(QColor(fillColor.red(), fillColor.green(), fillColor.blue(), 232));
			painter.drawPath(bodyPath);
			painter.setBrush(QColor(255, 255, 255, std::clamp(static_cast<int>(60 + bacterium->gfp * 26.0), 60, 180)));
			painter.setPen(Qt::NoPen);
			painter.drawEllipse(QPointF(-majorAxis * 0.18, -minorAxis * 0.10), majorAxis * 0.14, minorAxis * 0.14);
			painter.setPen(QPen(QColor(17, 24, 39, selected ? 220 : 180), selected ? 2.0 : 1.4));
			painter.drawLine(QPointF(0.0, 0.0), QPointF(majorAxis * 0.42, 0.0));
			painter.restore();

			if (selected) {
				painter.setPen(QPen(QColor(120, 53, 15, 210), 1.3));
				painter.setBrush(Qt::NoBrush);
				painter.drawEllipse(center, majorAxis * 0.58, minorAxis * 0.58);
			}
			_renderedMarkers.push_back({bacterium->id, center, majorAxis * 0.6});
		}

		_drawLegend(&painter, QRectF(rect().right() - 110.0, rect().top() + 24.0, 86.0, rect().height() - 48.0));
	}

	void mousePressEvent(QMouseEvent* event) override {
		if (event == nullptr || event->button() != Qt::LeftButton) {
			QWidget::mousePressEvent(event);
			return;
		}

		std::optional<unsigned int> selectedBacteriumId;
		double bestDistanceSquared = std::numeric_limits<double>::max();
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
		const QPointF clickPosition = event->position();
#else
		const QPointF clickPosition = event->localPos();
#endif
		for (const RenderedBacteriumMarker& marker : _renderedMarkers) {
			const QPointF delta = clickPosition - marker.center;
			const double distanceSquared = delta.x() * delta.x() + delta.y() * delta.y();
			const double maxDistance = std::max(8.0, marker.radius + 4.0);
			if (distanceSquared <= maxDistance * maxDistance && distanceSquared < bestDistanceSquared) {
				bestDistanceSquared = distanceSquared;
				selectedBacteriumId = marker.id;
			}
		}

		_selectedBacteriumId = selectedBacteriumId;
		if (_selectionChangedCallback) {
			_selectionChangedCallback(_selectedBacteriumId);
		}
		update();
		QWidget::mousePressEvent(event);
	}

private:
	void _drawLegend(QPainter* painter, const QRectF& legendRect) const {
		if (painter == nullptr) {
			return;
		}
		painter->setPen(Qt::NoPen);
		painter->setBrush(QColor(255, 255, 255, 232));
		painter->drawRoundedRect(legendRect, 10.0, 10.0);

		painter->setPen(QColor(30, 41, 59));
		QFont titleFont = painter->font();
		titleFont.setBold(true);
		painter->setFont(titleFont);
		painter->drawText(legendRect.adjusted(8, 8, -8, -8), Qt::AlignTop | Qt::AlignHCenter, tr("Signals"));

		const QRectF gradientRect(legendRect.left() + 24.0, legendRect.top() + 36.0, 20.0, legendRect.height() - 128.0);
		QLinearGradient gradient(gradientRect.topLeft(), gradientRect.bottomLeft());
		gradient.setColorAt(0.0, signalColorForValue(_snapshot.maxSignal, _snapshot.minSignal, _snapshot.maxSignal));
		gradient.setColorAt(1.0, signalColorForValue(_snapshot.minSignal, _snapshot.minSignal, _snapshot.maxSignal));
		painter->fillRect(gradientRect, gradient);
		painter->setPen(QColor(100, 116, 139));
		painter->drawRect(gradientRect);

		QFont bodyFont = painter->font();
		bodyFont.setBold(false);
		painter->setFont(bodyFont);
		painter->setPen(QColor(30, 41, 59));
		painter->drawText(QRectF(gradientRect.right() + 8.0, gradientRect.top() - 6.0, 44.0, 20.0),
		                 Qt::AlignLeft | Qt::AlignVCenter,
		                 QString::number(_snapshot.maxSignal, 'g', 3));
		painter->drawText(QRectF(gradientRect.right() + 8.0, gradientRect.bottom() - 14.0, 44.0, 20.0),
		                 Qt::AlignLeft | Qt::AlignVCenter,
		                 QString::number(_snapshot.minSignal, 'g', 3));

		const QRectF bacteriaTitleRect(legendRect.left() + 8.0, gradientRect.bottom() + 18.0, legendRect.width() - 16.0, 18.0);
		painter->setFont(titleFont);
		painter->drawText(bacteriaTitleRect, Qt::AlignLeft | Qt::AlignVCenter, tr("Bacteria"));

		painter->setFont(bodyFont);
		painter->setPen(QPen(QColor(255, 255, 255), 1.0));
		painter->setBrush(QColor(70, 150, 110));
		painter->drawEllipse(QPointF(legendRect.left() + 20.0, gradientRect.bottom() + 48.0), 8.0, 5.0);
		painter->setPen(QColor(30, 41, 59));
		painter->drawText(QRectF(legendRect.left() + 32.0, gradientRect.bottom() + 38.0, legendRect.width() - 40.0, 18.0),
		                 Qt::AlignLeft | Qt::AlignVCenter,
		                 tr("body / growth"));

		painter->setPen(QPen(QColor(255, 255, 255), 1.0));
		painter->setBrush(QColor(33, 150, 243));
		painter->drawEllipse(QPointF(legendRect.left() + 20.0, gradientRect.bottom() + 72.0), 8.0, 5.0);
		painter->setPen(QColor(30, 41, 59));
		painter->drawText(QRectF(legendRect.left() + 32.0, gradientRect.bottom() + 62.0, legendRect.width() - 40.0, 18.0),
		                 Qt::AlignLeft | Qt::AlignVCenter,
		                 tr("GFP"));

		painter->setPen(QPen(QColor(15, 23, 42, 170), 1.6));
		painter->drawLine(QPointF(legendRect.left() + 12.0, gradientRect.bottom() + 94.0),
		                 QPointF(legendRect.left() + 32.0, gradientRect.bottom() + 94.0));
		painter->setPen(QColor(30, 41, 59));
		painter->drawText(QRectF(legendRect.left() + 32.0, gradientRect.bottom() + 84.0, legendRect.width() - 40.0, 18.0),
		                 Qt::AlignLeft | Qt::AlignVCenter,
		                 tr("direction"));

		painter->setPen(QPen(QColor(255, 255, 255), 1.0));
		painter->setBrush(QColor(232, 126, 4));
		painter->drawEllipse(QPointF(legendRect.left() + 20.0, gradientRect.bottom() + 118.0), 8.0, 5.0);
		painter->setPen(QColor(30, 41, 59));
		painter->drawText(QRectF(legendRect.left() + 32.0, gradientRect.bottom() + 108.0, legendRect.width() - 40.0, 18.0),
		                 Qt::AlignLeft | Qt::AlignVCenter,
		                 tr("RFP"));

		painter->setPen(QPen(QColor(255, 255, 255), 1.0));
		painter->setBrush(QColor(224, 183, 0));
		painter->drawEllipse(QPointF(legendRect.left() + 20.0, gradientRect.bottom() + 142.0), 8.0, 5.0);
		painter->setPen(QColor(30, 41, 59));
		painter->drawText(QRectF(legendRect.left() + 32.0, gradientRect.bottom() + 132.0, legendRect.width() - 40.0, 18.0),
		                 Qt::AlignLeft | Qt::AlignVCenter,
		                 tr("YFP"));

		painter->setPen(QPen(QColor(255, 255, 255), 1.0));
		painter->setBrush(QColor(29, 185, 201));
		painter->drawEllipse(QPointF(legendRect.left() + 20.0, gradientRect.bottom() + 166.0), 8.0, 5.0);
		painter->setPen(QColor(30, 41, 59));
		painter->drawText(QRectF(legendRect.left() + 32.0, gradientRect.bottom() + 156.0, legendRect.width() - 40.0, 18.0),
		                 Qt::AlignLeft | Qt::AlignVCenter,
		                 tr("CFP"));
	}

private:
	ColonyVisualSnapshot _snapshot;
	std::map<unsigned int, std::vector<BacteriumTrailSample>> _trailsByBacteriumId;
	std::vector<RenderedBacteriumMarker> _renderedMarkers;
	std::optional<unsigned int> _selectedBacteriumId;
	SelectionChangedCallback _selectionChangedCallback;
};

class BacteriaColonyViewerDialog final : public QDialog {
public:
	BacteriaColonyViewerDialog(QWidget* parent, Simulator* simulator, QString initialColonyName = "")
		: QDialog(parent), _simulator(simulator), _initialColonyName(std::move(initialColonyName)) {
		setAttribute(Qt::WA_DeleteOnClose);
		setWindowTitle(tr("Bacteria Colony Viewer"));
		resize(920, 680);

		auto* rootLayout = new QVBoxLayout(this);
		auto* selectorLayout = new QHBoxLayout();

		auto* selectorLabel = new QLabel(tr("Colony:"), this);
		_colonySelector = new QComboBox(this);
		_colonySelector->setMinimumContentsLength(24);
		_colonySelector->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);

		auto* refreshButton = new QPushButton(tr("Refresh now"), this);
		auto* stepButton = new QPushButton(tr("Step colony"), this);
		_runToggleButton = new QPushButton(tr("Start run"), this);
		_liveToggleButton = new QPushButton(tr("Pause live"), this);
		auto* refreshIntervalLabel = new QLabel(tr("Refresh (ms):"), this);
		_refreshIntervalSpin = new QSpinBox(this);
		_refreshIntervalSpin->setRange(50, 5000);
		_refreshIntervalSpin->setSingleStep(50);
		_refreshIntervalSpin->setValue(150);
		selectorLayout->addWidget(selectorLabel);
		selectorLayout->addWidget(_colonySelector, 1);
		selectorLayout->addWidget(refreshIntervalLabel);
		selectorLayout->addWidget(_refreshIntervalSpin);
		selectorLayout->addWidget(stepButton);
		selectorLayout->addWidget(_runToggleButton);
		selectorLayout->addWidget(_liveToggleButton);
		selectorLayout->addWidget(refreshButton);
		rootLayout->addLayout(selectorLayout);

		_summaryLabel = new QLabel(this);
		_summaryLabel->setWordWrap(true);
		rootLayout->addWidget(_summaryLabel);

		_detailsLabel = new QLabel(this);
		_detailsLabel->setWordWrap(true);
		rootLayout->addWidget(_detailsLabel);

		_selectionLabel = new QLabel(this);
		_selectionLabel->setWordWrap(true);
		rootLayout->addWidget(_selectionLabel);

		_executionLabel = new QLabel(this);
		_executionLabel->setWordWrap(true);
		rootLayout->addWidget(_executionLabel);

		_canvas = new BacteriaColonyCanvas(this);
		_canvas->setSelectionChangedCallback([this](std::optional<unsigned int> selectedBacteriumId) {
			_selectedBacteriumId = selectedBacteriumId;
			_refreshSelectionSummary(_lastSnapshot);
		});
		rootLayout->addWidget(_canvas, 1);

		auto* buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
		rootLayout->addWidget(buttons);
		connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::close);
		connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::close);

		connect(refreshButton, &QPushButton::clicked, this, [this]() {
			_refreshColonyList();
			_refreshSnapshot();
		});
		connect(stepButton, &QPushButton::clicked, this, [this]() {
			_executeSelectedColonyStep();
			_refreshColonyList();
			_refreshSnapshot();
		});
		connect(_runToggleButton, &QPushButton::clicked, this, [this]() {
			_setRunEnabled(!_runEnabled);
		});
		connect(_liveToggleButton, &QPushButton::clicked, this, [this]() {
			_setLiveUpdatesEnabled(!_liveUpdatesEnabled);
		});
		connect(_refreshIntervalSpin, qOverload<int>(&QSpinBox::valueChanged), this, [this](int value) {
			if (_refreshTimer != nullptr) {
				_refreshTimer->setInterval(value);
			}
			if (_runTimer != nullptr) {
				_runTimer->setInterval(value);
			}
		});
		connect(_colonySelector, qOverload<int>(&QComboBox::currentIndexChanged), this, [this](int currentIndex) {
			Q_UNUSED(currentIndex)
			_refreshSnapshot();
		});

		_refreshTimer = new QTimer(this);
		_refreshTimer->setInterval(_refreshIntervalSpin->value());
		// Poll the live colony state because the generic GUI extension contract
		// does not yet expose per-event callbacks to extension-owned widgets.
		connect(_refreshTimer, &QTimer::timeout, this, [this]() {
			_refreshColonyList();
			_refreshSnapshot();
		});
		_setLiveUpdatesEnabled(true);

		_runTimer = new QTimer(this);
		_runTimer->setInterval(_refreshIntervalSpin->value());
		// Manual viewer execution mutates colony state for inspection, but it
		// does not advance ModelSimulation time; event-calendar dispatch does.
		connect(_runTimer, &QTimer::timeout, this, [this]() {
			_executeSelectedColonyStep();
			_refreshColonyList();
			_refreshSnapshot();
		});
		_setRunEnabled(false);

		_refreshColonyList();
		_refreshSnapshot();
	}

private:
	void _setRunEnabled(bool enabled) {
		_runEnabled = enabled;
		if (_runTimer != nullptr) {
			if (_runEnabled) {
				_runTimer->start();
			} else {
				_runTimer->stop();
			}
		}
		if (_runToggleButton != nullptr) {
			_runToggleButton->setText(_runEnabled ? tr("Stop run") : tr("Start run"));
		}
	}

	void _setLiveUpdatesEnabled(bool enabled) {
		_liveUpdatesEnabled = enabled;
		if (_refreshTimer != nullptr) {
			if (_liveUpdatesEnabled) {
				_refreshTimer->start();
			} else {
				_refreshTimer->stop();
			}
		}
		if (_liveToggleButton != nullptr) {
			_liveToggleButton->setText(_liveUpdatesEnabled ? tr("Pause live") : tr("Resume live"));
		}
	}

	void _executeSelectedColonyStep() {
		BacteriaColony* colony = _selectedColony();
		if (colony == nullptr) {
			_lastExecutionMessage = tr("Execution: no selected BacteriaColony.");
			_setRunEnabled(false);
			return;
		}

		GroProgramRuntime::ExecutionResult result = colony->executeGroProgram();
		if (!result.succeeded) {
			_lastExecutionMessage = tr("Execution failed: %1")
				.arg(QString::fromStdString(result.errorMessage));
			_setRunEnabled(false);
			return;
		}

		_lastExecutionMessage =
			tr("Execution: manual step, model time unchanged; %1 command(s), %2 signal mutation(s), %3 population mutation(s)")
				.arg(result.executedCommands)
				.arg(static_cast<qulonglong>(result.signalMutations.size()))
				.arg(static_cast<qulonglong>(result.populationMutations.size()));
	}

	Model* _currentModel() const {
		if (_simulator == nullptr || _simulator->getModelManager() == nullptr) {
			return nullptr;
		}
		return _simulator->getModelManager()->current();
	}

	std::vector<BacteriaColony*> _collectColonies() const {
		std::vector<BacteriaColony*> colonies;
		Model* model = _currentModel();
		if (model == nullptr || model->getComponentManager() == nullptr) {
			return colonies;
		}
		std::list<ModelComponent*>* allComponents = model->getComponentManager()->getAllComponents();
		if (allComponents == nullptr) {
			return colonies;
		}
		for (ModelComponent* component : *allComponents) {
			if (auto* colony = dynamic_cast<BacteriaColony*>(component)) {
				colonies.push_back(colony);
			}
		}
		std::sort(colonies.begin(), colonies.end(), [](const BacteriaColony* left, const BacteriaColony* right) {
			return left != nullptr && right != nullptr ? left->getName() < right->getName() : left != nullptr;
		});
		return colonies;
	}

	void _refreshColonyList() {
		QString previouslySelected = _colonySelector->currentData().toString();
		if (! _initialColonyName.isEmpty()) {
			previouslySelected = _initialColonyName;
			_initialColonyName.clear();
		}
		const std::vector<BacteriaColony*> colonies = _collectColonies();
		_colonySelector->blockSignals(true);
		_colonySelector->clear();
		for (BacteriaColony* colony : colonies) {
			if (colony != nullptr) {
				_colonySelector->addItem(QString::fromStdString(colony->getName()),
				                         QString::fromStdString(colony->getName()));
			}
		}
		const int selectedIndex = previouslySelected.isEmpty()
			? (_colonySelector->count() > 0 ? 0 : -1)
			: _colonySelector->findData(previouslySelected);
		if (selectedIndex >= 0) {
			_colonySelector->setCurrentIndex(selectedIndex);
		}
		_colonySelector->blockSignals(false);
	}

	BacteriaColony* _selectedColony() const {
		const QString selectedName = _colonySelector->currentData().toString();
		if (selectedName.isEmpty()) {
			return nullptr;
		}
		for (BacteriaColony* colony : _collectColonies()) {
			if (colony != nullptr && QString::fromStdString(colony->getName()) == selectedName) {
				return colony;
			}
		}
		return nullptr;
	}

	void _refreshSnapshot() {
		ColonyVisualSnapshot snapshot;
		BacteriaColony* colony = _selectedColony();
		if (colony == nullptr) {
			snapshot.statusMessage = _currentModel() == nullptr
			                         ? tr("No opened model.")
			                         : tr("Current model has no BacteriaColony component.");
			_summaryLabel->setText(snapshot.statusMessage);
			_detailsLabel->clear();
			_executionLabel->setText(_lastExecutionMessage);
			_lastSnapshot = snapshot;
			_selectedBacteriumId.reset();
			_selectionLabel->setText(tr("Selection: none"));
			_trailsByBacteriumId.clear();
			_canvas->setSelectedBacteriumId(std::nullopt);
			_canvas->setSnapshot(std::move(snapshot), {});
			return;
		}

		snapshot.valid = true;
		snapshot.colonyName = QString::fromStdString(colony->getName());
		snapshot.colonyTime = colony->getColonyTime();
		snapshot.populationSize = colony->getPopulationSize();
		snapshot.internalBacteriaCount = static_cast<unsigned int>(colony->getInternalBacteriaCount());
		snapshot.gridWidth = std::max(1u, colony->getGridWidth());
		snapshot.gridHeight = std::max(1u, colony->getGridHeight());
		snapshot.bioNetworkName = colony->getBioNetwork() != nullptr
		                          ? QString::fromStdString(colony->getBioNetwork()->getName())
		                          : tr("(none)");
		snapshot.signalGridName = colony->getSignalGrid() != nullptr
		                         ? QString::fromStdString(colony->getSignalGrid()->getName())
		                         : tr("(none)");
		snapshot.signalValues.reserve(static_cast<std::size_t>(snapshot.gridWidth) * snapshot.gridHeight);
		double minSignal = std::numeric_limits<double>::max();
		double maxSignal = std::numeric_limits<double>::lowest();
		for (unsigned int y = 0; y < snapshot.gridHeight; ++y) {
			for (unsigned int x = 0; x < snapshot.gridWidth; ++x) {
				const double signalValue = colony->getSignalValueAt(x, y);
				snapshot.signalValues.push_back(signalValue);
				minSignal = std::min(minSignal, signalValue);
				maxSignal = std::max(maxSignal, signalValue);
			}
		}
		if (snapshot.signalValues.empty()) {
			minSignal = 0.0;
			maxSignal = 0.0;
		}
		snapshot.minSignal = minSignal;
		snapshot.maxSignal = maxSignal;
		std::map<std::pair<unsigned int, unsigned int>, unsigned int> bacteriaPerCell;
		snapshot.bacteria.reserve(colony->getInternalBacteriaCount());
		for (std::size_t index = 0; index < colony->getInternalBacteriaCount(); ++index) {
			const BacteriaColony::BacteriumState& bacterium = colony->getBacteriumState(index);
			BacteriumVisualState visual;
			visual.id = bacterium.id;
			visual.generation = bacterium.generation;
			visual.gridX = bacterium.gridX;
			visual.gridY = bacterium.gridY;
			visual.positionX = bacterium.positionX;
			visual.positionY = bacterium.positionY;
			visual.directionRadians = bacterium.directionRadians;
			visual.volume = bacterium.volume;
			visual.size = bacterium.size;
			visual.gfp = bacterium.gfp;
			visual.rfp = bacterium.rfp;
			visual.yfp = bacterium.yfp;
			visual.cfp = bacterium.cfp;
			visual.age = colony->getBacteriumAge(index);
			visual.programName = bacterium.programName.empty()
			                     ? tr("(none)")
			                     : QString::fromStdString(bacterium.programName);
			QStringList variableParts;
			for (const auto& variable : bacterium.runtimeVariables) {
				variableParts << QString("%1=%2")
					.arg(QString::fromStdString(variable.first))
					.arg(QString::number(variable.second, 'g', 5));
				if (variableParts.size() >= 6) {
					break;
				}
			}
			if (static_cast<int>(bacterium.runtimeVariables.size()) > variableParts.size()) {
				variableParts << tr("...");
			}
			visual.runtimeVariablesSummary = variableParts.isEmpty() ? tr("(none)") : variableParts.join(tr(", "));
			snapshot.bacteria.push_back(visual);
			const auto key = std::make_pair(visual.gridX, visual.gridY);
			snapshot.maxBacteriaPerCell = std::max(snapshot.maxBacteriaPerCell, ++bacteriaPerCell[key]);
		}
		_updateTrails(snapshot);

		if (_selectedBacteriumFromSnapshot(snapshot) == nullptr) {
			_selectedBacteriumId.reset();
		}

		_summaryLabel->setText(
			tr("Colony <b>%1</b> | time=%2 | population=%3 | bacteria=%4 | grid=%5x%6 | BioNetwork=%7 | SignalGrid=%8")
			.arg(snapshot.colonyName)
			.arg(QString::number(snapshot.colonyTime, 'g', 8))
			.arg(snapshot.populationSize)
			.arg(snapshot.internalBacteriaCount)
			.arg(snapshot.gridWidth)
			.arg(snapshot.gridHeight)
			.arg(snapshot.bioNetworkName)
			.arg(snapshot.signalGridName));
		_detailsLabel->setText(
			tr("Signal range: [%1, %2] | Peak occupancy per cell: %3 | Trail depth: %4 refreshes | Live=%5 | Manual run=%6")
				.arg(QString::number(snapshot.minSignal, 'g', 4))
				.arg(QString::number(snapshot.maxSignal, 'g', 4))
				.arg(snapshot.maxBacteriaPerCell)
				.arg(kTrailHistoryLimit)
				.arg(_liveUpdatesEnabled ? tr("on") : tr("paused"))
				.arg(_runEnabled ? tr("on") : tr("off")));
		_executionLabel->setText(_lastExecutionMessage);
		_lastSnapshot = snapshot;
		_refreshSelectionSummary(_lastSnapshot);
		_canvas->setSelectedBacteriumId(_selectedBacteriumId);
		_canvas->setSnapshot(std::move(snapshot), _trailsByBacteriumId);
	}

	const BacteriumVisualState* _selectedBacteriumFromSnapshot(const ColonyVisualSnapshot& snapshot) const {
		if (!_selectedBacteriumId.has_value()) {
			return nullptr;
		}
		for (const BacteriumVisualState& bacterium : snapshot.bacteria) {
			if (bacterium.id == _selectedBacteriumId.value()) {
				return &bacterium;
			}
		}
		return nullptr;
	}

	void _refreshSelectionSummary(const ColonyVisualSnapshot& snapshot) {
		const BacteriumVisualState* selectedBacterium = _selectedBacteriumFromSnapshot(snapshot);
		if (selectedBacterium == nullptr) {
			_selectionLabel->setText(tr("Selection: click a bacterium in the viewer to inspect it."));
			return;
		}

		std::size_t trailDepth = 0;
		auto trailIt = _trailsByBacteriumId.find(selectedBacterium->id);
		if (trailIt != _trailsByBacteriumId.end()) {
			trailDepth = trailIt->second.size();
		}
		_selectionLabel->setText(
			tr("Selection: bacterium #%1 | program=%2 | gen=%3 | cell=(%4,%5) | pos=(%6,%7) | age=%8 | size=%9 | volume=%10 | dir=%11 | gfp=%12 | rfp=%13 | yfp=%14 | cfp=%15 | trail samples=%16 | vars: %17")
				.arg(selectedBacterium->id)
				.arg(selectedBacterium->programName)
				.arg(selectedBacterium->generation)
				.arg(selectedBacterium->gridX)
				.arg(selectedBacterium->gridY)
				.arg(QString::number(selectedBacterium->positionX, 'g', 5))
				.arg(QString::number(selectedBacterium->positionY, 'g', 5))
				.arg(QString::number(selectedBacterium->age, 'g', 5))
				.arg(QString::number(selectedBacterium->size, 'g', 5))
				.arg(QString::number(selectedBacterium->volume, 'g', 5))
				.arg(QString::number(selectedBacterium->directionRadians, 'g', 5))
				.arg(QString::number(selectedBacterium->gfp, 'g', 5))
				.arg(QString::number(selectedBacterium->rfp, 'g', 5))
				.arg(QString::number(selectedBacterium->yfp, 'g', 5))
				.arg(QString::number(selectedBacterium->cfp, 'g', 5))
				.arg(static_cast<qulonglong>(trailDepth))
				.arg(selectedBacterium->runtimeVariablesSummary));
	}

	void _updateTrails(const ColonyVisualSnapshot& snapshot) {
		if (!_lastColonyName.isEmpty() && _lastColonyName != snapshot.colonyName) {
			_trailsByBacteriumId.clear();
		}
		if (snapshot.colonyTime + 1e-12 < _lastColonyTime
		    || snapshot.gridWidth != _lastGridWidth
		    || snapshot.gridHeight != _lastGridHeight) {
			// Reset trails on colony-time rewind or grid reconfiguration.
			_trailsByBacteriumId.clear();
		}

		std::map<unsigned int, bool> aliveIds;
		for (const BacteriumVisualState& bacterium : snapshot.bacteria) {
			aliveIds[bacterium.id] = true;
			std::vector<BacteriumTrailSample>& trail = _trailsByBacteriumId[bacterium.id];
			if (trail.empty()
			    || std::abs(trail.back().positionX - bacterium.positionX) > 1e-6
			    || std::abs(trail.back().positionY - bacterium.positionY) > 1e-6
			    || std::abs(trail.back().colonyTime - snapshot.colonyTime) > 1e-12) {
				trail.push_back({bacterium.positionX, bacterium.positionY, snapshot.colonyTime});
			}
			if (trail.size() > kTrailHistoryLimit) {
				trail.erase(trail.begin(), trail.begin() + static_cast<long>(trail.size() - kTrailHistoryLimit));
			}
		}

		for (auto it = _trailsByBacteriumId.begin(); it != _trailsByBacteriumId.end();) {
			if (aliveIds.find(it->first) == aliveIds.end()) {
				it = _trailsByBacteriumId.erase(it);
			} else {
				++it;
			}
		}

		_lastColonyName = snapshot.colonyName;
		_lastColonyTime = snapshot.colonyTime;
		_lastGridWidth = snapshot.gridWidth;
		_lastGridHeight = snapshot.gridHeight;
	}

private:
	Simulator* _simulator = nullptr;
	QComboBox* _colonySelector = nullptr;
	QPushButton* _runToggleButton = nullptr;
	QPushButton* _liveToggleButton = nullptr;
	QSpinBox* _refreshIntervalSpin = nullptr;
	QLabel* _summaryLabel = nullptr;
	QLabel* _detailsLabel = nullptr;
	QLabel* _selectionLabel = nullptr;
	QLabel* _executionLabel = nullptr;
	BacteriaColonyCanvas* _canvas = nullptr;
	QTimer* _refreshTimer = nullptr;
	QTimer* _runTimer = nullptr;
	bool _runEnabled = false;
	bool _liveUpdatesEnabled = true;
	QString _lastExecutionMessage = tr("Execution: idle");
	std::optional<unsigned int> _selectedBacteriumId;
	ColonyVisualSnapshot _lastSnapshot;
	QString _lastColonyName;
	QString _initialColonyName;
	double _lastColonyTime = 0.0;
	unsigned int _lastGridWidth = 0;
	unsigned int _lastGridHeight = 0;
	std::map<unsigned int, std::vector<BacteriumTrailSample>> _trailsByBacteriumId;
	static constexpr std::size_t kTrailHistoryLimit = 12;
};

} // namespace

class BacteriaColonyViewerGuiExtensionPlugin final : public GuiExtensionPlugin {
public:
	std::string extensionId() const override {
		return "gui.extensions.gro.bacteria.colony.viewer";
	}

	std::string displayName() const override {
		return "Bacteria Colony Viewer";
	}

	std::string version() const override {
		return "1.0.0";
	}

	std::vector<std::string> requiredModelPlugins() const override {
		return {"bacteriacolony"};
	}

	void registerContributions(GuiExtensionRegistry* registry) const override {
		if (registry == nullptr) {
			return;
		}

		GuiActionContribution selectedViewerAction;
		selectedViewerAction.id = "actionGuiExtensionsBacteriaColonyViewerSelected";
		selectedViewerAction.menuPath = "Tools/Extensions/Biological";
		selectedViewerAction.text = "Visualize Selected Bacteria Colony...";
		selectedViewerAction.statusTip = "Open the bacteria colony viewer focused on the selected canvas component";
		selectedViewerAction.isVisible = [](const GuiExtensionRuntimeContext& context) {
			return context.mainWindow != nullptr && context.simulator != nullptr;
		};
		selectedViewerAction.isEnabled = [](const GuiExtensionRuntimeContext& context) {
			return selectedBacteriaColonyFromCanvas(context) != nullptr;
		};
		selectedViewerAction.trigger = [](const GuiExtensionRuntimeContext& context) {
			BacteriaColony* selectedColony = selectedBacteriaColonyFromCanvas(context);
			if (context.mainWindow == nullptr || context.simulator == nullptr || selectedColony == nullptr) {
				return;
			}

			auto* dialog = new BacteriaColonyViewerDialog(
				static_cast<QWidget*>(context.mainWindow),
				context.simulator,
				QString::fromStdString(selectedColony->getName()));
			dialog->show();
			dialog->raise();
			dialog->activateWindow();
		};
		registry->addAction(std::move(selectedViewerAction));

		GuiWindowContribution viewerWindow;
		viewerWindow.id = "actionGuiExtensionsBacteriaColonyViewer";
		viewerWindow.menuPath = "Tools/Extensions/Biological";
		viewerWindow.text = "Visualize Bacteria Colony...";
		viewerWindow.isVisible = [](const GuiExtensionRuntimeContext& context) {
			return context.mainWindow != nullptr && context.simulator != nullptr;
		};
		viewerWindow.open = [](const GuiExtensionRuntimeContext& context) {
			if (context.mainWindow == nullptr || context.simulator == nullptr) {
				return;
			}

			Model* model = context.simulator->getModelManager() != nullptr
			               ? context.simulator->getModelManager()->current()
			               : nullptr;
			if (model == nullptr) {
				QMessageBox::warning(static_cast<QWidget*>(context.mainWindow),
				                     QObject::tr("Bacteria Colony Viewer"),
				                     QObject::tr("No opened model."));
				return;
			}

			auto* dialog = new BacteriaColonyViewerDialog(static_cast<QWidget*>(context.mainWindow), context.simulator);
			dialog->show();
			dialog->raise();
			dialog->activateWindow();
		};
		registry->addWindow(std::move(viewerWindow));
	}
};

REGISTER_GUI_EXTENSION_PLUGIN(BacteriaColonyViewerGuiExtensionPlugin);
