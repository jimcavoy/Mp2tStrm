#pragma once

#include "CommandLineParser.h"

#include <memory>
#include <string>

namespace ThetaStream
{
    class Mp2tStreamer
    {
    public:
        enum class STATE
        {
            STOPPED,
            RUNNING,
            PAUSED,
            PROBING
        };
    public:
        Mp2tStreamer();
        Mp2tStreamer(const ThetaStream::CommandLineParser& arguments);
        ~Mp2tStreamer();

        void init(const ThetaStream::CommandLineParser& arguments);

        void probe();

        int run();

        void start();

        void stop();

        void pause();

        uint64_t tsPacketsRead() const;

        uint64_t udpPacketsSent() const;

        double duration() const;

        double averageBitrate() const;

        std::string metadataCarriage() const;

        int metadataFrequency() const;

        double framesPerSecond() const;

        int width() const;

        int height() const;

        int bytesSent() const;

        long position() const;

        int framerate() const;

        STATE getState() const;

    private:
        class Impl;
        std::unique_ptr<Impl> _pimpl;
    };
}

