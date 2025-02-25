#include <Mp2tStrm/Mp2tStreamer.h>

#include "FileReader.h"
#include "Mpeg2TsDecoder.h"
#include "Mpeg2TsProber.h"
#include "RateLimiter.h"
#include "UdpSender.h"

#ifdef _WIN32
#include <io.h>
#endif
#include <fcntl.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string.h>
#include <thread>

namespace fs = std::filesystem;

namespace ThetaStream
{
    class Mp2tStreamer::Impl
    {
    public:
        Impl() {}
        Impl(ThetaStream::CommandLineParser cmdline)
            :_arguments(cmdline)
        {}

        Impl(const Impl& other)
            :_tsRead(other._tsRead)
            , _udpSent(other._udpSent)
            , _arguments(other._arguments)
            , _fileReader(other._fileReader)
            , _decoder(other._decoder)
            , _limiter(other._limiter)
            , _sender(other._sender)
            , _state(other._state)
        {}
        ~Impl() {};

        void ProbeFile();
    public:
        uint64_t _tsRead{ 0 };
        uint64_t _udpSent{ 0 };
        ThetaStream::CommandLineParser _arguments;
        FileReader* _fileReader{};
        Mpeg2TsDecoder* _decoder{};
        RateLimiter* _limiter{};
        UdpSender* _sender{};
        Mpeg2TsProber _prober;
        std::streamsize _filesize{ 0 };
        ThetaStream::Mp2tStreamer::STATE _state{ ThetaStream::Mp2tStreamer::STATE::STOPPED };
    };

    void Mp2tStreamer::Impl::ProbeFile()
    {
        using namespace std;
        std::shared_ptr<std::istream> ifile;
        std::array<BYTE, 9212> buffer{};

        if (strcmp(_arguments.sourceFile(), "-") == 0)
        {
#ifdef _WIN32
            _setmode(_fileno(stdin), _O_BINARY);
#endif
            ifile.reset(&cin, [](...) {});
        }
        else
        {
            ifstream* tsfile = new std::ifstream(_arguments.sourceFile(), std::ios::binary);
            if (!tsfile->is_open())
            {
                char szErr[512]{};
                sprintf(szErr, "Failed to open input file %s", _arguments.sourceFile());
                std::runtime_error exp(szErr);
                throw exp;
            }
            ifile.reset(tsfile);
        }

        while (true)
        {
            if (ifile->good())
            {
                ifile->read((char*)buffer.data(), 9212);
                _filesize += ifile->gcount();
                bool result = _prober.parse(buffer.data(), (UINT32)ifile->gcount(), true);
                if (!result)
                {
                    if (_filesize == 9212)
                    {
                        fs::path srcPath(_arguments.sourceFile());
                        char szErr[512]{};
                        sprintf(szErr, "Unsupported multimedia container format. The file, %s, is not an MPEG-2 TS file.", srcPath.filename().string().c_str());
                        std::runtime_error exp(szErr);
                        throw exp;
                    }
                    else
                    {
                        cerr << "WARNING: MPEG-2 TS stream is malformed at file position " << _filesize << " bytes." << endl;
                    }
                }
            }
            else
            {
                break;
            }
        }
    }
}

ThetaStream::Mp2tStreamer::Mp2tStreamer()
{
    _pimpl = std::make_unique<ThetaStream::Mp2tStreamer::Impl>();
}

ThetaStream::Mp2tStreamer::Mp2tStreamer(const ThetaStream::CommandLineParser& arguments)
{
    _pimpl = std::make_unique<ThetaStream::Mp2tStreamer::Impl>(arguments);
}

ThetaStream::Mp2tStreamer::~Mp2tStreamer()
{
    delete _pimpl->_fileReader;
    delete _pimpl->_decoder;
    delete _pimpl->_limiter;
    delete _pimpl->_sender;
}

void ThetaStream::Mp2tStreamer::init(const ThetaStream::CommandLineParser& arguments)
{
    _pimpl->_arguments = arguments;
}

void ThetaStream::Mp2tStreamer::probe()
{
    try
    {
        _pimpl->_state = ThetaStream::Mp2tStreamer::STATE::PROBING;
        _pimpl->ProbeFile();
        _pimpl->_state = ThetaStream::Mp2tStreamer::STATE::STOPPED;
    }
    catch (const std::runtime_error& ex)
    {
        _pimpl->_state = ThetaStream::Mp2tStreamer::STATE::STOPPED;
        throw ex;
    }
}

int ThetaStream::Mp2tStreamer::run()
{
    FileReader::QueueType reader2decoderQueue;
    UdpSender::QueueType decoder2LimiterQueue;
    UdpSender::QueueType limiter2senderQueue;

    _pimpl->_fileReader = new FileReader(_pimpl->_arguments.sourceFile(), reader2decoderQueue, _pimpl->_filesize);
    _pimpl->_decoder = new Mpeg2TsDecoder(reader2decoderQueue, decoder2LimiterQueue);
    _pimpl->_limiter = new RateLimiter(decoder2LimiterQueue, limiter2senderQueue, framesPerSecond(), _pimpl->_arguments.startPosition());
    _pimpl->_sender = new UdpSender(_pimpl->_arguments.destinationIp(),
        _pimpl->_arguments.destinationPort(),
        limiter2senderQueue,
        _pimpl->_arguments.ttl(),
        _pimpl->_arguments.interfaceAddress(),
        _pimpl->_arguments.numberOfTsPackets());

    std::thread readerThread{ &FileReader::operator(), _pimpl->_fileReader };
    std::thread decoderThread{ &Mpeg2TsDecoder::operator(), _pimpl->_decoder };
    std::thread limiterThread{ &RateLimiter::operator(), _pimpl->_limiter };
    std::thread senderThread{ &UdpSender::operator(), _pimpl->_sender };

    _pimpl->_state = ThetaStream::Mp2tStreamer::STATE::RUNNING;

    readerThread.join();
    decoderThread.join();
    limiterThread.join();
    _pimpl->_sender->stop();
    senderThread.join();

    _pimpl->_tsRead = _pimpl->_fileReader->count();
    _pimpl->_udpSent = _pimpl->_sender->count();

    _pimpl->_state = ThetaStream::Mp2tStreamer::STATE::STOPPED;
    return 0;
}

void ThetaStream::Mp2tStreamer::start()
{
    if (_pimpl->_state == ThetaStream::Mp2tStreamer::STATE::PAUSED || _pimpl->_state == ThetaStream::Mp2tStreamer::STATE::STOPPED)
    {
        _pimpl->_limiter->start();
        _pimpl->_state = ThetaStream::Mp2tStreamer::STATE::RUNNING;
    }
}

void ThetaStream::Mp2tStreamer::stop()
{
    if (_pimpl->_fileReader != nullptr)
    {
        _pimpl->_fileReader->stop();
    }

    if (_pimpl->_decoder != nullptr)
    {
        _pimpl->_decoder->stop();
    }

    if (_pimpl->_limiter != nullptr)
    {
        _pimpl->_limiter->stop();
    }

    if (_pimpl->_sender != nullptr)
    {
        _pimpl->_sender->stop();
    }

    _pimpl->_state = ThetaStream::Mp2tStreamer::STATE::STOPPED;
}

void ThetaStream::Mp2tStreamer::pause()
{
    if (_pimpl->_state == ThetaStream::Mp2tStreamer::STATE::RUNNING)
    {
        _pimpl->_limiter->pause();
        _pimpl->_state = ThetaStream::Mp2tStreamer::STATE::PAUSED;
    }
}

uint64_t ThetaStream::Mp2tStreamer::tsPacketsRead() const
{
    return _pimpl->_tsRead;
}

uint64_t ThetaStream::Mp2tStreamer::udpPacketsSent() const
{
    return _pimpl->_udpSent;
}

double ThetaStream::Mp2tStreamer::duration() const
{
    return _pimpl->_prober.duration();
}

double ThetaStream::Mp2tStreamer::averageBitrate() const
{
    return _pimpl->_prober.averageBitrate();
}

std::string ThetaStream::Mp2tStreamer::metadataCarriage() const
{
    return _pimpl->_prober.metadataCarriage();
}

int ThetaStream::Mp2tStreamer::metadataFrequency() const
{
    return _pimpl->_prober.metadataFrequency();
}

double ThetaStream::Mp2tStreamer::framesPerSecond() const
{
    // Frame rate pass in as a command line parameter overrides the video frame rate.
    if (_pimpl->_arguments.framesPerSecond() > 0)
        return _pimpl->_arguments.framesPerSecond();

    return _pimpl->_prober.framesPerSecond();
}

int ThetaStream::Mp2tStreamer::width() const
{
    return _pimpl->_prober.h264Prober().width();;
}

int ThetaStream::Mp2tStreamer::height() const
{
    return _pimpl->_prober.h264Prober().height();
}

int ThetaStream::Mp2tStreamer::bytesSent() const
{
    return _pimpl->_sender->bytes();
}

long ThetaStream::Mp2tStreamer::position() const
{
    return _pimpl->_limiter->position();
}

int ThetaStream::Mp2tStreamer::framerate() const
{
    return _pimpl->_limiter->count();
}

ThetaStream::Mp2tStreamer::STATE ThetaStream::Mp2tStreamer::getState() const
{
    return _pimpl->_state;
}
