#include "Mpeg2TsDecoder.h"

#include "PCRClock.h"

#include "tspckt.h"
#include "tsadptfd.h"
#include "tspes.h"
#include "tspmt.h"
#include "tsmetadata.h"
#include "tsnit.h"

#include <iostream>
#include <sstream>

static auto t0 = std::chrono::steady_clock::now();

namespace
{
	const BYTE PTS_DTS_MASK = 0xC0;
	char TAG_HDMV[] = { (char)0x48, (char)0x44, (char)0x4D, (char)0x56, (char)0xFF, (char)0x1B, (char)0x44, (char)0x3F, 0 };
	char TAG_HDPR[] = { (char)0x48, (char)0x44, (char)0x50, (char)0x52, (char)0xFF, (char)0x1B, (char)0x67, (char)0x3F, 0 };

	void PrintTimestamp(const char* type, const lcss::PESPacket& pes, PCRClock& pcrClock)
	{
		using namespace std;
		
		UINT16 pts_dts_flag = (pes.flags2() & PTS_DTS_MASK);
		auto t1 = chrono::steady_clock::now();
		chrono::duration<double> time_span = t1 - t0;
		double pcr = pcrClock.timeInSeconds();

		if (pts_dts_flag == 0xC0)
		{
			cout.precision(12);
			std::stringstream pts;
			std::stringstream dts;
			pts.precision(12);
			dts.precision(12);

			pts << pes.pts() << "," << pes.ptsInSeconds();
			dts << pes.dts() << "," << pes.dtsInSeconds();
			cout << type << "," << pts.str() << "," << dts.str() << ",";
			cout << time_span.count() << "," << pcr << endl;
		}
		else if (pts_dts_flag == 0x80)
		{
			cout.precision(12);
			std::stringstream pts;
			pts.precision(12);

			pts << pes.pts() << "," << pes.ptsInSeconds();
			cout << type << "," << pts.str() << ", , ,";
			cout << time_span.count() << "," << pcr << endl;
		}
	}
}

using namespace std;

Mpeg2TsDecoder::Mpeg2TsDecoder(Mpeg2TsDecoder::InQueueType& iqueue, Mpeg2TsDecoder::OutQueueType& oqueue, int rate)
	:_inQueue(iqueue)
	, _outQueue(oqueue)
	, _run(true)
	, _pcr0(0.0)
	, _rate(rate)
	, _framecount(0)
{

}

void Mpeg2TsDecoder::onPacket(lcss::TransportPacket& pckt)
{
	const BYTE* data = pckt.getData();
	updateClock(pckt);

	if (pckt.payloadUnitStart())
	{
		if (pckt.PID() == 0) // Program Association Table
		{
			_pat.parse(data);
		}
		else if (_pat.find(pckt.PID()) != _pat.end()) // Program Specific Information Table, chapter 2.4.4
		{
			auto it = _pat.find(pckt.PID());
			if (it->second != 0)
			{
				_pmt.clear();
				_pmt.add(data, pckt.data_byte());
				if (_pmt.parse())
				{
					_pmtHelper.update(_pmt);
				}
			}
		}
		else // Packetize Elementary Stream packet
		{
			lcss::PESPacket pes;
			const UINT16 bytesParsed = pes.parse(data);
			if (bytesParsed > 0)
			{
				switch (_pmtHelper.packetType(pckt.PID()))
				{
				case Pid2TypeMap::STREAM_TYPE::VIDEO:
				case Pid2TypeMap::STREAM_TYPE::H264:
				case Pid2TypeMap::STREAM_TYPE::H265:
				case Pid2TypeMap::STREAM_TYPE::HDMV:
				{
					//PrintTimestamp("Video", pes);
					_framecount++;
					break;
				}
				case Pid2TypeMap::STREAM_TYPE::AUDIO:
					//PrintTimestamp("Audio", pes);
					break;
				case Pid2TypeMap::STREAM_TYPE::KLVA:
					//PrintTimestamp("KLVA", pes);
					break;
				}
			}
		}
	}
	else // The rest of the packets in a PES
	{
		// The program map table may span multiple TS packets
		auto it = _pat.find(pckt.PID());
		if (it != _pat.end() && it->second != 0)
		{
			_pmt.add(data, pckt.data_byte());
			if (_pmt.parse())
			{
				_pmtHelper.update(_pmt);
			}
		}
	}
	outputPacket(pckt);
}

void Mpeg2TsDecoder::operator()()
{
	//using namespace std;

	// Print the header for the csv file
	//cout << "Type,PTS,PTS(secs),DTS,DTS(secs),Wallclock,PCR" << endl;

	while (_run)
	{
		UdpData d(nullptr, UdpData::DEFAULT_BUFLEN);
		const bool isFull = _inQueue.Get(std::move(d), 100);
		if (isFull)
		{
			parse(d.data(), (UINT32)d.length());
		}
		else
		{
			stop();
		}
	}
}

void Mpeg2TsDecoder::stop() noexcept
{
	for (int i = 0; i < 10; i++)
	{
		UdpData d(nullptr, UdpData::DEFAULT_BUFLEN);
		const bool isFull = _inQueue.Get(std::move(d), 0);
	}

	_run = false;
}

uint64_t Mpeg2TsDecoder::count() noexcept
{
	uint64_t ret = _framecount;
	_framecount = 0;
	return ret;
}

uint64_t Mpeg2TsDecoder::bytes() noexcept
{
	return uint64_t();
}

void Mpeg2TsDecoder::address(char* addr, size_t len) noexcept
{
}

void Mpeg2TsDecoder::updateClock(const lcss::TransportPacket& pckt)
{
	using namespace std::chrono;

	const char afe = pckt.adaptationFieldExist();
	if (afe == 0x02 || afe == 0x03)
	{
		const lcss::AdaptationField* adf = pckt.getAdaptationField();
		if (adf != nullptr && adf->length() > 0 && adf->PCR_flag())
		{
			BYTE pcr[6]{};
			adf->getPCR(pcr);
			_pcrClock.setTime(pcr);

			if (_pcr0 == 0)
			{
				_t0 = std::chrono::steady_clock::now();
				_pcr0 = _pcrClock.timeInSeconds();
			}
			else
			{
				double pcr1 = _pcrClock.timeInSeconds();
				auto t1 = steady_clock::now();
				duration<double> time_span = (t1 - _t0);
				int timedInterval = (int)(time_span.count() * 1000);
				_t0 = t1;

				int pcrInterval = (int)((pcr1 - _pcr0) * 1000) + _rate;
				_pcr0 = pcr1;

				int interval = pcrInterval;
#ifndef NDEBUG
				cerr.precision(12);
				cerr << pcr1 << " " << time_span.count() << " " << pcrInterval << " " << timedInterval << " " << endl;
#endif
				if (interval > 0)
				{
					std::this_thread::sleep_for(chrono::milliseconds(interval));
				}
			}
		}
	}
}

void Mpeg2TsDecoder::outputPacket(lcss::TransportPacket& pckt)
{
	_outQueue.Put(std::move(pckt));
}
