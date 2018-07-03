#pragma once
#include "Bundle.h"
#include "DeviceController.h"

class SystemBus;

class TerminalController : public DeviceController
{
public:
	TerminalController()
		: DeviceController(L"Terminal Controller Thread")
	{}
	virtual ~TerminalController() 
	{
		StopUpdating();
	}
	virtual void Connect(SystemBus& bus);
	virtual void InternalUpdate();
};

