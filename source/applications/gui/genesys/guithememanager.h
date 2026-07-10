#ifndef GUITHEMEMANAGER_H
#define GUITHEMEMANAGER_H

class QApplication;
class ModelGraphicsView;

/**
 * @brief Applies the visual theme used by the Qt application and model views.
 *
 * The theme manager centralizes palette-level adjustments so the application
 * and graphics scene remain visually consistent when preferences change.
 */
class GuiThemeManager {
public:
    /**
     * @brief Applies application-wide theme settings to the Qt application object.
     * @param application Target QApplication instance.
     */
    static void applyApplicationTheme(QApplication* application);
    /**
     * @brief Applies the model-graphics theme to a specific graphics view.
     * @param graphicsView Target graphics view.
     */
    static void applyModelGraphicsTheme(ModelGraphicsView* graphicsView);

private:
    GuiThemeManager() = default;
};

#endif // GUITHEMEMANAGER_H
