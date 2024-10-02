#pragma once

#include <mp2tp/tsprsr.h>
#include <mp2tp/tspat.h>
#include <mp2tp/tspmt.h>
#include <mp2tp/tspckt.h>

#include "AccessUnit.h"
#include "PCRClock.h"
#include "Pid2TypeMap.h"
#include "FileReader.h"
#include "UdpSender.h"

#include "BaseIOInterface.h"


class Mpeg2TsDecoder : public lcss::TSParser
    , public BaseIOInterface
{
    typedef FileReader::QueueType InQueueType;
    typedef UdpSender::QueueType OutQueueType;

public:
    Mpeg2TsDecoder(InQueueType& iqueue, OutQueueType& oqueue);

    void onPacket(lcss::TransportPacket& pckt) override;

    void operator() ();

    void start() override;

    void stop() override;

    void pause() override;

    uint64_t count() override;

    uint64_t bytes() override;

    long position() override;

    void address(char* addr, size_t len) override;

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
    PCRClock _pcrClock;
    AccessUnit _currentAU;
};

