#pragma once

#include <QMainWindow>
#include <QObject>
#include <QWidget>
#include "kernel/simulator/Simulator.h"
#include "ui_mainwindow.h"

/**
 * @brief Compatibility base class that binds the Qt UI and the simulator model.
 *
 * Legacy GUI controllers in Genesys share the same two core pointers: the main
 * window UI object and the active simulator instance. This base class keeps
 * that pairing in one place so higher-level controllers can reuse it.
 */
class BaseMVController
{
public:
	BaseMVController();
protected: // View (UI) and Model (Simulator) main elements to join to the Controller
	/// MainWindow UI built from the Qt Designer form.
	Ui::MainWindow *ui;
	/// Active simulator instance owned elsewhere by the application.
	Simulator* simulator;
};
