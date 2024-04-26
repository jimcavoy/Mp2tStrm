#include "Mpeg2TsProber.h"

#include "PCRClock.h"
#include "Pid2TypeMap.h"
#include "H264Prober.h"


#include <mp2tp/tspckt.h>
#include <mp2tp/tsadptfd.h>
#include <mp2tp/tspat.h>
#include <mp2tp/tspes.h>
#include <mp2tp/tspmt.h>
#include <mp2tp/tsmetadata.h>
#include <mp2tp/tsnit.h>

#include <string>

#define ONESEC 90000

class Mpeg2TsProber::Impl
{
public:
	void onPacket(lcss::TransportPacket& pckt);
	void updateSystemClock(const lcss::TransportPacket& pckt);

public:
	std::string _filename;
	lcss::ProgramAssociationTable _pat;
	lcss::ProgramMapTable _pmt;
	double _startPTS{};
	double _curPTS{};
	H264Prober _h264p;
	Pid2TypeMap _pmtProxy;
	int _metadataCarriage{};
	int _klvSetCount{};
	int _frameCount{};
	UINT64 _startKlvPTS{};
	UINT64 _prevKlvPTS{};
	UINT64 _systemTime{};
	PCRClock _pcrClock;
};

void Mpeg2TsProber::Impl::onPacket(lcss::TransportPacket& pckt)
{
	bool isAdaptationField = false;
	// Get the TS packet payload minus header
	const BYTE* data = pckt.getData();

	updateSystemClock(pckt);

	if (pckt.payloadUnitStart())
	{
		if (pckt.PID() == 0 && _pat.size() == 0) // Program Association Table
		{
			_pat.parse(data);
		}
		else if (_pat.find(pckt.PID()) != _pat.end()) // Program Map Table
		{
			auto it = _pat.find(pckt.PID());
			if (it->second != 0) // program 0 is assigned to Network Information Table
			{
				_pmt = lcss::ProgramMapTable(data, pckt.data_byte());
				if (_pmt.parse())
				{
					_pmtProxy.update(_pmt);
				}
			}
		}
		else
		{
			lcss::PESPacket pes;
			const UINT16 bytesParsed = pes.parse(data);
			if (bytesParsed > 0)
			{
				switch (_pmtProxy.packetType(pckt.PID()))
				{
				case Pid2TypeMap::STREAM_TYPE::VIDEO:
				case Pid2TypeMap::STREAM_TYPE::H264:
				case Pid2TypeMap::STREAM_TYPE::H265:
				case Pid2TypeMap::STREAM_TYPE::HDMV:
				{
					if (_startPTS == 0.0)
						_startPTS = pes.ptsInSeconds();
					double curpts = pes.ptsInSeconds();
					if (curpts > 0.0 && curpts > _startPTS && curpts > _curPTS)
						_curPTS = curpts;

					if (_curPTS - _startPTS < 1.0)
					{
						_frameCount++;
					}
				}
				break;
				case Pid2TypeMap::STREAM_TYPE::KLVA:
				{
					_metadataCarriage = pes.stream_id() == 0xFC ? 1 : 2;
					if (pes.stream_id() == 0xFC)
					{
						if (_startKlvPTS == 0)
						{
							_startKlvPTS = pes.pts();
						}

						_prevKlvPTS = pes.pts();

						if (pes.pts() - _startKlvPTS < ONESEC)
						{
							_klvSetCount++;
						}
					}
					else
					{
						if (_startKlvPTS == 0)
						{
							_startKlvPTS = _systemTime;
						}

						if ((_systemTime - _startKlvPTS) < ONESEC)
						{
							_klvSetCount++;
						}
					}
				}
				break;
				}
			}
		}
	}
	else
	{
		// Program Map Table (PMT) may span multiple TS packets
		if (_pat.find(pckt.PID()) != _pat.end())
		{
			auto it = _pat.find(pckt.PID());
			if (it->second != 0)
			{
				_pmt.add(data, pckt.data_byte());
				if (_pmt.parse())
				{
					_pmtProxy.update(_pmt);
				}
			}
		}
	}

	switch (_pmtProxy.packetType(pckt.PID()))
	{
	case Pid2TypeMap::STREAM_TYPE::H264:
	case Pid2TypeMap::STREAM_TYPE::H265:
	case Pid2TypeMap::STREAM_TYPE::HDMV:
		_h264p.parse(data, pckt.data_byte());
		break;
	}
}

void Mpeg2TsProber::Impl::updateSystemClock(const lcss::TransportPacket& pckt)
{
	const char afe = pckt.adaptationFieldExist();
	if (afe == 0x02 || afe == 0x03) {

		const lcss::AdaptationField* adf = pckt.getAdaptationField();
		if (adf != nullptr && adf->length() > 0 && adf->PCR_flag())
		{
			BYTE pcr[6]{};
			adf->getPCR(pcr);
			_pcrClock.setTime(pcr);
			_systemTime = _pcrClock.baseTime(); // Use the 90 kHz
		}
	}

}

Mpeg2TsProber::Mpeg2TsProber()
	:_pimpl(std::make_unique<Mpeg2TsProber::Impl>())
{
	
}

Mpeg2TsProber::~Mpeg2TsProber()
{
}

void Mpeg2TsProber::onPacket(lcss::TransportPacket& pckt)
{
	_pimpl->onPacket(pckt);
}

double Mpeg2TsProber::duration() const
{
	return _pimpl->_curPTS - _pimpl->_startPTS;
}

double Mpeg2TsProber::averageBitrate() const
{
	return (packetCount() * 188 * 8) / (duration() * 1000);
}

std::string Mpeg2TsProber::metadataCarriage() const
{
	switch (_pimpl->_metadataCarriage)
	{
	case 0: return "No Metadata"; break;
	case 1: return "Synchronous"; break;
	case 2: return "Asynchronous"; break;
	default: return "Unknown";
	}
}

int Mpeg2TsProber::metadataFrequency() const
{
	return _pimpl->_klvSetCount;
}

double Mpeg2TsProber::framesPerSecond() const
{
	double fps = _pimpl->_h264p.framesPerSecond();

	return fps > 0 ? fps : (double) _pimpl->_frameCount;
}

const H264Prober& Mpeg2TsProber::h264Prober() const
{
	return _pimpl->_h264p;
}


