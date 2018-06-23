#pragma once

#include "Component.h"
#include "Bus.h"
#include "AndGate.h"
#include "NandGate.h"
#include "NorGate.h"
#include "Decoder.h"

class SystemBus : public Component
{
public:
	enum CtrlBit
	{
		Read,
		Write,
		Req,
		Ack,
		BusReq,
		BusGrant,
		IRQ,
		MAX,
	};
	static const int Nctrl = CtrlBit::MAX;
	static const int Naddr = 32;
	static const int Ndata = 256;


	class ControlBundle : public Bundle<Nctrl>
	{
	public:
		using Bundle<Nctrl>::Bundle;
		const Wire& Read() const { return Get(CtrlBit::Read); }
		const Wire& Write() const { return Get(CtrlBit::Write); }
		const Wire& Req() const { return Get(CtrlBit::Req); }
		const Wire& Ack() const { return Get(CtrlBit::Ack); }
		const Wire& BusReq() const { return Get(CtrlBit::BusReq); }
		const Wire& BusGrant() const { return Get(CtrlBit::BusGrant); }
		const Wire& IRQ() const { return Get(CtrlBit::IRQ); }
	};
	
	template <unsigned int N>	void ConnectData(const Bundle<N>& b, int start = 0);
	template <unsigned int N>	void ConnectAddr(const Bundle<N>& b, int start = 0);
	void ConnectCtrl(const Wire& wire, CtrlBit start);
	void Update();
	
private:
	const Bundle<Ndata>& OutData() const { return data.Out(); }
	const Bundle<Naddr>& OutAddr() const { return addr.Out(); }
	const ControlBundle& OutCtrl() const { return static_cast<const ControlBundle&>(ctrl.Out()); }

	Bus<Ndata> data;
	Bus<Naddr> addr;
	Bus<Nctrl> ctrl;

	friend class SystemBusBuffer;
	friend class ProgramLoader;
};


inline void SystemBus::Update()
{
	data.Update();
	addr.Update();
	ctrl.Update();
}

template<unsigned int N>
inline void SystemBus::ConnectData(const Bundle<N>& b, int start)
{
	data.Connect(b, start);
}

template<unsigned int N>
inline void SystemBus::ConnectAddr(const Bundle<N>& b, int start)
{
	addr.Connect(b, start);
}

inline void SystemBus::ConnectCtrl(const Wire& wire, CtrlBit start)
{
	ctrl.Connect(Bundle<1>(wire), start);
}