#pragma once
#include "Tools/MagicBundle.h"
#include "DeviceController.h"

class SystemBus;

class KeyboardController : public DeviceController
{
public:
	KeyboardController()
		: DeviceController(L"Keyboard Controller Thread")
	{}
	virtual ~KeyboardController();
	virtual void Connect(SystemBus& bus);
	virtual void InternalUpdate();
	
private:
	MagicBundle<8> c_in;
};
