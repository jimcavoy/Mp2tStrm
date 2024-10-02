// This file contains the 'main' function where program execution begins 
// and ends there.


#include <Mp2tStrm/CommandLineParser.h>
#include <Mp2tStrm/Mp2tStreamer.h>

#include <iostream>
#include <cstring>

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

void banner()
{
    std::cerr << "Mp2tStreamer: MPEG-2 TS Streamer Application v1.3.0" << std::endl;
    std::cerr << "Copyright (c) 2024 ThetaStream Consulting, jimcavoy@thetastream.com" << std::endl;
}


int main(int argc, char* argv[])
{
    using namespace std;

    try
    {
        banner();

        ThetaStream::CommandLineParser cmdline;
        bool r = cmdline.parse(argc, argv, "Mp2tStreamer.exe");

        if (!r)
        {
            return -1;
        }

        int ret = 0;

        std::cerr << std::endl << "Enter Ctrl-C to exit" << std::endl << std::endl;
#ifdef _WIN32
        if (!SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, TRUE))
        {
            DWORD err = GetLastError();
            std::cerr << "ERROR: Failed to set console control handler.  Error Code = " << err << std::endl;
            return err;
        }
#endif
        ThetaStream::Mp2tStreamer streamer(cmdline);
        pMp2tStreamer = &streamer;

        if ((cmdline.probe() || strcmp(cmdline.sourceFile(), "-") != 0) && cmdline.framesPerSecond() == 0)
        {
            std::cerr << "Probing input MPEG-2 TS stream..." << std::endl << std::endl;
            streamer.probe();

            std::cout << "Duration: " << streamer.duration() << std::endl;
            std::cout << "Average Bitrate: " << streamer.averageBitrate() << std::endl;
            std::cout << "Metadata Carriage: " << streamer.metadataCarriage() << std::endl;
            std::cout << "Metadata Frequency: " << streamer.metadataFrequency() << std::endl;
            std::cout << "Frame/Seconds: " << streamer.framesPerSecond() << std::endl;
            std::cout << "Resolution: " << streamer.width() << "x" << streamer.height() << std::endl << std::endl;
        }

        if (!cmdline.probe())
        {
            // Confirm the start position < duration
            if (cmdline.startPosition() > streamer.duration())
            {
                std::cerr << "ERROR: Start position, " << cmdline.startPosition() << " seconds, is greater than video duration, " << streamer.duration() << " seconds." << std::endl;
                return -1;
            }

            std::cerr << "Streaming file..." << std::endl << std::endl;
            ret = streamer.run();
            cout << "TS Packets Read: " << streamer.tsPacketsRead() << endl;
            cout << "UDP Packets Sent: " << streamer.udpPacketsSent() << endl;
        }

        return ret;
    }
    catch (const std::exception& exp)
    {
        cerr << exp.what() << endl;
        return -1;
    }
}