#pragma once
#include "BoundedBuffer.h"
#include "UdpData.h"

#include <memory>
#include <array>

#include "BaseIOInterface.h"


#ifndef QSIZE
#define QSIZE 100
#endif

class FileReader : public BaseIOInterface
{
public:
    typedef BoundedBuffer<UdpData, QSIZE> QueueType;

public:
    FileReader(const char* filename, QueueType& q, std::streamsize filesize);
    ~FileReader();

    void start() override;

    void stop() override;

    void pause() override;

    uint64_t count() override;

    uint64_t bytes() override;

    long position() override;

    void address(char* addr, size_t len) override;

    void operator () ();

private:
    bool _run;
    QueueType& _queue;
    uint64_t _count;
    uint64_t _bytes;
    std::shared_ptr<std::istream> _ifile;
    std::array<uint8_t, 9212> _buffer;
    size_t _bufsiz{ 9212 };
    std::string _address;
    std::streamsize _readcount{ 0 };
    std::streamsize _filesize{ 0 };
};

