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

#include <map>
#include <BaseApplLayer.h>
#include <Consts80211p.h>
#include <WaveShortMessage_m.h>
#include "base/connectionManager/ChannelAccess.h"
#include <WaveAppToMac1609_4Interface.h>

// Add for Epidemic
#include <stdio.h>

#include <fstream>
#include <map>
#include <unordered_map>
#include <algorithm>    // std::find
#include <vector>

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
        virtual void initialize_mfcv_epidemic(int stage);
        virtual void finish();

        virtual  void receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj);

        enum WaveApplMessageKinds {
            SERVICE_PROVIDER = LAST_BASE_APPL_MESSAGE_KIND,
            SEND_BEACON_EVT,SEND_BEACON_EVT_minicurso, SERVICE_EXPIRED_EVT, SERVICE_QUERY_EVT, SERVICE_EVT, MOBILITY_EVT, //modificado osdp, add SERVICE_EXPIRED_EVT, SERVICE_QUERY_EVT, SERVICE_EVT, anter era apenas SEND_BEACON_EVT, para o  service_discovery foi add MOBILITY_EVT
            SEND_BEACON_EVT_epidemic,
            SEND_BEACON_EVT_mfcv_epidemic
        };

//######################################### vehDist #########################################
        void saveMessagesOnFile(WaveShortMessage* wsm, string fileName);
        void printHeaderfileExecution(int ttlBeaconMessage, int countGenerateBeaconMessage);
        void openFileAndClose(string fileName, bool justForAppend, int ttlBeaconMessage, int countGenerateBeaconMessage);
        void generalInitializeVariables_executionByExperimentNumber();
//######################################### vehDist #########################################

    protected:

        static const simsignalwrap_t mobilityStateChangedSignal;

        /** @brief handle messages from below */
        virtual void handleLowerMsg(cMessage* msg);
        /** @brief handle self messages */
        virtual void handleSelfMsg(cMessage* msg);

        //virtual WaveShortMessage* prepareWSM(std::string name, int dataLengthBits, t_channel channel, int priority, int rcvId, int serial=0);
        virtual WaveShortMessage* prepareWSM(std::string name, int dataLengthBits, t_channel channel, int priority, unsigned int rcvId, int serial=0);

        //Add for Epidemic
        virtual WaveShortMessage* prepareWSM_epidemic(string name, int dataLengthBits, t_channel channel, int priority, unsigned int rcvId, int serial=0);
        virtual unsigned int MACToInteger();

        virtual void sendWSM(WaveShortMessage* wsm);
        virtual void onBeacon(WaveShortMessage* wsm) = 0;
        virtual void onData(WaveShortMessage* wsm) = 0;

        virtual void handlePositionUpdate(cObject* obj);

        // test Jonh
        static unsigned short int vehCount;
        static unsigned short int rsuCount;

        // record in file
        std::ofstream myfile;

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

        cMessage* sendBeaconEvt;

        WaveAppToMac1609_4Interface* myMac;

        // Add for Epidemic
        int sendSummaryVectorInterval;
        unsigned int maximumEpidemicBufferSize;
        unsigned int hopCount;

        unsigned int maximumMfcvEpidemicBufferSize;

        //To record statistics collection
        //cLongHistogram hopCountStats;
        //cOutVector hopCountVector;
        //cDoubleHistogram messageArrivalTimeStats;
        //cOutVector messageArrivalTimeVector;
        //long unsigned int numMessageReceived;

        //simsignal_t hopsToDeliverSignal;
        //simsignal_t delayToDeliverSignal;
        //simsignal_t messageArrivalSignal;

        string source;
        string target;
        int target_x;
        int target_y;

        string fileMessagesUnicast;
        string fileMessagesBroadcast;
        string fileMessagesCount;
        string fileMessagesDrop;
        string fileMessagesGenerated;

        int repeatNumber;
        int experimentNumber;
        unsigned int beaconMessageHopLimit;
        int countGenerateBeaconMessage;
        int ttlBeaconMessage;
        double timeLimitGenerateBeaconMessage;
        string stringTmp;
        double doubleTmp;
};
#endif /* BASEWAVEAPPLLAYER_H_ */
