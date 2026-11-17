#include "../../GuiExtensionPluginCatalog.h"

#include <QGraphicsRectItem>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

#include <algorithm>
#include <memory>

// An independent, real QGraphicsItem: no kernel object, concrete plugin
// reference, AnimationPlaceholder inheritance or generic host special case.
namespace {
class SceneMarkerItem final : public QGraphicsRectItem {
public:
    SceneMarkerItem() : QGraphicsRectItem(0.0, 0.0, 176.0, 72.0) {
        setFlags(ItemIsMovable | ItemIsSelectable | ItemIsFocusable);
        setAcceptHoverEvents(true);
    }

    void paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) override {
        painter->setRenderHint(QPainter::Antialiasing, true);
        const QRectF r = rect();
        painter->setPen(QPen(QColor(34, 85, 125), isSelected() ? 3.0 : 1.5));
        painter->setBrush(QColor(224, 243, 253));
        painter->drawRoundedRect(r, 7.0, 7.0);
        painter->setPen(QColor(20, 63, 92));
        painter->drawText(r.adjusted(10, 4, -10, -12), Qt::AlignCenter,
                          QStringLiteral("GUI scene marker"));
        // Draw a resize handle; corner dragging changes this item's bounds.
        painter->setBrush(QColor(53, 121, 171));
        painter->drawRect(QRectF(r.right() - 10, r.bottom() - 10, 9, 9));
    }

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override {
        if (event->button() == Qt::LeftButton &&
            QRectF(rect().right() - 14, rect().bottom() - 14, 14, 14)
                .contains(event->pos())) {
            _resizing = true;
            event->accept();
            return;
        }
        QGraphicsRectItem::mousePressEvent(event);
    }

    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override {
        if (_resizing) {
            const QPointF point = event->pos();
            setRect(QRectF(0.0, 0.0,
                           std::max<qreal>(48.0, point.x()),
                           std::max<qreal>(32.0, point.y())));
            event->accept();
            return;
        }
        QGraphicsRectItem::mouseMoveEvent(event);
    }

    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override {
        if (_resizing) {
            _resizing = false;
            event->accept();
            return;
        }
        QGraphicsRectItem::mouseReleaseEvent(event);
    }

private:
    bool _resizing = false;
};

class SceneMarkerGuiExtensionPlugin final : public GuiExtensionPlugin {
public:
    std::string extensionId() const override { return "gui.scene-marker.f2"; }
    std::string displayName() const override { return "Scene Marker"; }
    std::string version() const override { return "1.0.0"; }
    std::vector<std::string> requiredModelPlugins() const override { return {}; }

    void registerContributions(GuiExtensionRegistry* registry) const override {
        if (!registry) return;
        GuiAnimationToolContribution tool;
        tool.id = "gui.scene-marker.f2.insert";
        tool.text = "Scene Marker";
        tool.menuPath = "View/Animations";
        tool.statusTip = "Insert a movable and resizable GUI-only scene marker";
        tool.createItem = [](const GuiExtensionRuntimeContext&) -> std::unique_ptr<QGraphicsItem> {
            return std::make_unique<SceneMarkerItem>();
        };
        registry->addAnimationTool(std::move(tool));
    }
};
} // namespace

REGISTER_GUI_EXTENSION_PLUGIN(SceneMarkerGuiExtensionPlugin);
