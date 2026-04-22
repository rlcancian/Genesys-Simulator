#pragma once

#include <QMainWindow>
#include <QObject>
#include <QWidget>
#include "kernel/simulator/Simulator.h"
#include "ui_mainwindow.h"


class BaseMVController
{
public:
	BaseMVController();
protected: // View (UI) and Model (Simulator) main elements to join to the Controller
	Ui::MainWindow *ui;
	Simulator* simulator;
};
