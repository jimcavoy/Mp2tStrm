#include <Mp2tStrm/CommandLineParser.h>

#include <iostream>
#include <string>
#include <sstream>
#include <cstring>
#include <boost/program_options.hpp>
#include <boost/url.hpp>

using namespace std;

namespace ThetaStream
{
    class CommandLineParser::Impl
    {
    public:
        Impl() {}
        Impl(const Impl& other)
            :destinationPort(other.destinationPort)
            , ttl(other.ttl)
            , sourceFile(other.sourceFile)
            , destinationIP(other.destinationIP)
            , ifaceAddr(other.ifaceAddr)
            , framesPerSecond(other.framesPerSecond)
            , probe(other.probe)
            , numTsPackets(other.numTsPackets)
            , startPosition(other.startPosition)
        {

        }
        ~Impl() {}

    public:
        int destinationPort{ 50000 };
        int ttl{ 16 };
        std::string sourceFile{ "-" };
        std::string destinationIP{ "127.0.0.1" };
        std::string ifaceAddr;
        double framesPerSecond{};
        bool probe{ false };
        int numTsPackets{ 7 };
        int startPosition{};
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
    :_pimpl(std::make_unique<ThetaStream::CommandLineParser::Impl>(*other._pimpl))
{

}

ThetaStream::CommandLineParser& ThetaStream::CommandLineParser::operator=(const ThetaStream::CommandLineParser& rhs)
{
    if (this != &rhs)
    {
        _pimpl.reset(new ThetaStream::CommandLineParser::Impl(*rhs._pimpl));
    }
    return *this;
}

ThetaStream::CommandLineParser::CommandLineParser(CommandLineParser&& other) noexcept
{
    *this = std::move(other);
}

ThetaStream::CommandLineParser& ThetaStream::CommandLineParser::operator=(CommandLineParser&& rhs) noexcept
{
    if (this != &rhs)
    {
        _pimpl = std::move(rhs._pimpl);
    }
    return *this;
}

bool ThetaStream::CommandLineParser::parse(int argc, char** argv, const char* appname)
{
    using namespace boost;
    namespace po = boost::program_options;
    namespace url = boost::urls;

    try
    {
        string destUrl{ "udp://127.0.0.0.1:50000" };

        po::options_description desc("Allowed options");
        po::positional_options_description pos_desc;
        pos_desc.add("source", 1);

        desc.add_options()
            ("help,?", "Produce help message.")
            ("source", po::value<string>(&_pimpl->sourceFile), "Source MPEG-2 TS file path. (default: - )")
            ("destinationUrl,d", po::value<string>(&destUrl), "Destination URL. (default: udp://127.0.0.1:50000)")
            ("framesPerSecond,f", po::value<double>(&_pimpl->framesPerSecond), "Frames per second. (default: 0)")
            ("numTsPackets,n", po::value<int>(&_pimpl->numTsPackets), "Number of TS packets in an UDP packet. (default: 7)")
            ("startPosition,s", po::value<int>(&_pimpl->startPosition), "The position where to start playing in seconds. (default: 0)")
            ("probe,p", "Probe the source file and exit.")
            ;

        po::command_line_parser parser{ argc, argv };
        parser.options(desc).positional(pos_desc);
        po::parsed_options poptions = parser.run();

        po::variables_map vm;
        po::store(poptions, vm);
        po::notify(vm);

        if (vm.count("help"))
        {
            cerr << desc << endl;
            return false;
        }

        if (vm.count("probe"))
        {
            _pimpl->probe = true;
        }

        if (vm.count("destinationUrl"))
        {
            boost::system::result<url_view> res = url::parse_uri(destUrl);
            url::url_view u = res.value();

            if (u.scheme() != "udp")
            {
                std::cerr << "ERROR: " <<  u.scheme() << " protocol is not implemented. Only \"udp\" is supported.\n";
                return false;
            }

            _pimpl->destinationIP = u.host();
            if (u.has_port())
            {
                _pimpl->destinationPort = (int)u.port_number();
            }

            if (u.has_query())
            {
                url::params_encoded_view params_ref = u.encoded_params();

                for (const auto& v : params_ref)
                {
                    url::decode_view dk(v.key);
                    url::decode_view dv(v.value);

                    if (dk == "ttl")
                    {
                        string strVal(dv.begin(), dv.end());
                        _pimpl->ttl = std::stoi(strVal);
                    }
                    else if (dk == "localaddr")
                    {
                        string strVal(dv.begin(), dv.end());
                        _pimpl->ifaceAddr = strVal;
                    }
                }
            }
        }

        if (vm.count("numTsPackets"))
        {
            if (_pimpl->numTsPackets < 1 || _pimpl->numTsPackets > 7)
            {
                cerr << "ERROR: The number of TS packets >= 1 or <= 7.";
                return false;
            }
        }

        if (vm.count("startPosition"))
        {
            if (_pimpl->startPosition < 0)
            {
                cerr << "ERROR: The option --startPosition < 0.";
                return false;
            }
        }
    }
    catch (const std::exception& ex)
    {
        std::cerr << ex.what() << endl;
        return false;
    }
    return true;
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

double ThetaStream::CommandLineParser::framesPerSecond() const
{
    return _pimpl->framesPerSecond;
}

int ThetaStream::CommandLineParser::numberOfTsPackets() const
{
    return _pimpl->numTsPackets;
}

bool ThetaStream::CommandLineParser::probe() const
{
    return _pimpl->probe;
}

int ThetaStream::CommandLineParser::startPosition() const
{
    return _pimpl->startPosition;
}
