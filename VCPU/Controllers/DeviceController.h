#pragma once
#include "Bundle.h"
#include "Tools/MagicBundle.h"
#include "ThreadedComponent.h"

class DeviceController
{
public:
	typedef Bundle<32> DataBundle;

	DeviceController() {}
	virtual void Connect(const DataBundle& in) = 0;

	const DataBundle& GetControl() { return control; }
	const DataBundle& GetData() { return data; }

private:
	virtual void Update() = 0;
	MagicBundle<32> control;
	MagicBundle<32> data;
};

class KeyboardController : public DeviceController
{
public:
	virtual void Connect(const DataBundle& in);
	
private:
	virtual void Update();
	void GetChar();
	char c_in;
	bool pending;
};


class TerminalController : public DeviceController
{
public:
	virtual void Connect(const DataBundle& in);
	
private:
	virtual void Update();
};

