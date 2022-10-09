#include "Mp2tStreamer.h"

#include "FileReader.h"
#include "Mpeg2TsDecoder.h"
#include "UdpSender.h"

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
			, _sender(other._sender)
		{}
		~Impl() {};
	public:
		uint64_t _tsRead{ 0 };
		uint64_t _udpSent{ 0 };
		ThetaStream::CommandLineParser _arguments;
		FileReader* _fileReader{};
		Mpeg2TsDecoder* _decoder{};
		UdpSender* _sender{};
	};
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

}

void ThetaStream::Mp2tStreamer::init(const ThetaStream::CommandLineParser& arguments)
{
	_pimpl->_arguments = arguments;
}

int ThetaStream::Mp2tStreamer::run()
{
	FileReader::QueueType reader2decoderQueue;
	UdpSender::QueueType decoder2senderQueue;

	FileReader freader(_pimpl->_arguments.sourceFile(), reader2decoderQueue, 188 * 49);
	Mpeg2TsDecoder decoder(reader2decoderQueue, decoder2senderQueue, _pimpl->_arguments.rate());
	UdpSender sender(_pimpl->_arguments.destinationIp(),
		_pimpl->_arguments.destinationPort(),
		decoder2senderQueue,
		_pimpl->_arguments.ttl(),
		_pimpl->_arguments.interfaceAddress());

	_pimpl->_fileReader = &freader;
	_pimpl->_decoder = &decoder;
	_pimpl->_sender = &sender;

	std::thread readerThread{ &FileReader::operator(), &freader };
	std::thread decoderThread{ &Mpeg2TsDecoder::operator(), &decoder };
	std::thread senderThread{ &UdpSender::operator(), &sender };

	readerThread.join();
	decoderThread.join();
	senderThread.join();

	_pimpl->_tsRead = freader.count();
	_pimpl->_udpSent = sender.count();

	return 0;
}

void ThetaStream::Mp2tStreamer::stop()
{
	_pimpl->_fileReader->stop();
	_pimpl->_decoder->stop();
	_pimpl->_sender->stop();
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
