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

 - [mp2t library](https://github.com/jimcavoy/mp2tp) and;
 - [h264p library](https://github.com/jimcavoy/h264p).

Build and install these projects before building the application.
