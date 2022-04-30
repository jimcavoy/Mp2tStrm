#pragma once

#include "CommandLineParser.h"

namespace ThetaStream
{
	class Mp2tStreamer
	{
	public:
		Mp2tStreamer();
		Mp2tStreamer(const ThetaStream::CommandLineParser& arguments);
		~Mp2tStreamer();

		void init(const ThetaStream::CommandLineParser& arguments);

		int run();

		void stop();

		void pause();

		uint64_t tsPacketsRead() const;

		uint64_t udpPacketsSent() const;

	private:
		class Impl;
		Impl* _pimpl;
	};
}

