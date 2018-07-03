#pragma once
#include "DeviceController.h"

class SystemBus;

class TerminalController : public DeviceController
{
public:
	TerminalController()
		: DeviceController(L"Terminal Controller Thread")
	{}
	virtual ~TerminalController() = default;
	virtual void Connect(SystemBus& bus);
	virtual void InternalUpdate();
};

