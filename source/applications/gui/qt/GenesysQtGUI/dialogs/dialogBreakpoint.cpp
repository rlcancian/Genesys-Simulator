/*
 * The MIT License
 *
 * Copyright 2022 rlcancian.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/*
 * File:   dialogBreakpoint.cpp
 * Author: rlcancian
 *
 * Created on 6 de fevereiro de 2022, 14:24
 */

#include "dialogBreakpoint.h"
#include "../../../../plugins/components/Access.h"
#include "../../../../kernel/simulator/ModelDataDefinition.h"
#include "../../../../kernel/util/List.h"

dialogBreakpoint::dialogBreakpoint() {
	widget.setupUi(this);
}

dialogBreakpoint::~dialogBreakpoint() {
}

void dialogBreakpoint::setMVCModel(Simulator* simulator) {
	this->simulator = simulator;
	QStringList breakTypes;
	breakTypes << tr("Time") << tr("Component") << tr("Entity");
	widget.comboBox_Type->addItems(breakTypes);
	widget.comboBox_On->setEnabled(false);
}

dialogBreakpoint::MVCResult* dialogBreakpoint::getMVCResult() {
	std::string breaktype, breakon;
	breaktype = widget.comboBox_Type->currentText().toStdString();
	if (breaktype == "Time")
		breakon = std::to_string(widget.doubleSpinBox_OnTme->value());
	else
		breakon = widget.comboBox_On->currentText().toStdString();
	return new dialogBreakpoint::MVCResult(breaktype, breakon);
}

void dialogBreakpoint::on_comboBox_Type_activated(const QString &arg1) {
	bool isOnTime = arg1 == "Time";
	widget.comboBox_On->setVisible(!isOnTime);
	widget.comboBox_On->setEnabled(true);
	widget.doubleSpinBox_OnTme->setVisible(isOnTime);
	Model* model = simulator->getModelManager()->current();
	ModelSimulation* sim = model->getSimulation();
	widget.doubleSpinBox_OnTme->setMaximum(sim->getReplicationLength());
	widget.doubleSpinBox_OnTme->setSingleStep(sim->getReplicationLength() / 200.0);
	if (!isOnTime) {
		widget.comboBox_On->clear();
		List<ModelDataDefinition*>* datadefs;
		if (arg1 == "Entity") {
			datadefs = model->getDataManager()->getDataDefinitionList(Util::TypeOf<Entity>());
		} else if (arg1 == "Component") {
			datadefs = model->getDataManager()->getDataDefinitionList(Util::TypeOf<Entity>());
		}
		for (ModelDataDefinition* dataDef : *datadefs->list()) {
			widget.comboBox_On->addItem(QString::fromStdString(dataDef->getName()));
		}
	}
}

void dialogBreakpoint::on_comboBox_On_activated(const QString &arg1) {
	//
}

void dialogBreakpoint::on_buttonBox_accepted() {
	//
}
