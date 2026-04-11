#ifndef TRAITSGUI_H
#define TRAITSGUI_H

#include <cstdint>
#include <string>

template <class T>
struct TraitsGUI {
};

// include only features for this GUI, such as theme, default widgets, ...

class GMainWindow;
template <> struct TraitsGUI<GMainWindow> {
	static const bool startMaximized = true;
	static const int zoomButtonChange = 30;
	static const uint64_t consoleTextColor = 0x008000FF;
};

class GModelGraphicPic;
template <> struct TraitsGUI<GModelGraphicPic> {
	static const int nodeComponentFontSize = 12;
	static const uint64_t nodeComponentFontColor = 0x000000FF;
	static const uint64_t nodeComponentFillColor = 0x80A040FF;
	static const int edgeComponentFontSize = 7;
	static const uint64_t edgeComponentEdgeColor = 0x000000FF;
	static const uint64_t edgeComponentFontColor = 0x000000FF;
	static const int nodeDatadefInternalFontSize = 8;
	static const uint64_t nodeDatadefInternalColor = 0xA0A0A0FF;
	static const uint64_t nodeDatadefInternalFontColor = 0xA0A0A0FF;
	static const int nodeDatadefAttachedFontSize = 8;
	static const uint64_t nodeDatadefAttachedFontColor = 0x808080FF;
};

class GAssociation;
template <> struct TraitsGUI<GAssociation> {
	static const uint64_t foreColor = 0x808080FF;
	constexpr static const int margin = 2;
	constexpr static const int selectionWidth = 8;
};

class GConnection;
template <> struct TraitsGUI<GConnection> {
	static const uint64_t foreColor = 0x000000FF;
	constexpr static const int margin = 2;
	constexpr static const int selectionWidth = 8;
};

class GView;
template <> struct TraitsGUI<GView> {
	constexpr static const int sceneCenter = 1024;
	constexpr static const int sceneDistanceCenter = 1024;
	static const uint64_t backgroundEnabledColor = 0xFFFF8040;
	static const uint64_t backgroundDisabledColor = 0xC0C0C020;
};

class GScene;
template <> struct TraitsGUI<GScene> {
	constexpr static const int gridInterval = 20;
	constexpr static const int gridPenWidth = 1;
	static const uint64_t gridColor = 0xC0C0C0E0;
};

class GComponentPort;
template <> struct TraitsGUI<GComponentPort> {
	constexpr static const int width = 20;
	constexpr static const int penWidth = 1;
	constexpr static const int raise = 3;
	constexpr static const int margin = 2;
	static const uint64_t connectedBackgroundColor = 0x2020FF80;
	static const uint64_t unconnectedBackgroundColor = 0xFF202040;
	static const uint64_t borderColor = 0x000000FF;
	static const uint64_t pathRaised = 0x808080FF;
	static const uint64_t pathStunken = 0xFFFFFFFF;
	static const uint64_t textColor = 0xFFFFFFFF;
	static const uint64_t textShadowColor = 0x000000FF;
	static const uint64_t selectionSquaresColor = 0x000000FF;
};

class GModelDataDefinition;
template <> struct TraitsGUI<GModelDataDefinition> {//<GraphicalModelComponent> {
    constexpr static const int opacity = 256*0.33;
    constexpr static const int width = 180;
	constexpr static const int penWidth = 1;
    constexpr static const int raise = 5;
    constexpr static const int margin = 8;
	constexpr static const int selectionWidth = 8;
	constexpr static const float stretchPos = 0.5;
	constexpr static const float stretch = 0.0;
    constexpr static const float heightProportion = 0.35;
	static const uint64_t breakpointColor = 0xFF0000FF;
	static const uint64_t borderColor = 0x000000FF;
    static const uint64_t pathRaised = 0x808080FF;
    static const uint64_t pathStunken = 0xFFFFFFFF;
    static const uint64_t textColor = 0x0; // 0xFFFFFFFF;
	static const uint64_t textShadowColor = 0x000000FF;
	static const uint64_t selectionSquaresColor = 0x000000FF;
};

class GModelComponent;
template <> struct TraitsGUI<GModelComponent> {//<GraphicalModelComponent> {
	constexpr static const int opacity = 256*0.75;
	constexpr static const int width = 150;
	constexpr static const int penWidth = 1;
	constexpr static const int raise = 5;
	constexpr static const int margin = 8;
	constexpr static const int selectionWidth = 8;
	constexpr static const float stretchPos = 0.5;
	constexpr static const float stretch = 0.0;
	constexpr static const float heightProportion = 0.67;
	static const uint64_t breakpointColor = 0xFF0000FF;
	static const uint64_t borderColor = 0x000000FF;
	static const uint64_t pathRaised = 0xFFFFFFFF;
	static const uint64_t pathStunken = 0x808080FF;
	static const uint64_t textColor = 0xFFFFFFFF;
	static const uint64_t textShadowColor = 0x000000FF;
	static const uint64_t selectionSquaresColor = 0x000000FF;
};

#endif // TRAITSGUI_H
