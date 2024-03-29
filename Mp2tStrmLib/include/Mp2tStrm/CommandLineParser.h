#pragma once

#include <memory>

namespace ThetaStream
{
	class CommandLineParser
	{
	public:
		CommandLineParser();
		~CommandLineParser();

		CommandLineParser(const CommandLineParser& other);
		CommandLineParser& operator=(const CommandLineParser& rhs);

		CommandLineParser(CommandLineParser&& other) noexcept;
		CommandLineParser& operator=(CommandLineParser&& rhs) noexcept;

		void parse(int argc, char** argv, const char* appname);

		const char* sourceFile() const;
		const char* destinationIp() const;
		const char* interfaceAddress() const;
		int destinationPort() const;
		int ttl() const;
		double framesPerSecond() const;
		bool probe() const;

	private:
		class Impl;
		std::unique_ptr<Impl> _pimpl;
	};

}
