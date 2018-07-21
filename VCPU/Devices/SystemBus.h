#pragma once
#include <bitset>
#include <iomanip>
#include <sstream>
#include <fstream>
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
		IRQ,
		BusReq0,
		BusGrant0,
		BusReq1,
		BusGrant1,
		MAX,
	};
	static const int Nctrl = CtrlBit::MAX;
	static const int Naddr = 32;
	static const int Ndata = 256;
	static const int NReqLines = (CtrlBit::MAX - CtrlBit::BusReq0) / 2;

	static CtrlBit GetBusReq(int which) { return (CtrlBit)((int)BusReq0 + 2 * which); }
	static CtrlBit GetBusGrant(int which) { return (CtrlBit)((int)BusGrant0 + 2 * which); }

	class ControlBundle : public Bus<Nctrl>
	{
	public:
		using Bus<Nctrl>::Bus;
		const Wire& Read() const { return Get(CtrlBit::Read); }
		const Wire& Write() const { return Get(CtrlBit::Write); }
		const Wire& Req() const { return Get(CtrlBit::Req); }
		const Wire& Ack() const { return Get(CtrlBit::Ack); }
		const Wire& IRQ() const { return Get(CtrlBit::IRQ); }
		const Wire& BusReq(int which) const { return Get(SystemBus::GetBusReq(which) ); }
		const Wire& BusGrant(int which) const { return Get(SystemBus::GetBusGrant(which)); }

		void print(std::ostream& s) const
		{
			for (int i = CtrlBit::MAX - 1; i >= 0; i--)
			{
				s << "   " << Get(i);
			}
		}
	};
	
	void ConnectData(const BundleAny& b, int start = 0);
	template <unsigned int N>	void ConnectAddr(const Bundle<N>& b, int start = 0);
	void DisconnectData(const BundleAny& b, int start = 0);
	template<unsigned int N>	void DisconnectAddr(const Bundle<N>& b, int start = 0);
	void ConnectCtrl(const Wire& wire, CtrlBit start);
	void DisconnectCtrl(const Wire & wire, CtrlBit start);

	const Bundle<Ndata>& OutData() const { return data; }
	const Bundle<Naddr>& OutAddr() const { return addr; }
	const ControlBundle& OutCtrl() const { return ctrl; }

	void WriteBus()
	{
		static std::ofstream f;
		if (!f.is_open())
		{
			f.open("busout.txt", std::ofstream::out);
		}
		f << ToString();
		f.flush();
	}

	void PrintBus(bool lock = true)
	{
		std::cout << ToString();
	}

	inline std::string ToString()
	{
		std::stringstream ss;
		ss << "  Address  |  G1  R1  G0  R0  IRQ ACK REQ  W  RD  | ----- Data (by word) ----------" << std::endl;
		ss << "  " << std::hex << std::left << std::setw(8) << OutAddr().UnsignedRead() << " |";
		OutCtrl().print(ss);
		ss << "  |  ";
		OutData().print(ss);
		ss << std::dec << std::endl;
		return ss.str();
	}
	
private:
	Bus<Ndata> data;
	Bus<Naddr> addr;
	ControlBundle ctrl;


	friend class ProgramLoader;
	friend class Debugger;
};



inline void SystemBus::ConnectData(const BundleAny& b, int start)
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

inline void SystemBus::DisconnectData(const BundleAny& b, int start)
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
