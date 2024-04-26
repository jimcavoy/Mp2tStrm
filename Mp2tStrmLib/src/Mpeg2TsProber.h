#pragma once

#include <mp2tp/tsprsr.h>
#include "H264Prober.h"

#include <memory>
#include <string>

class Mpeg2TsProber : public lcss::TSParser
{
public:
	Mpeg2TsProber();
	virtual ~Mpeg2TsProber();

	virtual void onPacket(lcss::TransportPacket& pckt) override;

	double duration() const;
	double averageBitrate() const;
	std::string metadataCarriage() const;
	int metadataFrequency() const;
	double framesPerSecond() const;

	const H264Prober& h264Prober() const;

private:
	class Impl;
	std::unique_ptr<Impl> _pimpl;
};
