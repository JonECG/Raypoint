#pragma once
#include "Value.h"
class VariablePacket
{
	Value currentValue;
	bool ref;
	bool valued;
public:
	VariablePacket();
	explicit VariablePacket(Value currentValue, bool ref = false);
	Value getValue();
	void setValue(Value val);
	bool isReferenceType();
	bool isValued();
};

