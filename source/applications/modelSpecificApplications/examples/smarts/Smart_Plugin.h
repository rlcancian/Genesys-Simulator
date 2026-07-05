/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cppFiles/class.h to edit this template
 */

/* 
 * File:   Smart_Plugin.h
 * Author: rlcancian
 *
 * Created on 13 de janeiro de 2022, 17:47
 */

#ifndef SMART_PLUGIN_H
#define SMART_PLUGIN_H

#include "../../../BaseGenesysTerminalApplication.h"

class Smart_Plugin : public BaseGenesysTerminalApplication {
public:
	Smart_Plugin();
public:
	virtual int main(int argc, char** argv) override;
};

#endif /* SMART_PLUGIN_H */

