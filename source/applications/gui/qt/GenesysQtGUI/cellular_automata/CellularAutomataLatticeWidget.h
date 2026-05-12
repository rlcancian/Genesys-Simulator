#pragma once

#include <QPoint>
#include <QRectF>
#include <QWidget>

class CellularAutomataViewerController;

class CellularAutomataLatticeWidget : public QWidget {
public:
	explicit CellularAutomataLatticeWidget(QWidget* parent = nullptr);

	void setController(CellularAutomataViewerController* controller);

protected:
	void paintEvent(QPaintEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	void contextMenuEvent(QContextMenuEvent* event) override;
	QSize minimumSizeHint() const override;

private:
	bool _ensureSelectedPaintState();
	QPoint _cellAtPosition(const QPoint& widgetPosition) const;
	QRectF _gridRect() const;
	void _paintCellState(const QPoint& cellPosition);
	void _showStateContextMenu(const QPoint& globalPosition, const QPoint& cellPosition);

private:
	CellularAutomataViewerController* _controller = nullptr;
	bool _dragging = false;
	QPoint _lastPaintedCell = QPoint{-1, -1};
};
