#pragma once
#include <string>

class Function
{
	std::string identifier;
public:
	Function(std::string identifier);
	std::string getIdentifier();
};

