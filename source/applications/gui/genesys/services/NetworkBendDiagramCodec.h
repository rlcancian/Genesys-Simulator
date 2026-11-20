#pragma once

#include "../extensions/GuiExtensionContracts.h"
#include "../graphicals/NetworkBendGraphicsItem.h"

#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/model/Model.h"
#include "kernel/simulator/model/ModelDataManager.h"
#include "kernel/simulator/model/ModelManager.h"

#include <QHash>
#include <QLocale>
#include <QMap>
#include <QPointF>
#include <QTextStream>
#include <QUrl>

#include <any>
#include <cmath>
#include <utility>
#include <functional>

/**
 * Optional .gui geometry for network relations not derived from ModelComponent.
 *
 * An opaque route key is always checked against a currently projected edge.
 * A missing, renamed or reordered relation discards its old bend rather than
 * applying it to a different mathematical relation.
 */
struct NetworkBendDiagramLayout {
    QString networkName;
    QString networkClassname;
    QMap<QString, QPointF> bends;
};

namespace NetworkBendDiagramCodec {

inline QString encode(const QString& value) {
    return QString::fromUtf8(QUrl::toPercentEncoding(value));
}

inline QString decode(const QString& value) {
    return QUrl::fromPercentEncoding(value.toUtf8());
}

inline QString relationKey(qulonglong ordinalOrId,
                           const QString& source, const QString& destination,
                           const QString& name) {
    // Length-free, collision-free tokenization after URL encoding.
    return QString::number(ordinalOrId) + QLatin1Char('|')
        + encode(source) + QLatin1Char('|')
        + encode(destination) + QLatin1Char('|')
        + encode(name);
}

inline std::any capture(ModelDataDefinition* network, ModelGraphicsScene* scene,
                        const QString& extensionId) {
    if (network == nullptr || scene == nullptr || scene->networkEditorOwner() != network)
        return {};
    NetworkBendDiagramLayout layout;
    layout.networkName = QString::fromStdString(network->getName());
    layout.networkClassname = QString::fromStdString(network->getClassname());
    for (QGraphicsItem* item : scene->extensionOverlayItems(extensionId)) {
        auto* edge = dynamic_cast<NetworkBendGraphicsItem*>(item);
        if (edge == nullptr || !edge->hasManualBendPoint()) continue;
        layout.bends.insert(edge->routeKey(), edge->manualBendPoint());
    }
    return std::any(std::move(layout));
}

inline void restore(ModelDataDefinition* network, ModelGraphicsScene* scene,
                    const std::any& data, const QString& extensionId) {
    auto* layout = std::any_cast<NetworkBendDiagramLayout>(&data);
    if (network == nullptr || scene == nullptr || layout == nullptr
        || scene->networkEditorOwner() != network
        || layout->networkName != QString::fromStdString(network->getName())
        || layout->networkClassname != QString::fromStdString(network->getClassname()))
        return;

    for (QGraphicsItem* item : scene->extensionOverlayItems(extensionId)) {
        auto* edge = dynamic_cast<NetworkBendGraphicsItem*>(item);
        if (edge == nullptr) continue;
        auto it = layout->bends.constFind(edge->routeKey());
        if (it != layout->bends.constEnd()
            && std::isfinite(it.value().x()) && std::isfinite(it.value().y()))
            edge->setManualBendPoint(it.value());
    }
}

inline QStringList serialize(Simulator* simulator, const GuiDiagramLayouts& layouts) {
    QStringList lines;
    if (simulator == nullptr || simulator->getModelManager() == nullptr) return lines;
    Model* model = simulator->getModelManager()->current();
    if (model == nullptr || model->getDataManager() == nullptr) return lines;

    for (const auto& [networkId, data] : layouts) {
        auto* layout = std::any_cast<NetworkBendDiagramLayout>(&data);
        if (layout == nullptr || layout->bends.isEmpty()) continue;
        ModelDataDefinition* network = model->getDataManager()->getDataDefinition(
            layout->networkClassname.toStdString(), layout->networkName.toStdString());
        if (network == nullptr || network->getId() != networkId) continue;
        for (auto bend = layout->bends.constBegin(); bend != layout->bends.constEnd(); ++bend) {
            if (!std::isfinite(bend.value().x()) || !std::isfinite(bend.value().y())) continue;
            lines << QStringList({
                QString::number(networkId),
                QStringLiteral("NetworkBend"),
                QStringLiteral("version=1"),
                QStringLiteral("classname=") + encode(layout->networkClassname),
                QStringLiteral("name=") + encode(layout->networkName),
                QStringLiteral("key=") + encode(bend.key()),
                QStringLiteral("x=") + QString::number(bend.value().x(), 'g', 17),
                QStringLiteral("y=") + QString::number(bend.value().y(), 'g', 17)
            }).join(QLatin1Char('\t'));
        }
    }
    return lines;
}

inline GuiDiagramLayouts parse(Model* model, const QStringList& lines,
                               const std::function<bool(ModelDataDefinition*)>& isSupported) {
    GuiDiagramLayouts layouts;
    if (model == nullptr || model->getDataManager() == nullptr) return layouts;
    for (const QString& line : lines) {
        const QStringList fields = line.split(QLatin1Char('\t'), Qt::SkipEmptyParts);
        if (fields.size() < 8 || fields.at(1).trimmed() != QStringLiteral("NetworkBend"))
            continue;
        QHash<QString, QString> attributes;
        for (int i = 2; i < fields.size(); ++i) {
            const int equals = fields.at(i).indexOf(QLatin1Char('='));
            if (equals > 0)
                attributes.insert(fields.at(i).left(equals).trimmed(),
                                  fields.at(i).mid(equals + 1).trimmed());
        }
        if (attributes.value(QStringLiteral("version")) != QStringLiteral("1")
            || !attributes.contains(QStringLiteral("classname"))
            || !attributes.contains(QStringLiteral("name"))
            || !attributes.contains(QStringLiteral("key"))
            || !attributes.contains(QStringLiteral("x"))
            || !attributes.contains(QStringLiteral("y")))
            continue;

        const QString classname = decode(attributes.value(QStringLiteral("classname")));
        const QString name = decode(attributes.value(QStringLiteral("name")));
        const QString key = decode(attributes.value(QStringLiteral("key")));
        if (key.isEmpty()) continue;

        bool okX = false, okY = false;
        const double x = QLocale::c().toDouble(attributes.value(QStringLiteral("x")), &okX);
        const double y = QLocale::c().toDouble(attributes.value(QStringLiteral("y")), &okY);
        if (!okX || !okY || !std::isfinite(x) || !std::isfinite(y)) continue;

        // Rebind by the unique live classname/name (IDs can be remapped by a
        // model load). The opaque relation key is checked again at restore.
        ModelDataDefinition* network = model->getDataManager()->getDataDefinition(
            classname.toStdString(), name.toStdString());
        if (network == nullptr || (isSupported && !isSupported(network))) continue;
        auto [it, inserted] = layouts.try_emplace(static_cast<unsigned long>(network->getId()));
        if (inserted) {
            it->second = NetworkBendDiagramLayout{name, classname, {}};
        }
        auto* layout = std::any_cast<NetworkBendDiagramLayout>(&it->second);
        if (layout != nullptr && layout->networkName == name
            && layout->networkClassname == classname
            && !layout->bends.contains(key)) {
            layout->bends.insert(key, QPointF(x, y));
        }
    }
    return layouts;
}

} // namespace NetworkBendDiagramCodec
