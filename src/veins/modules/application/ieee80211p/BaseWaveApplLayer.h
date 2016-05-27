//
// Copyright (C) 2011 David Eckhoff <eckhoff@cs.fau.de>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#ifndef BASEWAVEAPPLLAYER_H_
#define BASEWAVEAPPLLAYER_H_

#include "veins/base/modules/BaseApplLayer.h"
#include "veins/modules/utility/Consts80211p.h"
#include "veins/modules/messages/WaveShortMessage_m.h"
#include "veins/base/connectionManager/ChannelAccess.h"
#include "veins/modules/mac/ieee80211p/WaveAppToMac1609_4Interface.h"

// Add for Epidemic
#include <stdio.h>

#include <map>
#include <queue>
#include <vector>
#include <fstream>
#include <unordered_map>
#include <algorithm> // std::find

using namespace std;

//Define a constant to support send beacons or send messages in a broadcast fashion
#ifndef BROADCAST
#define BROADCAST 268435455
#endif
//

#ifndef DBG
#define DBG EV
#endif
//#define DBG std::cerr << "[" << simTime().raw() << "] " << getParentModule()->getFullPath() << " "

/**
 * @brief
 * WAVE application layer base class.
 *
 * @author David Eckhoff
 *
 * @ingroup applLayer
 *
 * @see BaseWaveApplLayer
 * @see Mac1609_4
 * @see PhyLayer80211p
 * @see Decider80211p
 */
class BaseWaveApplLayer : public BaseApplLayer {

    public:
        ~BaseWaveApplLayer();
        virtual void initialize_default_veins_TraCI(int stage);
        virtual void initialize_minicurso_UFPI_TraCI(int stage);
        virtual void initialize_epidemic(int stage);
        virtual void finish();

        virtual  void receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj, cObject* details);

        enum WaveApplMessageKinds {
            SERVICE_PROVIDER = LAST_BASE_APPL_MESSAGE_KIND,
            SEND_BEACON_EVT,SEND_BEACON_EVT_minicurso, SERVICE_EXPIRED_EVT, SERVICE_QUERY_EVT, SERVICE_EVT, MOBILITY_EVT, //modificado osdp, add SERVICE_EXPIRED_EVT, SERVICE_QUERY_EVT, SERVICE_EVT, anter era apenas SEND_BEACON_EVT, para o  service_discovery foi add MOBILITY_EVT
            SEND_BEACON_EVT_epidemic,
            SEND_BEACON_EVT_mfcv_epidemic,
            Send_EpidemicMessageRequestEvt
        };


    protected:

        static const simsignalwrap_t mobilityStateChangedSignal;

        /** @brief handle messages from below */
        virtual void handleLowerMsg(cMessage* msg);
        /** @brief handle self messages */
        virtual void handleSelfMsg(cMessage* msg);

        //virtual WaveShortMessage* prepareWSM(std::string name, int dataLengthBits, t_channel channel, int priority, int rcvId, int serial=0);
        virtual WaveShortMessage* prepareWSM(std::string name, int dataLengthBits, t_channel channel, int priority, unsigned int rcvId, int serial=0);

//######################################### vehDist #########################################
        void saveMessagesOnFile(WaveShortMessage* wsm, string fileName);
        void printHeaderfileExecution(double ttlBeaconMessage, unsigned short int countGenerateBeaconMessage);
        void openFileAndClose(string fileName, bool justForAppend, double ttlBeaconMessage, unsigned short int countGenerateBeaconMessage);
        void generalInitializeVariables_executionByExpNumber();
        string getFolderResult(unsigned short int experimentSendbyDSR);

        //## Used to another projects
        void messagesReceivedMeasuringRSU(WaveShortMessage* wsm);
        void printCountMessagesReceived();
        void toFinishRSU();
//######################################### vehDist #########################################

//######################################### Epidemic #########################################
        virtual WaveShortMessage* prepareWSM_epidemic(std::string name, int dataLengthBits, t_channel channel, int priority, unsigned int rcvId, int serial=0);
        unsigned int MACToInteger();

        void receivedOnBeacon(WaveShortMessage* wsm);
        void receivedOnData(WaveShortMessage* wsm);

        void printWaveShortMessage(WaveShortMessage* wsm);

        void printQueueFIFO(std::queue <string> qFIFO);
        void sendLocalSummaryVector(unsigned int newRecipientAddress);

        //Return a string with the whole summaryVector
        string getLocalSummaryVectorData();
        string getEpidemicRequestMessageVectorData();

        void printNodesIRecentlySentSummaryVector();

        void printEpidemicLocalMessageBuffer();
        void printEpidemicRequestMessageVector();
        void printEpidemicLocalSummaryVectorData();
        void printEpidemicRemoteSummaryVectorData();

        void sendEpidemicRequestMessageVector(unsigned int newRecipientAddress);
        void sendMessagesRequested(string s, unsigned int recipientAddress);

        void createEpidemicRequestMessageVector();
        void createEpidemicRemoteSummaryVector(string s);
//######################################### Epidemic #########################################


        virtual void sendWSM(WaveShortMessage* wsm);
        virtual void onBeacon(WaveShortMessage* wsm) = 0;
        virtual void onData(WaveShortMessage* wsm) = 0;

        virtual void handlePositionUpdate(cObject* obj);

    protected:
        int beaconLengthBits;
        int beaconPriority;
        bool sendData;
        bool sendBeacons;
        simtime_t individualOffset;
        int dataLengthBits;
        bool dataOnSch;
        int dataPriority;
        Coord curPosition;
        int mySCH;
        int myId;

        string source;
        string target;

        cMessage* sendBeaconEvt;

        WaveAppToMac1609_4Interface* myMac;

//######################################### vehDist #########################################
        ofstream myfile; // record in file

        unsigned short int target_x;
        unsigned short int target_y;

        string fileMessagesUnicast;
        string fileMessagesBroadcast;
        string fileMessagesCount;
        string fileMessagesDrop;
        string fileMessagesGenerated;

        unsigned short int repeatNumber;
        unsigned short int expNumber;
        unsigned short int expSendbyDSCR;
        double ttlBeaconMessage;
        unsigned short int countGenerateBeaconMessage;

        //## Used to another projects
        unsigned short int beaconMessageHopLimit;

        struct messages {
          unsigned short int copyMessage;
          string hops;
          unsigned short int minHop;
          unsigned short int maxHop;
          unsigned short int sumHops;
          unsigned short int countT;
          unsigned short int countP;
          string wsmData;
          simtime_t sumTimeRecived;
          string times;
        };
        map <string, struct messages> messagesReceived;
//######################################### vehDist #########################################

//######################################### Epidemic #########################################
        //Hash table to represent the node buffer. It stores messages generated by itself as well messages carried on behalf of other nodes
        //Creating a unordered_map to represent the local epidemic messages buffer
        unordered_map <string, WaveShortMessage> epidemicLocalMessageBuffer;
        //Bit Vector to represent a summary vector that indicates which entries in their local hash table are set.
        unordered_map <string, bool> epidemicLocalSummaryVector;
        //Bit Vector to represent a summary vector that indicates which entries in remote hash table are set.
        unordered_map <string, bool> epidemicRemoteSummaryVector;
        //Bit Vector to represent a result summary vector that will be used to make a request of messages.
        unordered_map <string, bool> epidemicRequestMessageVector;
        //Cache with nodes that I recently met
        unordered_map <unsigned int, simtime_t> nodesRecentlyFoundList;
        //Cache with nodes that I recently sent the summary vector
        unordered_map <unsigned int, simtime_t> nodesIRecentlySentSummaryVector;
        //implementation of FIFO in order to maintain the limited length of the buffer activated
        queue <string> queueFIFO;

        int sendSummaryVectorInterval;
        unsigned int maximumEpidemicBufferSize;
        //unsigned int hopCount;


        cMessage* sendEpidemicMessageRequestEvt;
        unordered_map <string, WaveShortMessage> epidemicMessageSend;
//######################################### Epidemic #########################################
};

#endif /* BASEWAVEAPPLLAYER_H_ */
