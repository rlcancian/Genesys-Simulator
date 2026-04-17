#include "graphicals/GraphicalModelItemRenderStrategy.h"
#include "systempreferences.h"

#include <gtest/gtest.h>

#include <QApplication>
#include <QByteArray>
#include <QColor>
#include <QImage>
#include <QPainter>
#include <QPointF>

#include <algorithm>

namespace {

constexpr int kImageWidth = 240;
constexpr int kImageHeight = 160;

GraphicalModelItemRenderContext baseContext(GraphicalModelItemRenderContext::ItemKind kind) {
    GraphicalModelItemRenderContext context;
    context.kind = kind;
    context.bounds = QRectF(0, 0, 180, 80);
    context.width = 180;
    context.height = 80;
    context.margin = 8;
    context.penWidth = 1;
    context.raise = 5;
    context.selectionWidth = 8;
    context.fillColor = QColor(80, 138, 214, 210);
    context.primaryText = kind == GraphicalModelItemRenderContext::ItemKind::Component
                              ? QStringLiteral("Create")
                              : QStringLiteral("Queue");
    context.secondaryText = kind == GraphicalModelItemRenderContext::ItemKind::DataDefinition
                                ? QStringLiteral("Data")
                                : QString();
    context.selected = true;
    context.breakpoint = kind == GraphicalModelItemRenderContext::ItemKind::Component;
    context.stretchPosTop = 0.5;
    context.stretchPosBottom = 0.5;
    context.stretchPosLeft = 0.5;
    context.stretchPosRight = 0.5;
    context.borderColor = 0x1F2937FF;
    context.raisedColor = 0xF7FAFCFF;
    context.sunkenColor = 0x2D3748FF;
    context.textColor = 0xFFFFFFFF;
    context.textShadowColor = 0x111827AA;
    context.selectionColor = 0x60A5FAFF;
    context.breakpointColor = 0xF87171FF;
    return context;
}

QImage render(const GraphicalModelItemRenderContext& context) {
    QImage image(kImageWidth, kImageHeight, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);

    QPainter painter(&image);
    painter.translate(24, 24);
    GraphicalModelItemRenderer::paint(&painter, context);
    return image;
}

int countPaintedPixels(const QImage& image) {
    int count = 0;
    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            if (qAlpha(image.pixel(x, y)) != 0) {
                ++count;
            }
        }
    }
    return count;
}

int countDifferentPixels(const QImage& left, const QImage& right) {
    const int width = std::min(left.width(), right.width());
    const int height = std::min(left.height(), right.height());
    int count = 0;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (left.pixel(x, y) != right.pixel(x, y)) {
                ++count;
            }
        }
    }
    return count;
}

} // namespace

TEST(GuiRenderStrategy, SelectsStrategyFromInterfaceStylePreference) {
    SystemPreferences::setInterfaceStyle(SystemPreferences::InterfaceStyle::Classic);
    EXPECT_STREQ("classic", GraphicalModelItemRenderer::currentStrategy().name());

    SystemPreferences::setInterfaceStyle(SystemPreferences::InterfaceStyle::Modern);
    EXPECT_STREQ("organic", GraphicalModelItemRenderer::currentStrategy().name());
}

TEST(GuiRenderStrategy, ClassicAndOrganicStrategiesHaveDifferentHitShapes) {
    const auto dataContext = baseContext(GraphicalModelItemRenderContext::ItemKind::DataDefinition);
    const auto componentContext = baseContext(GraphicalModelItemRenderContext::ItemKind::Component);

    SystemPreferences::setInterfaceStyle(SystemPreferences::InterfaceStyle::Classic);
    const QPainterPath classicDataShape = GraphicalModelItemRenderer::shape(dataContext);
    const QPainterPath classicComponentShape = GraphicalModelItemRenderer::shape(componentContext);

    SystemPreferences::setInterfaceStyle(SystemPreferences::InterfaceStyle::Modern);
    const QPainterPath organicDataShape = GraphicalModelItemRenderer::shape(dataContext);
    const QPainterPath organicComponentShape = GraphicalModelItemRenderer::shape(componentContext);

    EXPECT_TRUE(classicDataShape.contains(QPointF(2, 2)));
    EXPECT_FALSE(organicDataShape.contains(QPointF(2, 2)));
    EXPECT_TRUE(organicDataShape.contains(QPointF(dataContext.width / 2.0, dataContext.height / 2.0)));

    EXPECT_TRUE(classicComponentShape.contains(QPointF(2, 2)));
    EXPECT_FALSE(organicComponentShape.contains(QPointF(2, 2)));
    EXPECT_TRUE(organicComponentShape.contains(QPointF(componentContext.width / 2.0, componentContext.height / 2.0)));
}

TEST(GuiRenderStrategy, OrganicRenderingIsNonBlankAndVisiblyDifferentFromClassic) {
    const auto context = baseContext(GraphicalModelItemRenderContext::ItemKind::Component);

    SystemPreferences::setInterfaceStyle(SystemPreferences::InterfaceStyle::Classic);
    const QImage classicImage = render(context);

    SystemPreferences::setInterfaceStyle(SystemPreferences::InterfaceStyle::Modern);
    const QImage organicImage = render(context);

    EXPECT_GT(countPaintedPixels(classicImage), 5000);
    EXPECT_GT(countPaintedPixels(organicImage), 5000);
    EXPECT_GT(countDifferentPixels(classicImage, organicImage), 2500);
}

int main(int argc, char** argv) {
    qputenv("QT_QPA_PLATFORM", QByteArray("offscreen"));
    QApplication app(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    const int result = RUN_ALL_TESTS();
    SystemPreferences::resetToDefaults();
    return result;
}
