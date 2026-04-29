#include "GuiExtensionPluginCatalog.h"

#include "kernel/simulator/ComponentManager.h"
#include "kernel/simulator/Model.h"
#include "kernel/simulator/ModelManager.h"
#include "kernel/simulator/Simulator.h"
#include "plugins/components/BiologicalModeling/BacteriaColony.h"
#include "plugins/data/BiologicalModeling/BacteriaSignalGrid.h"

#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMainWindow>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QPen>
#include <QPushButton>
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
	double age = 0.0;
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
	unsigned int gridX = 0;
	unsigned int gridY = 0;
	double colonyTime = 0.0;
};

struct RenderedBacteriumMarker {
	unsigned int id = 0;
	QPointF center;
	double radius = 0.0;
};

QColor signalColorForValue(double value, double minValue, double maxValue) {
	if (!std::isfinite(value)) {
		return QColor(96, 96, 96);
	}
	if (maxValue - minValue < 1e-12) {
		return QColor(231, 244, 236);
	}
	const double normalized = std::clamp((value - minValue) / (maxValue - minValue), 0.0, 1.0);
	const int red = static_cast<int>(28.0 + normalized * 201.0);
	const int green = static_cast<int>(94.0 + (1.0 - normalized) * 150.0);
	const int blue = static_cast<int>(123.0 + (1.0 - normalized) * 90.0);
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
		painter.fillRect(rect(), QColor(250, 252, 249));

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
		const double cellWidth = drawingRect.width() / static_cast<double>(gridWidth);
		const double cellHeight = drawingRect.height() / static_cast<double>(gridHeight);
		const auto cellRectFor = [&drawingRect, cellWidth, cellHeight](unsigned int x, unsigned int y) {
			return QRectF(drawingRect.left() + static_cast<double>(x) * cellWidth,
			              drawingRect.top() + static_cast<double>(y) * cellHeight,
			              cellWidth,
			              cellHeight);
		};
		const auto cellCenterFor = [&cellRectFor](unsigned int x, unsigned int y) {
			return cellRectFor(x, y).center();
		};

		QPen gridPen(QColor(113, 125, 126));
		gridPen.setWidthF(0.8);
		painter.setPen(gridPen);

		for (unsigned int y = 0; y < gridHeight; ++y) {
			for (unsigned int x = 0; x < gridWidth; ++x) {
				const QRectF cellRect = cellRectFor(x, y);
				const std::size_t index = static_cast<std::size_t>(y) * static_cast<std::size_t>(gridWidth) + x;
				const double value = index < _snapshot.signalValues.size() ? _snapshot.signalValues[index] : 0.0;
				painter.fillRect(cellRect, signalColorForValue(value, _snapshot.minSignal, _snapshot.maxSignal));
				painter.drawRect(cellRect);

				if (gridWidth * gridHeight <= 144) {
					painter.setPen(QColor(31, 41, 55));
					painter.drawText(cellRect.adjusted(4, 4, -4, -4),
					                 Qt::AlignTop | Qt::AlignLeft,
					                 QString::number(value, 'g', 3));
					painter.setPen(gridPen);
				}
			}
		}

		std::map<std::pair<unsigned int, unsigned int>, std::vector<const BacteriumVisualState*>> bacteriaByCell;
		for (const BacteriumVisualState& bacterium : _snapshot.bacteria) {
			bacteriaByCell[{bacterium.gridX, bacterium.gridY}].push_back(&bacterium);
		}

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
				painter.drawLine(cellCenterFor(previous.gridX, previous.gridY),
				                 cellCenterFor(current.gridX, current.gridY));
			}
		}

		// Spread multiple bacteria inside one cell so crowded colonies remain readable.
		for (const auto& entry : bacteriaByCell) {
			const unsigned int x = entry.first.first;
			const unsigned int y = entry.first.second;
			if (x >= gridWidth || y >= gridHeight) {
				continue;
			}

			const QRectF cellRect = cellRectFor(x, y);
			const std::vector<const BacteriumVisualState*>& cellBacteria = entry.second;
			const int columns = std::max(1, static_cast<int>(std::ceil(std::sqrt(static_cast<double>(cellBacteria.size())))));
			const int rows = std::max(1, static_cast<int>(std::ceil(static_cast<double>(cellBacteria.size()) / columns)));
			const double dotDiameter = std::max(5.0, std::min(cellWidth / (columns + 1.0), cellHeight / (rows + 1.0)));

			for (std::size_t i = 0; i < cellBacteria.size(); ++i) {
				const int row = static_cast<int>(i) / columns;
				const int col = static_cast<int>(i) % columns;
				const double centerX = cellRect.left() + ((col + 1.0) / (columns + 1.0)) * cellRect.width();
				const double centerY = cellRect.top() + ((row + 1.0) / (rows + 1.0)) * cellRect.height();
				const QColor fillColor = (cellBacteria[i]->generation % 2 == 0) ? QColor(32, 54, 93) : QColor(14, 116, 144);
				const bool selected = _selectedBacteriumId.has_value() && _selectedBacteriumId.value() == cellBacteria[i]->id;
				painter.setPen(QPen(selected ? QColor(245, 158, 11) : QColor(255, 255, 255), selected ? 2.2 : 0.9));
				painter.setBrush(fillColor);
				const double radius = dotDiameter * 0.5;
				painter.drawEllipse(QPointF(centerX, centerY), radius, radius);
				if (selected) {
					painter.setPen(QPen(QColor(120, 53, 15, 210), 1.2));
					painter.setBrush(Qt::NoBrush);
					painter.drawEllipse(QPointF(centerX, centerY), radius + 3.0, radius + 3.0);
				}
				_renderedMarkers.push_back({cellBacteria[i]->id, QPointF(centerX, centerY), radius});
			}
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
		painter->setBrush(QColor(32, 54, 93));
		painter->drawEllipse(QPointF(legendRect.left() + 20.0, gradientRect.bottom() + 48.0), 6.0, 6.0);
		painter->setPen(QColor(30, 41, 59));
		painter->drawText(QRectF(legendRect.left() + 32.0, gradientRect.bottom() + 38.0, legendRect.width() - 40.0, 18.0),
		                 Qt::AlignLeft | Qt::AlignVCenter,
		                 tr("even gen"));

		painter->setPen(QPen(QColor(255, 255, 255), 1.0));
		painter->setBrush(QColor(14, 116, 144));
		painter->drawEllipse(QPointF(legendRect.left() + 20.0, gradientRect.bottom() + 72.0), 6.0, 6.0);
		painter->setPen(QColor(30, 41, 59));
		painter->drawText(QRectF(legendRect.left() + 32.0, gradientRect.bottom() + 62.0, legendRect.width() - 40.0, 18.0),
		                 Qt::AlignLeft | Qt::AlignVCenter,
		                 tr("odd gen"));

		painter->setPen(QPen(QColor(15, 23, 42, 170), 1.6));
		painter->drawLine(QPointF(legendRect.left() + 12.0, gradientRect.bottom() + 94.0),
		                 QPointF(legendRect.left() + 28.0, gradientRect.bottom() + 94.0));
		painter->setPen(QColor(30, 41, 59));
		painter->drawText(QRectF(legendRect.left() + 32.0, gradientRect.bottom() + 84.0, legendRect.width() - 40.0, 18.0),
		                 Qt::AlignLeft | Qt::AlignVCenter,
		                 tr("trail"));
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
	BacteriaColonyViewerDialog(QWidget* parent, Simulator* simulator) : QDialog(parent), _simulator(simulator) {
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
		connect(_liveToggleButton, &QPushButton::clicked, this, [this]() {
			_setLiveUpdatesEnabled(!_liveUpdatesEnabled);
		});
		connect(_refreshIntervalSpin, qOverload<int>(&QSpinBox::valueChanged), this, [this](int value) {
			if (_refreshTimer != nullptr) {
				_refreshTimer->setInterval(value);
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

		_refreshColonyList();
		_refreshSnapshot();
	}

private:
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
		const QString previouslySelected = _colonySelector->currentData().toString();
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
			visual.age = colony->getBacteriumAge(index);
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
			tr("Signal range: [%1, %2] | Peak occupancy per cell: %3 | Trail depth: %4 refreshes | Live=%5")
				.arg(QString::number(snapshot.minSignal, 'g', 4))
				.arg(QString::number(snapshot.maxSignal, 'g', 4))
				.arg(snapshot.maxBacteriaPerCell)
				.arg(kTrailHistoryLimit)
				.arg(_liveUpdatesEnabled ? tr("on") : tr("paused")));
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
			tr("Selection: bacterium #%1 | gen=%2 | cell=(%3,%4) | age=%5 | trail samples=%6")
				.arg(selectedBacterium->id)
				.arg(selectedBacterium->generation)
				.arg(selectedBacterium->gridX)
				.arg(selectedBacterium->gridY)
				.arg(QString::number(selectedBacterium->age, 'g', 5))
				.arg(static_cast<qulonglong>(trailDepth)));
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
			    || trail.back().gridX != bacterium.gridX
			    || trail.back().gridY != bacterium.gridY
			    || std::abs(trail.back().colonyTime - snapshot.colonyTime) > 1e-12) {
				trail.push_back({bacterium.gridX, bacterium.gridY, snapshot.colonyTime});
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
	QPushButton* _liveToggleButton = nullptr;
	QSpinBox* _refreshIntervalSpin = nullptr;
	QLabel* _summaryLabel = nullptr;
	QLabel* _detailsLabel = nullptr;
	QLabel* _selectionLabel = nullptr;
	BacteriaColonyCanvas* _canvas = nullptr;
	QTimer* _refreshTimer = nullptr;
	bool _liveUpdatesEnabled = true;
	std::optional<unsigned int> _selectedBacteriumId;
	ColonyVisualSnapshot _lastSnapshot;
	QString _lastColonyName;
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
