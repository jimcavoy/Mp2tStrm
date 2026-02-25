// This file contains the 'main' function where program execution begins 
// and ends there.


#include <Mp2tStrm/CommandLineParser.h>
#include <Mp2tStrm/Mp2tStreamer.h>

#include <iostream>
#include <cstring>
#include <thread>

ThetaStream::Mp2tStreamer* pMp2tStreamer;
bool run = true;

#ifdef _WIN32
#include <Windows.h>
#include <conio.h>

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
#else
#include <unistd.h>
#include <termios.h>

void RestoreKeyboardBlocking(struct termios* initial_settings)
{
    tcsetattr(0, TCSANOW, initial_settings);
}

void SetKeyboardNonBlock(struct termios* initial_settings)
{
    struct termios new_settings;
    tcgetattr(0, initial_settings);

    new_settings = *initial_settings;
    new_settings.c_lflag &= ~ICANON;
    new_settings.c_lflag &= ~ECHO;
    new_settings.c_lflag &= ~ISIG;
    new_settings.c_cc[VMIN] = 0;
    new_settings.c_cc[VTIME] = 0;

    tcsetattr(0, TCSANOW, &new_settings);
}

char getcharAlt()
{
    char buff[2];
    int l = read(STDIN_FILENO, buff, 1);
    if (l > 0) return buff[0];
    return (EOF);
}

#endif

void banner()
{
    std::cerr << "Mp2tStreamer: MPEG-2 TS Streamer Application v1.4.4" << std::endl;
    std::cerr << "Copyright (c) 2026 ThetaStream Consulting, jimcavoy@thetastream.com" << std::endl;
}

class InputHandler
{
public:
    void operator()()
    {
        char c{};
        while (run)
        {
#ifdef _WIN32
            if (_kbhit())
            {
                c = _getch();
            }
#else
            c = getcharAlt();
#endif
            switch (c)
            {
            case 'p': std::cerr << "Paused" << std::endl; pMp2tStreamer->pause(); break;
            case 's': std::cerr << "Start" << std::endl; pMp2tStreamer->start(); break;
            case 'q': 
            {
                std::cerr << "Quit" << std::endl; pMp2tStreamer->stop();
                run = false;
                break;
            }
            }
            c = 0;
#ifdef _WIN32
            Sleep(500);
#else
            sleep(1);
#endif
        }
    }
};


int main(int argc, char* argv[])
{
    using namespace std;

    try
    {
        banner();

        ThetaStream::CommandLineParser cmdline;
        bool result = cmdline.parse(argc, argv, "Mp2tStreamer.exe");

        if (!result)
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

            std::cout << "Duration: " << streamer.duration() << " seconds" << std::endl;
            std::cout << "Average Bitrate: " << streamer.averageBitrate() << " kilobits/second" << std::endl;
            std::cout << "Metadata Carriage: " << streamer.metadataCarriage() << std::endl;
            std::cout << "Metadata Frequency: " << streamer.metadataFrequency() << " Hertz" << std::endl;
            std::cout << "Framerate: " << streamer.framesPerSecond() << " frames/second" << std::endl;
            std::cout << "Resolution: " << streamer.width() << "x" << streamer.height() << " pixels" << std::endl << std::endl;
        }

        if (!cmdline.probe())
        {
            // Confirm the start position < duration
            if (cmdline.startPosition() > streamer.duration())
            {
                std::cerr << "ERROR: Start position, " << cmdline.startPosition() << " seconds, is greater than video duration, " << streamer.duration() << " seconds." << std::endl;
                return -1;
            }

#ifndef _WIN32
            // Utilize an non-blocking getchar()
            struct termios term_settings;
            SetKeyboardNonBlock(&term_settings);
#endif
            InputHandler handler;
            std::thread inputThread{ &InputHandler::operator(), &handler };

            std::cerr << "Streaming file..." << std::endl << std::endl;
            ret = streamer.run();
            cout << "TS Packets Read: " << streamer.tsPacketsRead() << endl;
            cout << "UDP Packets Sent: " << streamer.udpPacketsSent() << endl;

            run = false;

            inputThread.detach();

#ifndef _WIN32
            RestoreKeyboardBlocking(&term_settings);
#endif
        }

        return ret;
    }
    catch (const std::exception& exp)
    {
        cerr << "*** ERROR: " << exp.what() << " ***" << endl;
        return -1;
    }
}