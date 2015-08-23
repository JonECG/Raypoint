#pragma once
#include <string>

struct StructureObject
{
	std::string name;
	int numOutlets;
	std::string outletNames[20];
	float outletValues[20];
	bool enabledModes[3];
	std::string codeBlocks[3];

	static StructureObject load( std::string name );
};