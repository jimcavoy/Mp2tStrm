#pragma once
#include <stddef.h>
#include <cstdint>

class BaseIOInterface
{
public:
    BaseIOInterface(void);
    virtual ~BaseIOInterface(void);

    virtual void start() = 0;

    virtual void stop() = 0;

    virtual void pause() = 0;

    virtual uint64_t count() = 0;

    virtual uint64_t bytes() = 0;

    virtual void address(char* addr, size_t len) = 0;

    virtual long position() = 0;
};

