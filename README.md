# Mp2tStrm
For users that have MPEG-2 TS files (*.ts)
who need to restream them over an IP network.
The MPEG-2 TS File Streamer `Mp2tStreamer` is a command line tool that
will allow clients to retransmit a MPEG-2 TS stream from a file onto an
IP network.  Unlike Colasoft Packet Player that retransmit video from
a Wireshark capture file, this tool will allow users to replay a video
stream from a MPEG-2 TS file.

## How To Build
This project has an external dependency on:

 - [mp2t library](https://github.com/jimcavoy/mp2tp) build and install the library.
 - [h264p library](https://github.com/jimcavoy/h264p) build and install the library.

`Mp2tStrm` is a CMake project.  To configure and build the project use the following commands:

    cmake -S . -B ./build

<p></p>

    cmake --build ./build
    
<p></p>

    cmake --install ./build

### To Test
The project has a test case.  Run the following command:

    ctest --test-dir ./build

The test case duration is about 13 seconds.

## Usage
Usage: __Mp2tStreamer__ [-?] [-p] [-s|-] [-d 127.0.0.1:50000] [-t [0..255]] [-i STRING] [-f DOUBLE]

Options:

  `-s-`                           The source MPEG-2 TS file path (default: "-")

  `-d 127.0.0.1:50000`             The destination socket address (ip:port) (default: "127.0.0.1:50000")

  `-t [0..255]`                    Time to Live. (default: 16)

  `-i STRING`                      Specifies the network interface IP address for the destination stream. 

  `-f DOUBLE`                    Frames per second. (default: 0)

  `-p`                          Probe the input stream and exit.

Help options:

  `-?`                            Show this help message

### Examples

__Stream a file.__

    Mp2tStreamer.exe -s C:\Samples\somefile.ts -d 239.3.1.11:50000

__Pipe a file into Mp2tStrm application to stream.  Ensure the `-f` parameter is set greater than 0.__

    Mp2tStreamer.exe -d 239.3.1.11:50000 -f 29.97 < C:\Samples\somefile.ts

__Pipe Motion Imagery stream from another application.  Ensure the `-f` parameter is set greater than 0.__

    SampleApp.exe | Mp2tStreamer.exe -d239.3.1.11:50000 -f 30

SampleApp.exe is an application that streams Motion Imagery data out to 
console and is piped into Mp2tStreamer.exe.

__Probe a file and exit.__

    Mp2tStreamer.exe -s C:\Samples\somefile.ts -p
