#ifndef GUITHEMEMANAGER_H
#define GUITHEMEMANAGER_H

class QApplication;
class ModelGraphicsView;

class GuiThemeManager {
public:
    static void applyApplicationTheme(QApplication* application);
    static void applyModelGraphicsTheme(ModelGraphicsView* graphicsView);

private:
    GuiThemeManager() = default;
};

#endif // GUITHEMEMANAGER_H
