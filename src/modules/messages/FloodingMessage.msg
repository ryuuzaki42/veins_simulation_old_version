cplusplus {{
    #include <list>
	#include "WaveShortMessage_m.h"
}}

class WaveShortMessage;
struct Coord;

message FloodingMessage extends WaveShortMessage
{
    unsigned int srcAddr;
    unsigned int msgId;
    // TODO
}