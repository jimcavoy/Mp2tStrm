#include "RateLimiter.h"

#include <iostream>
#include <thread>

RateLimiter::RateLimiter(QueueType& in, QueueType& out, double fps, int startPosition)
    :_inQueue(in)
    , _outQueue(out)
    , _framePerSeconds(fps)
    , _startPosition(startPosition * 90'000)
{
    if (_framePerSeconds == 0.0)
    {
        std::runtime_error exp("ERROR: Frames per seconds is zero.  Set -f parameter.\n");
        throw exp;
    }
    _window = (long)(90'000 / _framePerSeconds);
}

void RateLimiter::operator() ()
{
    std::chrono::steady_clock::time_point zero{};
    bool add = false;

    while (_run)
    {
        if (_isPaused) 
        {
            // prevent busy-wait and to avoid more complicated solutions using sync mech, such as mutex and critical_sections.
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        else
        {
            AccessUnit au;
            const bool isFull = _inQueue.Get(std::move(au), 10);
            if (isFull)
            {
                if (au.timestamp() == 0)
                {
                    _queue.push(std::move(au));
                }
                else if (_startTime == zero && au.timestamp() != 0 && _startPosition == 0)
                {
                    _startTime = std::chrono::steady_clock::now();
                    _startPts = au.timestamp();
                    _firstPts = _startPts;
                    add = true;
                }
                else if (_startTime == zero && au.timestamp() != 0 && _startPosition > 0)
                {
                    if (_firstPts == 0)
                    {
                        _firstPts = au.timestamp();
                    }

                    uint64_t timespan = au.timestamp() - _firstPts;
                    if (_startPosition < timespan)
                    {
                        _startTime = std::chrono::steady_clock::now();
                        _startPts = au.timestamp();
                        add = true;
                    }
                }

                if (add)
                {
                    _queue.push(std::move(au));
                }
            }
            else if (_queue.empty())
            {
                stop();
            }
            poll();
        }
    }
}

void RateLimiter::start()
{
    if (_isPaused)
    {
        _startTime = std::chrono::steady_clock::now();
        AccessUnit& au = _queue.front();
        _startPts = au.timestamp();
        _isPaused = false;
    }
}

void RateLimiter::stop()
{
    for (int i = 0; i < 10; i++)
    {
        AccessUnit au;
        const bool isFull = _inQueue.Get(std::move(au), 0);
    }
    _run = false;
}

void RateLimiter::pause()
{
    _isPaused = true;
}

uint64_t RateLimiter::count()
{
    uint64_t ret = _framecount;
    _framecount = 0;
    return ret;
}

uint64_t RateLimiter::bytes()
{
    return 0;
}

void RateLimiter::address(char* addr, size_t len)
{
}

long RateLimiter::position()
{
    return _position;
}

void RateLimiter::poll()
{
    std::chrono::steady_clock::time_point zero{};

    if (_queue.empty())
    {
        return;
    }

    if (_startTime != zero)
    {
        AccessUnit& au = _queue.front();
        auto timeNow = std::chrono::steady_clock::now();
        std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(timeNow - _startTime);
        long clockTime = time_span.count() * 90'000;
        long runTime = (long)au.timestamp() - _startPts;

        if (inWindow(clockTime, runTime) ||
            au.timestamp() == 0 ||
            au.timestamp() == _startPts)
        {
            if (runTime > 0.0)
            {
#ifdef DEBUG
                std::cout << runTime << ", " << clockTime << ", " << time_span.count() << std::endl;
#endif
                _position = au.timestamp() - _firstPts;
                _framecount++;
            }
            _outQueue.Put(std::move(au));
            _queue.pop();
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
#ifdef DEBUG
            std::cout << runTime << ", " << clockTime << ", " << time_span.count() << " SKIPPED" << std::endl;
#endif
        }

    }
}

bool RateLimiter::inWindow(long clocktime, long pts)
{
    bool ret = false;

    if (pts == clocktime)
        ret = true;
    else if (pts > clocktime)
    {
        long diff = abs(clocktime - pts);
        if (diff < _window)
        {
            ret = true;
        }
    }
    else
    {
        ret = true;
    }

    return ret;
}
