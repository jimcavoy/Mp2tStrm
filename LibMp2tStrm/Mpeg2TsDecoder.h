#pragma once


#include "tsprsr.h"
#include "tspat.h"
#include "tspmt.h"
#include "tspckt.h"

#include "AccessUnit.h"
#include "PCRClock.h"
#include "Pid2TypeMap.h"
#include "FileReader.h"
#include "UdpSender.h"

#ifdef PERFCNTR
#include "Mp2tPerfCntr/BaseIOInterface.h"
#endif

class Mpeg2TsDecoder : public lcss::TSParser
#ifdef PERFCNTR
	, public BaseIOInterface
#endif
{
	typedef FileReader::QueueType InQueueType;
	typedef UdpSender::QueueType OutQueueType;

public:
	Mpeg2TsDecoder(InQueueType& iqueue, OutQueueType& oqueue);

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
	int _rate;
	PCRClock _pcrClock;
	AccessUnit _currentAU;
};

