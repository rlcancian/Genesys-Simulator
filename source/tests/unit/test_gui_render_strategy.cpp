#include "graphicals/GraphicalConnectionStyle.h"
#include "graphicals/GraphicalModelItemRenderStrategy.h"
#include "services/GraphicalDataDefinitionLayout.h"
#include "systempreferences.h"

#include <gtest/gtest.h>

#include <QApplication>
#include <QByteArray>
#include <QColor>
#include <QImage>
#include <QLineF>
#include <QPainter>
#include <QPointF>

#include <algorithm>
#include <cmath>

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

GraphicalModelItemRenderContext geneticPartContext(const QString& partType) {
    GraphicalModelItemRenderContext context = baseContext(GraphicalModelItemRenderContext::ItemKind::DataDefinition);
    context.primaryText = partType;
    context.secondaryText = QStringLiteral("genetic_part");
    context.tertiaryText = QStringLiteral("role=test");
    context.semanticClassName = QStringLiteral("GeneticCircuitPart");
    context.semanticSubtype = partType;
    context.fillColor = QColor(72, 124, 179, 210);
    return context;
}

GraphicalModelItemRenderContext geneticRegulationContext(const QString& regulationType) {
    GraphicalModelItemRenderContext context = baseContext(GraphicalModelItemRenderContext::ItemKind::DataDefinition);
    context.primaryText = regulationType;
    context.secondaryText = QStringLiteral("target_part");
    context.tertiaryText = QStringLiteral("from=regulator_species");
    context.semanticClassName = QStringLiteral("GeneticRegulation");
    context.semanticSubtype = regulationType;
    context.fillColor = QColor(214, 82, 102, 210);
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

TEST(GuiRenderStrategy, GeneticCircuitPartUsesSpecializedGlyphInClassicAndOrganicModes) {
    const auto genericContext = baseContext(GraphicalModelItemRenderContext::ItemKind::DataDefinition);
    const auto promoterContext = geneticPartContext(QStringLiteral("Promoter"));

    SystemPreferences::setInterfaceStyle(SystemPreferences::InterfaceStyle::Classic);
    const QPainterPath genericClassicShape = GraphicalModelItemRenderer::shape(genericContext);
    const QPainterPath promoterClassicShape = GraphicalModelItemRenderer::shape(promoterContext);
    const QImage genericClassicImage = render(genericContext);
    const QImage promoterClassicImage = render(promoterContext);

    EXPECT_TRUE(genericClassicShape.contains(QPointF(2, 2)));
    EXPECT_FALSE(promoterClassicShape.contains(QPointF(2, 2)));
    EXPECT_GT(countDifferentPixels(genericClassicImage, promoterClassicImage), 1500);

    SystemPreferences::setInterfaceStyle(SystemPreferences::InterfaceStyle::Modern);
    const QPainterPath genericModernShape = GraphicalModelItemRenderer::shape(genericContext);
    const QPainterPath promoterModernShape = GraphicalModelItemRenderer::shape(promoterContext);
    const QImage genericModernImage = render(genericContext);
    const QImage promoterModernImage = render(promoterContext);

    EXPECT_TRUE(genericModernShape.contains(QPointF(genericContext.width / 2.0, genericContext.height / 2.0)));
    EXPECT_FALSE(promoterModernShape.contains(QPointF(2, 2)));
    EXPECT_GT(countDifferentPixels(genericModernImage, promoterModernImage), 1500);
}

TEST(GuiRenderStrategy, GeneticRegulationUsesSpecializedGlyphInClassicAndOrganicModes) {
    const auto genericContext = baseContext(GraphicalModelItemRenderContext::ItemKind::DataDefinition);
    const auto activationContext = geneticRegulationContext(QStringLiteral("Activation"));
    const auto repressionContext = geneticRegulationContext(QStringLiteral("Repression"));

    SystemPreferences::setInterfaceStyle(SystemPreferences::InterfaceStyle::Classic);
    const QPainterPath genericClassicShape = GraphicalModelItemRenderer::shape(genericContext);
    const QPainterPath activationClassicShape = GraphicalModelItemRenderer::shape(activationContext);
    const QPainterPath repressionClassicShape = GraphicalModelItemRenderer::shape(repressionContext);
    const QImage genericClassicImage = render(genericContext);
    const QImage activationClassicImage = render(activationContext);
    const QImage repressionClassicImage = render(repressionContext);

    EXPECT_TRUE(genericClassicShape.contains(QPointF(2, 2)));
    EXPECT_FALSE(activationClassicShape.contains(QPointF(2, 2)));
    EXPECT_FALSE(repressionClassicShape.contains(QPointF(2, 2)));
    EXPECT_GT(countDifferentPixels(genericClassicImage, activationClassicImage), 1500);
    EXPECT_GT(countDifferentPixels(activationClassicImage, repressionClassicImage), 500);

    SystemPreferences::setInterfaceStyle(SystemPreferences::InterfaceStyle::Modern);
    const QPainterPath genericModernShape = GraphicalModelItemRenderer::shape(genericContext);
    const QPainterPath activationModernShape = GraphicalModelItemRenderer::shape(activationContext);
    const QPainterPath repressionModernShape = GraphicalModelItemRenderer::shape(repressionContext);
    const QImage genericModernImage = render(genericContext);
    const QImage activationModernImage = render(activationContext);
    const QImage repressionModernImage = render(repressionContext);

    EXPECT_TRUE(genericModernShape.contains(QPointF(genericContext.width / 2.0, genericContext.height / 2.0)));
    EXPECT_FALSE(activationModernShape.contains(QPointF(2, 2)));
    EXPECT_FALSE(repressionModernShape.contains(QPointF(2, 2)));
    EXPECT_GT(countDifferentPixels(genericModernImage, activationModernImage), 1500);
    EXPECT_GT(countDifferentPixels(activationModernImage, repressionModernImage), 500);
}

TEST(GuiConnectionStyle, ModernModelPathUsesCurveAndSupportsDistanceBasedSampling) {
    const QPointF source(0.0, 20.0);
    const QPointF destination(180.0, 110.0);

    const QPainterPath classicPath = GraphicalConnectionStyle::modelConnectionPath(
        source,
        destination,
        GraphicalConnectionStyle::RouteType::Horizontal,
        false);
    const QPainterPath modernPath = GraphicalConnectionStyle::modelConnectionPath(
        source,
        destination,
        GraphicalConnectionStyle::RouteType::Horizontal,
        true);

    EXPECT_GT(modernPath.length(), QLineF(source, destination).length());
    EXPECT_NEAR(classicPath.pointAtPercent(0.0).x(), source.x(), 0.001);
    EXPECT_NEAR(classicPath.pointAtPercent(1.0).y(), destination.y(), 0.001);

    const QPointF curvedMidpoint = GraphicalConnectionStyle::pointAtProgress(modernPath, 0.5);
    const QPointF straightMidpoint = (source + destination) / 2.0;
    EXPECT_GT(QLineF(curvedMidpoint, straightMidpoint).length(), 5.0);
}

TEST(GuiConnectionStyle, ModernDiagramPathCurvesAwayFromStraightLine) {
    const QPointF source(20.0, 20.0);
    const QPointF destination(200.0, 20.0);

    const QPainterPath classicPath = GraphicalConnectionStyle::diagramConnectionPath(source, destination, false);
    const QPainterPath modernPath = GraphicalConnectionStyle::diagramConnectionPath(source, destination, true);

    EXPECT_EQ(classicPath.elementCount(), 2);
    EXPECT_GT(modernPath.elementCount(), classicPath.elementCount());
    EXPECT_GT(std::abs(GraphicalConnectionStyle::pointAtProgress(modernPath, 0.5).y() - source.y()), 10.0);
}

TEST(GuiDataDefinitionLayout, UpperArcPlacesEditableChildrenAboveAnchorWithoutOverlap) {
    const QRectF componentBounds(300.0, 260.0, 150.0, 100.0);
    const QSizeF gmddSize(180.0, 63.0);

    const QPointF left = GraphicalDataDefinitionLayout::arcPosition(componentBounds, gmddSize, 0, 3, true);
    const QPointF center = GraphicalDataDefinitionLayout::arcPosition(componentBounds, gmddSize, 1, 3, true);
    const QPointF right = GraphicalDataDefinitionLayout::arcPosition(componentBounds, gmddSize, 2, 3, true);

    EXPECT_LT(left.y() + gmddSize.height(), componentBounds.top());
    EXPECT_LT(center.y() + gmddSize.height(), componentBounds.top());
    EXPECT_LT(right.y() + gmddSize.height(), componentBounds.top());
    EXPECT_LT(left.x(), center.x());
    EXPECT_LT(center.x(), right.x());
    EXPECT_NEAR(center.x() + gmddSize.width() / 2.0, componentBounds.center().x(), 0.001);
    EXPECT_LT(center.y(), left.y());
    EXPECT_GT(right.x() - left.x(), gmddSize.width());
}

TEST(GuiDataDefinitionLayout, LowerArcPlacesNonEditableChildrenBelowAnchorWithoutOverlap) {
    const QRectF parentBounds(300.0, 260.0, 180.0, 63.0);
    const QSizeF gmddSize(180.0, 63.0);

    const QPointF left = GraphicalDataDefinitionLayout::arcPosition(parentBounds, gmddSize, 0, 3, false);
    const QPointF center = GraphicalDataDefinitionLayout::arcPosition(parentBounds, gmddSize, 1, 3, false);
    const QPointF right = GraphicalDataDefinitionLayout::arcPosition(parentBounds, gmddSize, 2, 3, false);

    EXPECT_GT(left.y(), parentBounds.bottom());
    EXPECT_GT(center.y(), parentBounds.bottom());
    EXPECT_GT(right.y(), parentBounds.bottom());
    EXPECT_LT(left.x(), center.x());
    EXPECT_LT(center.x(), right.x());
    EXPECT_NEAR(center.x() + gmddSize.width() / 2.0, parentBounds.center().x(), 0.001);
    EXPECT_GT(center.y(), left.y());
    EXPECT_GT(right.x() - left.x(), gmddSize.width());
}

TEST(GuiDataDefinitionLayout, RadialLayerKeepsUpperChildrenOnSameArcSide) {
    const QRectF parentBounds(300.0, 260.0, 180.0, 63.0);
    const QSizeF gmddSize(180.0, 63.0);

    const QPointF firstLayer = GraphicalDataDefinitionLayout::arcPosition(parentBounds, gmddSize, 1, 3, true);
    const QPointF secondLayer = GraphicalDataDefinitionLayout::arcPosition(parentBounds, gmddSize, 1, 3, true, 1);

    EXPECT_NEAR(firstLayer.x(), secondLayer.x(), 0.001);
    EXPECT_LT(secondLayer.y(), firstLayer.y());
    EXPECT_LT(secondLayer.y() + gmddSize.height(), parentBounds.top());
}

TEST(GuiDataDefinitionLayout, RadialLayerKeepsLowerChildrenOnSameArcSide) {
    const QRectF parentBounds(300.0, 260.0, 180.0, 63.0);
    const QSizeF gmddSize(180.0, 63.0);

    const QPointF firstLayer = GraphicalDataDefinitionLayout::arcPosition(parentBounds, gmddSize, 1, 3, false);
    const QPointF secondLayer = GraphicalDataDefinitionLayout::arcPosition(parentBounds, gmddSize, 1, 3, false, 1);

    EXPECT_NEAR(firstLayer.x(), secondLayer.x(), 0.001);
    EXPECT_GT(secondLayer.y(), firstLayer.y());
    EXPECT_GT(secondLayer.y(), parentBounds.bottom());
}

int main(int argc, char** argv) {
    qputenv("QT_QPA_PLATFORM", QByteArray("offscreen"));
    QApplication app(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    const int result = RUN_ALL_TESTS();
    SystemPreferences::resetToDefaults();
    return result;
}
