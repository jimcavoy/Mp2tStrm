// This file contains the 'main' function where program execution begins 
// and ends there.

#include "CommandLineParser.h"
#include "Mp2tStreamer.h"

#include <iostream>

ThetaStream::Mp2tStreamer* pMp2tStreamer;

#ifdef _WIN32
#include <Windows.h>

BOOL CtrlHandler(DWORD fdwCtrlType)
{
	switch (fdwCtrlType)
	{
		// Handle the CTRL-C signal.
	case CTRL_C_EVENT:
	case CTRL_CLOSE_EVENT:
	case CTRL_BREAK_EVENT:
	case CTRL_SHUTDOWN_EVENT:
	case CTRL_LOGOFF_EVENT:
		pMp2tStreamer->stop();
		std::cerr << "Closing down, please wait..." << std::endl;
		return TRUE;
	default:
		return FALSE;
	}
}
#endif


int main(int argc, char *argv[])
{
	using namespace std;

	try
	{
		ThetaStream::CommandLineParser cmdline;
		cmdline.parse(argc, argv, "Mp2tStrm.exe");

		std::cerr << std::endl << "Enter Ctrl-C to exit" << std::endl << std::endl;
#ifdef _WIN32
		if (!SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, TRUE))
		{
			DWORD err = GetLastError();
			std::cerr << "Failed to set console control handler.  Error Code = " << err << std::endl;
			return err;
		}
#endif
		clock_t start, end;

		ThetaStream::Mp2tStreamer streamer(cmdline);
		pMp2tStreamer = &streamer;

		start = clock();
		int ret = streamer.run();
		end = clock();

		double dif = (double)(end - start) / CLOCKS_PER_SEC;
		fprintf_s(stderr, "\nElapsed time is %2.3lf seconds.\n\n", dif);

		cout << "TS Packets Read: " << streamer.tsPacketsRead() << endl;
		cout << "UDP Packets Sent: " << streamer.udpPacketsSent() << endl;

		return ret;
	}
	catch (const std::exception& exp)
	{
		cerr << exp.what() << endl;
		return -1;
	}
}