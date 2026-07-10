#ifndef DIALOGTIMERCONFIGURE_H
#define DIALOGTIMERCONFIGURE_H

#include <QDialog>
#include <QPushButton>

#include "../animations/AnimationTimer.h"

namespace Ui {
    class DialogTimerConfigure;
}

class DialogTimerConfigure : public QDialog
{
    Q_OBJECT

public:
    explicit DialogTimerConfigure(QWidget *parent = nullptr);
    ~DialogTimerConfigure();

    // recebe o Timer
    void setAnimationTimer(AnimationTimer *animationTimer);
    // atualiza os valores no Timer
    void updateAnimationTimer();

private slots:
    // slot para tratar o clique no botão "OK"
    void on_buttonBox_accepted();

    // slot para tratar mudança no formato de hora para 12 horas
    void on_radioButtonTwelve_clicked(bool checked);

    // slot para tratar mudança no formato de hora para 24 horas
    void on_radioButtonTwentyFour_clicked(bool checked);

private:
    Ui::DialogTimerConfigure *ui;
    AnimationTimer *_animationTimer;
};

#endif // DIALOGTIMERCONFIGURE_H
