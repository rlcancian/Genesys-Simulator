#ifndef DIALOGSYSTEMPREFERENCES_H
#define DIALOGSYSTEMPREFERENCES_H

#include <QDialog>

class Simulator;

namespace Ui {
	class DialogSystemPreferences;
}

class DialogSystemPreferences : public QDialog
{
	Q_OBJECT

public:
    explicit DialogSystemPreferences(Simulator* simulator = nullptr, QWidget *parent = nullptr);
	~DialogSystemPreferences();

private:
    void loadPreferencesIntoUi();
    void savePreferencesFromUi();
    void populateTraceLevels();
    void browseSpecificModelFile();

	Ui::DialogSystemPreferences *ui;
    Simulator* _simulator = nullptr;
};

#endif // DIALOGSYSTEMPREFERENCES_H
