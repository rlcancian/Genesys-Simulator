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
 * File:   dialogBreakpoint.h
 * Author: rlcancian
 *
 * Created on 6 de fevereiro de 2022, 14:24
 */

#ifndef _DIALOGBREAKPOINT_H
#define _DIALOGBREAKPOINT_H

#include "ui_dialogBreakpoint.h"
#include "kernel/simulator/Simulator.h"

class dialogBreakpoint : public QDialog {

	Q_OBJECT
public:
	class MVCResult {
	public:

		MVCResult(std::string type, std::string on) {
			this->type = type;
			this->on = on;
		}
		std::string type;
		std::string on;
	};
public:
	dialogBreakpoint();
	virtual ~dialogBreakpoint();
public:
	void setMVCModel(Simulator* simulator);
	dialogBreakpoint::MVCResult* getMVCResult();
private slots:
	void on_comboBox_Type_activated(const QString &arg1);
	void on_comboBox_On_activated(const QString &arg1);
	void on_buttonBox_accepted();
private:
	Ui::dialogBreakpoint widget;
	Simulator* simulator;
};

#endif /* _DIALOGBREAKPOINT_H */
