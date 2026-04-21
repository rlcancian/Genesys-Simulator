/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   TraitsTerminalApp.h
 * Author: rafael.luiz.cancian
 *
 * Created on 29/09/2023
 */

#pragma once

#include "GenesysShell/GenesysShell.h"

template <typename T>
struct TraitsTerminalApp {
};

/*!
 *  Configure the Genesys Application to be compiled and executed
 */
template <>
struct TraitsTerminalApp<GenesysApplication_if> {
#ifndef GENESYS_TERMINAL_APP_CLASS
#define GENESYS_TERMINAL_APP_CLASS GenesysShell
#endif
    typedef GENESYS_TERMINAL_APP_CLASS Application;
};
