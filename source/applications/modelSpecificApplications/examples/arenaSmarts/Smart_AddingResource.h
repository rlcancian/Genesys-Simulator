/* 
 * File:   AddingResource.h
 * Author: genesys
 *
 * Created on 10 de dezembro de 2022, 17:57
 */

#ifndef ADDINGRESOURCE_H
#define ADDINGRESOURCE_H

#include "../../../BaseGenesysTerminalApplication.h"

class Smart_AddingResource : public BaseGenesysTerminalApplication {
public:
	Smart_AddingResource();
public:
	virtual int main(int argc, char** argv) override;
};

#endif /* ADDINGRESOURCE_H */

