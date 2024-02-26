# Mp2tStrm
For users that have MPEG-2 TS files (*.ts)
who need to restream them over an IP network.
The MPEG-2 TS File Streamer `Mp2tStrmApp` is a command line tool that
will allow clients to retransmit a MPEG-2 TS stream from a file onto an
IP network.  Unlike Colasoft Packet Player that retransmit video from
a Wireshark capture file, this tool will allow users to replay a video
stream from a MPEG-2 TS file.

## How To Build
This project has an external dependency on:

 - [loki library](https://github.com/snaewe/loki-lib.git) clone in the same directory where this project was cloned.
 - [mp2t library](https://github.com/jimcavoy/mp2tp) build and install the library.
 - [h264p library](https://github.com/jimcavoy/h264p) build and install the library.

`Mp2tStrm` is a CMake project.  To configure and build the project use the following commands:

 - cmake -S . -B ./build
 - cmake --build ./build
 - cmake --install ./build

## Usage
Usage: Mp2tStrmApp.exe [-?] [-s|-] [-d127.0.0.1:50000] [-t[0..255]] [-iSTRING]

Usage: Mp2tStrm.exe [OPTION...] 

  `-s-`                           The source MPEG-2 TS file path (default: "-")
  
  `-d127.0.0.1:50000`             The destination socket address (ip:port) 
                                (default: "127.0.0.1:50000")
                                
  `-t[0..255]`                    Time to Live. (default: 16)
  
  `-iSTRING`                      Specifies the network interface IP
                                address for the destination stream. 

Help options:

  `-?`                            Show this help message

### Examples

1. Stream a file

	> Mp2tStrmApp.exe -sC:\Samples\somefile.ts -d239.3.1.11:50000
	
2. Pipe a file into Mp2tStrm application to stream

	> Mp2tStrmApp.exe -d239.3.1.11:50000 < C:\Samples\somefile.ts

3. Pipe Motion Imagery stream from another application

	> SampleApp.exe | Mp2tStrmApp.exe -d239.3.1.11:50000
	
SampleApp.exe is an application that streams Motion Imagery data out to 
console and is piped into Mp2tStrmApp.exe.
