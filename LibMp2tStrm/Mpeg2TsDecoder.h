#pragma once

#include <chrono>
#include <map>

#include "tsprsr.h"
#include "tspat.h"
#include "tspmt.h"
#include "tspckt.h"

#include "BoundedBuffer.h"
#include "PCRClock.h"
#include "Pid2TypeMap.h"
#include "UdpData.h"

class Mpeg2TsDecoder : public lcss::TSParser
{
	typedef BoundedBuffer<UdpData> InQueueType;
	typedef BoundedBuffer<lcss::TransportPacket> OutQueueType;

public:
	Mpeg2TsDecoder(InQueueType& iqueue, OutQueueType& oqueue, int rate);

	void onPacket(lcss::TransportPacket& pckt) override;

	void operator() ();

	void stop() noexcept;

	uint64_t count() noexcept;

	uint64_t bytes() noexcept;

	void address(char* addr, size_t len) noexcept;

private:
	void updateClock(const lcss::TransportPacket& pckt);
	void outputPacket(lcss::TransportPacket& pckt);

private:
	lcss::ProgramAssociationTable _pat;
	lcss::ProgramMapTable _pmt;
	Pid2TypeMap _pmtHelper;
	InQueueType& _inQueue;
	OutQueueType& _outQueue;
	bool _run;
	std::chrono::time_point<std::chrono::steady_clock> _t0;
	double _pcr0;
	int _rate;
	int _framecount;
	int _version{ -1 };
	PCRClock _pcrClock;
};

