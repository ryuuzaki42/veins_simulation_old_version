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
            SEND_BEACON_EVT, SEND_BEACON_EVT_minicurso,
            SERVICE_EXPIRED_EVT, SERVICE_QUERY_EVT, SERVICE_EVT, // Modificado OSDP, add SERVICE_EXPIRED_EVT, SERVICE_QUERY_EVT, SERVICE_EVT, antes era apenas SEND_BEACON_EVT
            SEND_BEACON_EVT_epidemic, Send_EpidemicMessageRequestEvt
        };

    protected:
        static const simsignalwrap_t mobilityStateChangedSignal;

        virtual void handleLowerMsg(cMessage* msg); /** @brief handle messages from below */

        virtual void handleSelfMsg(cMessage* msg); /** @brief handle self messages */

        //virtual WaveShortMessage* prepareWSM(string name, int dataLengthBits, t_channel channel, int priority, int rcvId, int serial=0);
        virtual WaveShortMessage* prepareWSM(string name, int dataLengthBits, t_channel channel, int priority, unsigned int rcvId, int serial=0);

//######################################### vehDist #########################################
        void generalInitializeVariables_executionByExpNumberVehDist();
        string getFolderResultVehDist(unsigned short int experimentSendbyDSR);

        //## Used to another projects
        void toFinishRSU();
        string boolToString(bool value);
        void restartFilesResultRSU(string folderResult);
        void printCountMessagesReceivedRSU();
        void messagesReceivedMeasuringRSU(WaveShortMessage* wsm);

        void saveMessagesOnFile(WaveShortMessage* wsm, string fileName);
        void printHeaderfileExecution(double ttlBeaconMessage, unsigned short int countGenerateBeaconMessage);
        void openFileAndClose(string fileName, bool justForAppend, double ttlBeaconMessage, unsigned short int countGenerateBeaconMessage);
//######################################### vehDist #########################################

//######################################### Epidemic #########################################
        virtual WaveShortMessage* prepareWSM_epidemic(string name, int dataLengthBits, t_channel channel, int priority, unsigned int rcvId, int serial=0);
        unsigned int MACToInteger();

        void receivedOnBeacon(WaveShortMessage* wsm);
        void receivedOnData(WaveShortMessage* wsm);

        void printWaveShortMessageEpidemic(WaveShortMessage* wsm);

        void printQueueFIFO(std::queue <string> qFIFO);
        void sendLocalSummaryVector(unsigned int newRecipientAddress);

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
        int beaconLengthBits, beaconPriority, mySCH, myId, dataLengthBits, dataPriority;
        bool sendData, sendBeacons, dataOnSch;
        simtime_t individualOffset;
        Coord curPosition;

        string source, target;

        cMessage* sendBeaconEvt;

        WaveAppToMac1609_4Interface* myMac;

//######################################### vehDist #########################################
        ofstream myfile; // record in file

        unsigned short int target_x, target_y;

        string fileMessagesUnicast, fileMessagesBroadcast, fileMessagesCount, fileMessagesDrop, fileMessagesGenerated;

        unsigned short int repeatNumber, expNumber, expSendbyDSCR, countGenerateBeaconMessage, ttlBeaconMessage;

        //## Used to another projects
        unsigned short int beaconMessageHopLimit;

        struct messages {
          string firstSource, hops, wsmData, times;
          unsigned short int minHop, maxHop, sumHops, countT, countP, copyMessage;
          simtime_t sumTimeRecived;
        };
        map <string, struct messages> messagesReceived;
//######################################### vehDist #########################################

//######################################### Epidemic #########################################
        unordered_map <string, WaveShortMessage> epidemicLocalMessageBuffer, epidemicMessageSend;

        unordered_map <string, bool> epidemicLocalSummaryVector, epidemicRemoteSummaryVector, epidemicRequestMessageVector;

        unordered_map <unsigned int, simtime_t> nodesRecentlyFoundList, nodesIRecentlySentSummaryVector;

        // Implementation of FIFO in order to maintain the limited length of the buffer activated
        queue <string> queueFIFO;

        unsigned int sendSummaryVectorInterval, maximumEpidemicBufferSize;
        unsigned int nodesRecentlySendLocalSummaryVector = 0;
        simtime_t lastTimeSendLocalSummaryVector = 0;

        cMessage* sendEpidemicMessageRequestEvt;
//######################################### Epidemic #########################################
};

#endif /* BASEWAVEAPPLLAYER_H_ */
