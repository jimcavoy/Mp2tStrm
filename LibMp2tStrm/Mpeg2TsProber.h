#pragma once

#include "tsprsr.h"

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

private:
	class Impl;
	std::unique_ptr<Impl> _pimpl;
};
