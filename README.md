# Mp2tStrm
For users that have MPEG-2 TS files (*.ts)
who need to restream them over an IP network.
The MPEG-2 TS File Streamer __Mp2tStreamer__ is a command line tool that
will allow clients to retransmit a MPEG-2 TS stream from a file onto an
IP network.  Unlike Colasoft Packet Player that retransmit video from
a Wireshark capture file, this tool will allow users to replay a video
stream from a MPEG-2 TS file.

## How To Build
This project has an external dependency on:

 - [mp2t library](https://github.com/jimcavoy/mp2tp) build and install the library.
 - [h264p library](https://github.com/jimcavoy/h264p) build and install the library.

`Mp2tStrm` is a CMake project.  To configure and build the project use the following commands:

```
cmake --preset=<windows-base|linux-base>
```
```
cmake --build ./build
```
```
cmake --install ./build
```
### To Test
The project has a test case.  Run the following command:

    ctest --test-dir ./build -C <Debug|Release>

The test case duration is about 67 seconds to complete.

## Usage
Usage: __Mp2tStreamer__ \<Source MPEG-2 TS File> OPTIONS

```
Allowed options:
  -? [ --help ]                Produce help message.
  --source arg                 Source MPEG-2 TS file path. (default: - )
  -d [ --destinationUrl ] arg  Destination URL. (default:
                               udp://127.0.0.1:50000)
  -f [ --framesPerSecond ] arg Frames per second. (default: 0)
  -n [ --numTsPackets ] arg    Number of TS packets in an UDP packet. (default:
                               7)
  -s [ --startPosition ] arg   The position where to start playing in seconds.
                               (default: 0)
  -p [ --probe ]               Probe the source file and exit.
```
The `--destinationUrl` has an optional query component with the following attribute-value pairs:

- __ttl__. The time-to-live parameter.
- __localaddr__. Transmit on a network adapter with an assigned IP address.

For example, you want to stream using UDP on a multicast address of 239.3.1.11 with a time-to-live of 16 and transmit
onto a network adapter with an assigned IP address of 192.168.0.24:

```
--destinationUrl=udp://239.3.1.11:50000?ttl=16&localaddr=192.168.0.24
```

__Note__: Presently, __Mp2tStreamer__ only supports UDP protocol.
### Examples

#### 1. Stream a file
```
Mp2tStreamer.exe C:\Samples\somefile.ts -d udp://239.3.1.11:50000
```

#### 2. Pipe a file into Mp2tStrm application to stream
```
Mp2tStreamer.exe -d udp://239.3.1.11:50000?ttl=255&localaddr=192.168.0.24 -f 29.97 < C:\Samples\somefile.ts
```

Ensure the `-f|--framesPerSecond` option is set greater than 0.

#### 3. Pipe Motion Imagery stream from another application
```
SampleApp.exe | Mp2tStreamer.exe --destinationUrl=udp://239.3.1.11:50000 --framesPerSecond=30
```

SampleApp.exe is an application that streams Motion Imagery data out to 
console and is piped into Mp2tStreamer.exe.  Ensure the `-f|--framesPerSecond` option is set greater than 0.

#### 4. Probe a file and exit
```
Mp2tStreamer.exe C:\Samples\somefile.ts --probe
```

#### 5. Start streaming at seconds from the start
```
Mp2tStreamer.exe --destinationUrl=udp://239.3.1.11:50000 --startPosition=120
```

Start streaming at `--startPosition=120` seconds from the start of the video.

### Runtime Commands
When __Mp2tStreamer__ is playing, you can enter the following on the command line:

- __Pause__.  Type <kbd>p</kbd> + <kbd>⏎ Enter</kbd> to pause playing.
- __Start__.  After the playing is paused, type <kbd>s</kbd> + <kbd>⏎ Enter</kbd> to continue playing.
- __Quit__.  Type <kbd>q</kbd> + <kbd>⏎ Enter</kbd> to quit playing and exit.