/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   TraitsApp.h
 * Author: rafael.luiz.cancian
 *
 * Created on 14 de Agosto de 2018, 19:36
 */

#ifndef TRAITSAPP_H
#define TRAITSAPP_H

#include "GenesysApplication_if.h"
#include "../kernel/simulator/TraceManager.h"
template <typename T>
struct TraitsApp {
};

/*!
 *  Configure the Genesys Application
 */
template <> struct TraitsApp<GenesysApplication_if> {
	static const TraceManager::Level traceLevel = TraceManager::Level::L9_mostDetailed;
	static const bool runTests = false; //!< If false, Genesys will run a GUI app or a Terminal app, dependending on "runGraphicalUserInterface". If true, run google tests.
    static const bool runGraphicalUserInterface = true; //!< If false, a terminal application will be compiled and executed (See TraitsTerminalApp.h). If true, a GUI for a general simulator will be executed.
};

#endif /* TRAITSAPP_H */

