#pragma once

#include <h264p/h264prsr.h>

#include <memory>
#include <string>

class H264Prober :
	public ThetaStream::H264Parser
{
public:
	H264Prober();
	virtual ~H264Prober();

	virtual void onNALUnit(ThetaStream::NALUnit& nalu) override;

	double framesPerSecond() const;
	int width() const;
	int height() const;
	std::string profile() const;
	std::string level() const;
	int numberOfFrames() const;
	bool isInterlaced() const;
	int aspectRatioX() const;
	int aspectRatioY() const;


public:
	class Impl;

private:
	std::unique_ptr<Impl> _pimpl;
};
