#include <Mp2tStrm/Mp2tStreamer.h>

#include "FileReader.h"
#include "Mpeg2TsDecoder.h"
#include "Mpeg2TsProber.h"
#include "RateLimiter.h"
#include "UdpSender.h"

#ifdef PERFCNTR
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <Mp2tPerfCntr/Mp2tStrmCounter.h>
#endif

#ifdef _WIN32
#include <io.h>
#endif
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <string.h>
#include <thread>

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
#ifdef _WIN32
				sprintf_s(szErr, "Failed to open input file %s", _arguments.sourceFile());
#else
				sprintf(szErr, "Failed to open input file %s", filename);
#endif
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
				const streamsize len = ifile->gcount();
				_prober.parse(buffer.data(), (UINT32)len);
				_filesize += len;
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
	_pimpl->ProbeFile();
}

int ThetaStream::Mp2tStreamer::run()
{
	FileReader::QueueType reader2decoderQueue;
	UdpSender::QueueType decoder2LimiterQueue;
	UdpSender::QueueType limiter2senderQueue;

	_pimpl->_fileReader = new FileReader(_pimpl->_arguments.sourceFile(), reader2decoderQueue, _pimpl->_filesize);
	_pimpl->_decoder = new Mpeg2TsDecoder(reader2decoderQueue, decoder2LimiterQueue);
	_pimpl->_limiter = new RateLimiter(decoder2LimiterQueue, limiter2senderQueue, framesPerSecond());
	_pimpl->_sender = new UdpSender(_pimpl->_arguments.destinationIp(),
		_pimpl->_arguments.destinationPort(),
		limiter2senderQueue,
		_pimpl->_arguments.ttl(),
		_pimpl->_arguments.interfaceAddress());
#ifdef PERFCNTR
	Mp2tStrmCounter perfCounter(*_pimpl->_fileReader, *_pimpl->_limiter, *_pimpl->_sender);
#endif

	std::thread readerThread{ &FileReader::operator(), _pimpl->_fileReader };
	std::thread decoderThread{ &Mpeg2TsDecoder::operator(), _pimpl->_decoder };
	std::thread limiterThread{ &RateLimiter::operator(), _pimpl->_limiter};
	std::thread senderThread{ &UdpSender::operator(), _pimpl->_sender };
#ifdef PERFCNTR
	std::thread perfCounterThread{ &Mp2tStrmCounter::operator(), &perfCounter };
#endif

	readerThread.join();
	decoderThread.join();
	limiterThread.join();
	_pimpl->_sender->stop();
	senderThread.join();
#ifdef PERFCNTR
	perfCounter.stop();
	perfCounterThread.join();
#else
	_pimpl->_tsRead = _pimpl->_fileReader->count();
	_pimpl->_udpSent = _pimpl->_sender->count();
#endif
	return 0;
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
}

void ThetaStream::Mp2tStreamer::pause()
{

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
	return _pimpl->_prober.h264Prober().framesPerSecond();
}

int ThetaStream::Mp2tStreamer::width() const
{
	return _pimpl->_prober.h264Prober().width();;
}

int ThetaStream::Mp2tStreamer::height() const
{
	return _pimpl->_prober.h264Prober().height();
}
