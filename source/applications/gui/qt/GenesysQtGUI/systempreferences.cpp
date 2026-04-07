#include "systempreferences.h"

bool SystemPreferences::_startMaximized = true;
bool SystemPreferences::_autoLoadPlugins = true;
bool SystemPreferences::_checkSystemPackagesAtStart = true;
unsigned short int SystemPreferences::_openModelAtStart = 2;
std::string SystemPreferences::_modelfilenameToOpen = "";

bool SystemPreferences::load()
{
    return false;
}

bool SystemPreferences::save()
{
    return false;
}

bool SystemPreferences::startMaximized()
{
    return _startMaximized;
}

void SystemPreferences::setStartMaximized(bool newStartMaximized)
{
    _startMaximized = newStartMaximized;
}

bool SystemPreferences::autoLoadPlugins()
{
    return _autoLoadPlugins;
}

void SystemPreferences::setAutoLoadPlugins(bool newAutoLoadPlugins)
{
    _autoLoadPlugins = newAutoLoadPlugins;
}

unsigned short SystemPreferences::modelAtStart()
{
    return _openModelAtStart;
}

void SystemPreferences::setModelAtStart(unsigned short newModelAtStart)
{
    if (newModelAtStart>=0 && newModelAtStart<=2)
        _openModelAtStart = newModelAtStart;
}

std::string SystemPreferences::modelfilename()
{
    return _modelfilenameToOpen;
}

void SystemPreferences::setModelfilename(const std::string &newModelfilename)
{
    _modelfilenameToOpen = newModelfilename;
}

bool SystemPreferences::checkSystemPackagesAtStart()
{
    return _checkSystemPackagesAtStart;
}

void SystemPreferences::setCheckSystemPackagesAtStart(bool newCheckSystemPackagesAtStart)
{
    _checkSystemPackagesAtStart = newCheckSystemPackagesAtStart;
}
