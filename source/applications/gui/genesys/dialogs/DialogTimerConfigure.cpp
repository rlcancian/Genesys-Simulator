#include "DialogTimerConfigure.h"
#include "ui_DialogTimerConfigure.h"

DialogTimerConfigure::DialogTimerConfigure(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogTimerConfigure)
{
    ui->setupUi(this);
    QPushButton *okButton = ui->buttonBox->button(QDialogButtonBox::Ok);
    if (okButton) {
        okButton->setAutoDefault(true);
        okButton->setDefault(true);
    }

    QPushButton *cancelButton = ui->buttonBox->button(QDialogButtonBox::Cancel);
    if (cancelButton) {
        cancelButton->setAutoDefault(false);
        cancelButton->setDefault(false);
    }
}

DialogTimerConfigure::~DialogTimerConfigure()
{
    delete ui;
}

void DialogTimerConfigure::setAnimationTimer(AnimationTimer *timer)
{
    _animationTimer = timer;

    if (_animationTimer->getTimeFormat() == Util::TimeFormat::twelve) {
        ui->radioButtonTwelve->setChecked(true);
        ui->radioButtonTwentyFour->setChecked(false);
        ui->spinBoxHour->setMaximum(12);
    } else {
        ui->radioButtonTwelve->setChecked(false);
        ui->radioButtonTwentyFour->setChecked(true);
        ui->spinBoxHour->setMaximum(24);
    }

    ui->spinBoxHour->setValue(_animationTimer->getInitialHours());
    ui->spinBoxMinute->setValue(_animationTimer->getInitialMinutes());
    ui->spinBoxSecond->setValue(_animationTimer->getInitialSeconds());
}

void DialogTimerConfigure::updateAnimationTimer()
{
    if (ui->radioButtonTwelve->isChecked()) {
        _animationTimer->setTimeFormat(Util::TimeFormat::twelve);
    } else {
        _animationTimer->setTimeFormat(Util::TimeFormat::twentyFour);
    }

    _animationTimer->setInitialHours(ui->spinBoxHour->value());
    _animationTimer->setInitialMinutes(ui->spinBoxMinute->value());
    _animationTimer->setInitialSeconds(ui->spinBoxSecond->value());

    _animationTimer->setTime(0.0);
}

void DialogTimerConfigure::on_buttonBox_accepted()
{
    updateAnimationTimer();
}

void DialogTimerConfigure::on_radioButtonTwelve_clicked(bool checked)
{
    int hour = ui->spinBoxHour->value();
    if (checked) {
        if (hour == 0 || hour == 24) {
            ui->spinBoxHour->setValue(12);
        } else if (hour >= 1 && hour <= 12) {
            // Horas entre 1 e 12 permanecem as mesmas
            ui->spinBoxHour->setValue(ui->spinBoxHour->value());
        } else if (hour >= 13 && hour <= 23) {
            ui->spinBoxHour->setValue(hour - 12);
        }
        ui->spinBoxHour->setMaximum(12); // Seta o máximo para 12
    }
}

void DialogTimerConfigure::on_radioButtonTwentyFour_clicked(bool checked)
{
    if (checked) {
        ui->spinBoxHour->setMaximum(24); // Seta o máximo para 24
    }
}

