#include "JankConnect.h"


JankConnect::JankConnect(std::function<void()> func)
{
	this->func = func;
}

void JankConnect::proxySlot()
{
	func();
}