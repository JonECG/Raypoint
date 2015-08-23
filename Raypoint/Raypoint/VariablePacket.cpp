#include "VariablePacket.h"

VariablePacket::VariablePacket()
{
	valued = false;
}
VariablePacket::VariablePacket(Value currentValue, bool ref)
{
	this->currentValue = currentValue;
	this->ref = ref;
	valued = true;
}
Value VariablePacket::getValue()
{
	return currentValue;
}
void VariablePacket::setValue(Value val)
{
	this->currentValue = val;
}
bool VariablePacket::isReferenceType()
{
	return ref;
}
bool VariablePacket::isValued()
{
	return valued;
}

