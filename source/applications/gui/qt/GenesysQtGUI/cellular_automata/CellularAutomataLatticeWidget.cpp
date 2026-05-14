#include "CellularAutomataLatticeWidget.h"

#include "CellularAutomataStateColorMap.h"
#include "CellularAutomataViewerController.h"
#include "plugins/components/ModalModel/CellularAutomata/State.h"

#include <algorithm>
#include <cmath>

#include <QContextMenuEvent>
#include <QInputDialog>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>

namespace {
constexpr qreal kMargin = 16.0;
}

CellularAutomataLatticeWidget::CellularAutomataLatticeWidget(QWidget* parent)
	: QWidget(parent) {
	setMouseTracking(true);
	setAutoFillBackground(true);
}

void CellularAutomataLatticeWidget::setController(CellularAutomataViewerController* controller) {
	if (_controller == controller) {
		return;
	}
	if (_controller != nullptr) {
		disconnect(_controller, nullptr, this, nullptr);
	}
	_controller = controller;
	if (_controller != nullptr) {
		connect(_controller, &CellularAutomataViewerController::modelChanged, this, QOverload<>::of(&CellularAutomataLatticeWidget::update));
		connect(_controller, &CellularAutomataViewerController::timeChanged, this, QOverload<>::of(&CellularAutomataLatticeWidget::update));
		connect(_controller, &CellularAutomataViewerController::paintStateChanged, this, QOverload<>::of(&CellularAutomataLatticeWidget::update));
	}
	update();
}

void CellularAutomataLatticeWidget::paintEvent(QPaintEvent* event) {
	Q_UNUSED(event);

	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing, false);
	painter.fillRect(rect(), CellularAutomataStateColorMap::backgroundColor());

	if (_controller == nullptr || !_controller->hasDemo()) {
		painter.setPen(CellularAutomataStateColorMap::emptyTextColor());
		painter.drawText(rect(), Qt::AlignCenter, tr("No cellular automaton loaded."));
		return;
	}

	const QSize latticeSize = _controller->latticeSize();
	const int columns = latticeSize.width();
	const int rows = latticeSize.height();
	if (columns <= 0 || rows <= 0) {
		painter.setPen(CellularAutomataStateColorMap::emptyTextColor());
		painter.drawText(rect(), Qt::AlignCenter, tr("Invalid lattice size."));
		return;
	}

	const qreal usableWidth = std::max<qreal>(1.0, width() - 2.0 * kMargin);
	const qreal usableHeight = std::max<qreal>(1.0, height() - 2.0 * kMargin);
	const qreal cellWidth = std::floor(std::min(usableWidth / columns, usableHeight / rows));
	if (cellWidth < 1.0) {
		painter.setPen(CellularAutomataStateColorMap::emptyTextColor());
		painter.drawText(rect(), Qt::AlignCenter, tr("Window too small to render the lattice."));
		return;
	}
	const qreal cellHeight = cellWidth;
	const qreal gridWidth = cellWidth * columns;
	const qreal gridHeight = cellHeight * rows;
	const qreal originX = (width() - gridWidth) * 0.5;
	const qreal originY = (height() - gridHeight) * 0.5;

	QPen gridPen(CellularAutomataStateColorMap::gridLineColor());
	gridPen.setWidthF(0.75);
	painter.setPen(Qt::NoPen);

	for (int y = 0; y < rows; ++y) {
		for (int x = 0; x < columns; ++x) {
			const long state = _controller->cellState(x, y);
			const QString stateText = _controller->model()->stateTextAt(x, y);
			const QRectF cellRect(originX + x * cellWidth, originY + y * cellHeight, cellWidth, cellHeight);
			painter.fillRect(cellRect, CellularAutomataStateColorMap::colorForStateText(stateText, state));
		}
	}

	painter.setPen(gridPen);
	for (int x = 0; x <= columns; ++x) {
		const qreal xPos = originX + x * cellWidth;
		painter.drawLine(QPointF(xPos, originY), QPointF(xPos, originY + gridHeight));
	}
	for (int y = 0; y <= rows; ++y) {
		const qreal yPos = originY + y * cellHeight;
		painter.drawLine(QPointF(originX, yPos), QPointF(originX + gridWidth, yPos));
	}
}

void CellularAutomataLatticeWidget::mousePressEvent(QMouseEvent* event) {
	if (_controller == nullptr || !_controller->hasDemo()) {
		return QWidget::mousePressEvent(event);
	}

	if (event->button() == Qt::LeftButton) {
		if (!_ensureSelectedPaintState()) {
			return;
		}
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
		const QPoint pointerPosition = event->position().toPoint();
#else
		const QPoint pointerPosition = event->pos();
#endif
		const QPoint cell = _cellAtPosition(pointerPosition);
		if (cell.x() >= 0) {
			_dragging = true;
			_lastPaintedCell = QPoint{-1, -1};
			_paintCellState(cell);
		}
		event->accept();
		return;
	}

	if (event->button() == Qt::RightButton) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
		const QPoint pointerPosition = event->position().toPoint();
		const QPoint globalPosition = event->globalPosition().toPoint();
#else
		const QPoint pointerPosition = event->pos();
		const QPoint globalPosition = event->globalPos();
#endif
		const QPoint cell = _cellAtPosition(pointerPosition);
		if (cell.x() >= 0) {
			_showStateContextMenu(globalPosition, cell);
			event->accept();
			return;
		}
	}

	QWidget::mousePressEvent(event);
}

void CellularAutomataLatticeWidget::mouseMoveEvent(QMouseEvent* event) {
	if (_controller == nullptr || !_controller->hasDemo()) {
		return QWidget::mouseMoveEvent(event);
	}

	if (_dragging && (event->buttons() & Qt::LeftButton)) {
		if (!_ensureSelectedPaintState()) {
			return;
		}
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
		const QPoint pointerPosition = event->position().toPoint();
#else
		const QPoint pointerPosition = event->pos();
#endif
		const QPoint cell = _cellAtPosition(pointerPosition);
		if (cell.x() >= 0 && cell != _lastPaintedCell) {
			_paintCellState(cell);
		}
		event->accept();
		return;
	}

	QWidget::mouseMoveEvent(event);
}

void CellularAutomataLatticeWidget::mouseReleaseEvent(QMouseEvent* event) {
	if (event->button() == Qt::LeftButton) {
		_dragging = false;
	}
	QWidget::mouseReleaseEvent(event);
}

void CellularAutomataLatticeWidget::contextMenuEvent(QContextMenuEvent* event) {
	if (_controller == nullptr || !_controller->hasDemo()) {
		return QWidget::contextMenuEvent(event);
	}

	const QPoint cell = _cellAtPosition(event->pos());
	if (cell.x() >= 0) {
		_showStateContextMenu(event->globalPos(), cell);
		event->accept();
		return;
	}
	QWidget::contextMenuEvent(event);
}

QSize CellularAutomataLatticeWidget::minimumSizeHint() const {
	return QSize{480, 360};
}

bool CellularAutomataLatticeWidget::_ensureSelectedPaintState() {
	if (_controller == nullptr) {
		return false;
	}
	if (_controller->hasSelectedPaintState()) {
		return true;
	}
	bool ok = false;
	const std::vector<long> paintStates = _controller->availablePaintStates();
	const int defaultValue = paintStates.empty() ? 1 : static_cast<int>(paintStates.front());
	const int value = QInputDialog::getInt(this,
	                                       tr("Cell State"),
	                                       tr("State to paint with:"),
	                                       defaultValue,
	                                       -1024,
	                                       1024,
	                                       1,
	                                       &ok);
	if (!ok) {
		return false;
	}
	_controller->setSelectedPaintState(value);
	return true;
}

QPoint CellularAutomataLatticeWidget::_cellAtPosition(const QPoint& widgetPosition) const {
	if (_controller == nullptr || !_controller->hasDemo()) {
		return QPoint{-1, -1};
	}
	const QSize latticeSize = _controller->latticeSize();
	if (latticeSize.width() <= 0 || latticeSize.height() <= 0) {
		return QPoint{-1, -1};
	}

	const QRectF gridRect = _gridRect();
	if (!gridRect.contains(widgetPosition)) {
		return QPoint{-1, -1};
	}
	const qreal cellWidth = gridRect.width() / latticeSize.width();
	const qreal cellHeight = gridRect.height() / latticeSize.height();
	const int x = static_cast<int>((widgetPosition.x() - gridRect.left()) / cellWidth);
	const int y = static_cast<int>((widgetPosition.y() - gridRect.top()) / cellHeight);
	if (!_controller->model()->isValidPosition(x, y)) {
		return QPoint{-1, -1};
	}
	return QPoint{x, y};
}

QRectF CellularAutomataLatticeWidget::_gridRect() const {
	if (_controller == nullptr || !_controller->hasDemo()) {
		return QRectF();
	}
	const QSize latticeSize = _controller->latticeSize();
	const int columns = latticeSize.width();
	const int rows = latticeSize.height();
	if (columns <= 0 || rows <= 0) {
		return QRectF();
	}
	const qreal usableWidth = std::max<qreal>(1.0, width() - 2.0 * kMargin);
	const qreal usableHeight = std::max<qreal>(1.0, height() - 2.0 * kMargin);
	const qreal cellWidth = std::floor(std::min(usableWidth / columns, usableHeight / rows));
	if (cellWidth < 1.0) {
		return QRectF();
	}
	const qreal cellHeight = cellWidth;
	const qreal gridWidth = cellWidth * columns;
	const qreal gridHeight = cellHeight * rows;
	const qreal originX = (width() - gridWidth) * 0.5;
	const qreal originY = (height() - gridHeight) * 0.5;
	return QRectF(originX, originY, gridWidth, gridHeight);
}

void CellularAutomataLatticeWidget::_paintCellState(const QPoint& cellPosition) {
	if (_controller == nullptr || !_controller->hasSelectedPaintState()) {
		return;
	}
	const long state = _controller->selectedPaintState().value_or(0);
	if (_controller->setCellState(cellPosition.x(), cellPosition.y(), state)) {
		_lastPaintedCell = cellPosition;
	}
}

void CellularAutomataLatticeWidget::_showStateContextMenu(const QPoint& globalPosition, const QPoint& cellPosition) {
	if (_controller == nullptr) {
		return;
	}

	if (!_controller->hasSelectedPaintState()) {
		_ensureSelectedPaintState();
	}

	QMenu menu(this);
	const std::vector<long> paintStates = _controller->availablePaintStates();
	const int defaultValue = paintStates.empty() ? 1 : static_cast<int>(paintStates.front());
	std::vector<QAction*> stateActions;
	stateActions.reserve(paintStates.size());
	for (long state : paintStates) {
		HppLatticeGasState hppState;
		hppState.setValue(state);
		const QString label = _controller->settings().statePreset == CellularAutomataStatePreset::HppLatticeGas
			                      ? QString::fromStdString(hppState.toString())
			                      : QString::number(state);
		stateActions.push_back(menu.addAction(tr("Paint with %1").arg(label)));
	}
	if (!stateActions.empty()) {
		menu.addSeparator();
	}
	QAction* customState = menu.addAction(tr("Custom state..."));
	menu.addSeparator();
	QAction* useCurrent = nullptr;
	if (_controller->hasSelectedPaintState()) {
		useCurrent = menu.addAction(tr("Use current state (%1)").arg(_controller->selectedPaintStateText()));
	}

	QAction* selected = menu.exec(globalPosition);
	if (selected == nullptr) {
		return;
	}
	for (size_t index = 0; index < stateActions.size(); ++index) {
		if (selected == stateActions[index]) {
			_controller->setSelectedPaintState(paintStates.at(index));
			_paintCellState(cellPosition);
			return;
		}
	}
	if (selected == customState) {
		bool ok = false;
		const int value = QInputDialog::getInt(this,
		                                       tr("Cell State"),
		                                       tr("State to paint with:"),
		                                       _controller->hasSelectedPaintState() ? static_cast<int>(_controller->selectedPaintState().value_or(defaultValue))
		                                                                            : defaultValue,
		                                       -1024,
		                                       1024,
		                                       1,
		                                       &ok);
		if (ok) {
			_controller->setSelectedPaintState(value);
			_paintCellState(cellPosition);
		}
		return;
	}
	if (useCurrent != nullptr && selected == useCurrent) {
		_paintCellState(cellPosition);
	}
}
