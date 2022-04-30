#include "CommandLineParser.h"

#include <exception>
#include <iostream>
#include <string>
#include <sstream>

using namespace std;

const char* usage = "Usage: Mp2tStrm -s<MPEG_transport_stream_file> -d<socket_addr>";
const char* opts = "  -s\tSource MPEG-2 TS file path (default: -).\n \
 -d\tDestination socket address (i.e. IP Address:Port).\n \
 -t\tTime to Live. (default: 16)\n \
 -i\tSpecifies the network interface IP address for the destination stream.\n \
 -r\tAdjust the default playout rate in milliseconds.  Positive values slowdown \n \
   \tthe video streaming.\n \
 -?\tPrint this message.";

namespace
{
	string getip(const char* ip)
	{
		string sip(ip);
		string::size_type pos = sip.find(":");
		string ret = sip.substr(0, pos);
		return ret;
	}

	int getport(const char* ip)
	{
		string sip(ip);
		string::size_type pos = sip.find(":");
		pos++;
		string port = sip.substr(pos, sip.size() - pos);
		int ret = atoi(port.c_str());
		return ret;
	}
}

namespace ThetaStream
{
	class CommandLineParser::Impl
	{
	public:
		Impl() {}

	public:
		int destinationPort{ 50000 };
		int ttl{ 16 };
		int rateAdjustment{ 0 };
		std::string sourceFile{ "-" };
		std::string destinationIP{ "127.0.0.1" };
		std::string ifaceAddr;
	};
}

ThetaStream::CommandLineParser::CommandLineParser()
{
	_pimpl = std::make_unique<ThetaStream::CommandLineParser::Impl>();
}

ThetaStream::CommandLineParser::~CommandLineParser()
{
}

ThetaStream::CommandLineParser::CommandLineParser(const CommandLineParser& other)
{
	_pimpl = std::make_unique<ThetaStream::CommandLineParser::Impl>();
	_pimpl->destinationPort = other.destinationPort();
	_pimpl->ttl = other.ttl();
	_pimpl->rateAdjustment = other.rate();
	_pimpl->sourceFile = other.sourceFile();
	_pimpl->destinationIP = other.destinationIp();
	_pimpl->ifaceAddr = other.interfaceAddress();
}

ThetaStream::CommandLineParser& ThetaStream::CommandLineParser::operator=(const ThetaStream::CommandLineParser& rhs)
{
	ThetaStream::CommandLineParser temp(rhs);
	swap(temp);
	return *this;
}

void ThetaStream::CommandLineParser::parse(int argc, char** argv, const char* appname)
{
	string dest;
	char c{};

	while (--argc > 0 && (*++argv)[0] == '-')
	{
		c = *++argv[0];
		switch (c)
		{
		case 's':
			_pimpl->sourceFile = *argv + 1;
			break;
		case 'd':
			dest = *argv + 1;
			break;
		case 'i':
			_pimpl->ifaceAddr = *argv + 1;
			break;
		case 't':
			_pimpl->ttl = std::stoi(*argv + 1);
			break;
		case 'r':
			_pimpl->rateAdjustment = std::stoi(*argv + 1);
			break;
		case '?':
		{
			std::stringstream msg;
			msg << usage << endl;
			msg << endl << "Options: " << endl;
			msg << opts << endl;
			std::exception exp(msg.str().c_str());
			throw exp;
		}
		default:
		{
			std::stringstream msg;
			msg << appname << ": illegal option " << c << endl;
			std::exception exp(msg.str().c_str());
			throw exp;
		}
		}
	}

	_pimpl->destinationIP = getip(dest.c_str());
	_pimpl->destinationPort = getport(dest.c_str());
}

const char* ThetaStream::CommandLineParser::sourceFile() const
{
	return _pimpl->sourceFile.c_str();
}

const char* ThetaStream::CommandLineParser::destinationIp() const
{
	return _pimpl->destinationIP.c_str();
}

const char* ThetaStream::CommandLineParser::interfaceAddress() const
{
	return _pimpl->ifaceAddr.c_str();
}

int ThetaStream::CommandLineParser::destinationPort() const
{
	return _pimpl->destinationPort;
}

int ThetaStream::CommandLineParser::ttl() const
{
	return _pimpl->ttl;
}

int ThetaStream::CommandLineParser::rate() const
{
	return _pimpl->rateAdjustment;
}

void ThetaStream::CommandLineParser::swap(ThetaStream::CommandLineParser& other)
{
	std::swap(_pimpl->destinationPort, other._pimpl->destinationPort);
	std::swap(_pimpl->ttl, other._pimpl->ttl);
	std::swap(_pimpl->rateAdjustment, other._pimpl->rateAdjustment);
	_pimpl->destinationIP.swap(other._pimpl->destinationIP);
	_pimpl->sourceFile.swap(other._pimpl->sourceFile);
	_pimpl->ifaceAddr.swap(other._pimpl->ifaceAddr);
}
