#pragma once
#include <mutex>
#include <bitset>
#include <iomanip>
#include "Bus.h"
#include "Decoder.h"

class SystemBus
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


	class ControlBundle : public Bus<Nctrl>
	{
	public:
		using Bus<Nctrl>::Bus;
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
	template<unsigned int N>	void DisconnectData(const Bundle<N>& b, int start = 0);
	template<unsigned int N>	void DisconnectAddr(const Bundle<N>& b, int start = 0);
	void ConnectCtrl(const Wire& wire, CtrlBit start);
	void DisconnectCtrl(const Wire & wire, CtrlBit start);

	const Bundle<Ndata>& OutData() const { return data; }
	const Bundle<Naddr>& OutAddr() const { return addr; }
	const ControlBundle& OutCtrl() const { return ctrl; }

	void PrintBus(bool lock=true)
	{
		std::cout << (lock ? "L: " : "U: ") << " Addr | Ctrl: IGBKQWR (irq, grant, busreq, ack, req, write, read) ----- Data (by word) ----------" << std::endl;
		std::cout << std::hex << std::left << std::setw(8) << OutAddr().UnsignedRead() << "    |    ";
		std::cout << std::bitset<Nctrl>(OutCtrl().UnsignedRead()) << "     |    ";
		OutData().print(std::cout);
		std::cout << std::dec << std::endl;
	}

	// This is a hack, need a bus arbitrator.
	void LockForBusRequest();
	void UnlockForBusRequest();
	
private:
	std::mutex mBusMutex;

	Bus<Ndata> data;
	Bus<Naddr> addr;
	ControlBundle ctrl;


	friend class ProgramLoader;
	friend class Debugger;
};



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

template<unsigned int N>
inline void SystemBus::DisconnectData(const Bundle<N>& b, int start)
{
	data.Remove(b, start);
}

template<unsigned int N>
inline void SystemBus::DisconnectAddr(const Bundle<N>& b, int start)
{
	addr.Remove(b, start);
}

inline void SystemBus::DisconnectCtrl(const Wire& wire, CtrlBit start)
{
	ctrl.Remove(Bundle<1>(wire), start);
}

inline void SystemBus::LockForBusRequest()
{
	mBusMutex.lock();
#if DEBUG
	PrintBus(true);
#endif
}

inline void SystemBus::UnlockForBusRequest()
{
#if DEBUG
	PrintBus(false);
#endif
	mBusMutex.unlock();
}