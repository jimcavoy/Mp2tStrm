#pragma once

#include "UdpSender.h"

#include "BaseIOInterface.h"


#include <chrono>
#include <queue>


class RateLimiter : public BaseIOInterface
{
    typedef UdpSender::QueueType QueueType;

public:
    RateLimiter(QueueType& in, QueueType& out, double fps, int startPosition);

    void operator() ();

    void start() override;

    void stop() override;

    void pause() override;

    uint64_t count() override;

    uint64_t bytes() override;

    void address(char* addr, size_t len) override;

    long position() override;

private:
    void poll();
    bool inWindow(long clocktime, long pts);

private:
    QueueType& _inQueue;
    QueueType& _outQueue;
    bool _run{ true };
    double _framePerSeconds{};
    std::chrono::steady_clock::time_point _startTime{};
    uint64_t _startPts{};
    std::queue<AccessUnit> _queue;
    long _window;
    uint64_t _framecount{};
    long _position{};
    long _startPosition{};
    bool _isPaused{ false };
};
