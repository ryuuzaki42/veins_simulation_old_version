//
// Copyright (C) 2015-2016 João Batista <joao.b@usp.br>
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

#ifndef vehDist_H
#define vehDist_H

#include "veins/modules/application/ieee80211p/BaseWaveApplLayer.h"
#include "veins/modules/mobility/traci/TraCIMobility.h"
#include "veins/modules/mobility/traci/TraCICommandInterface.h"

using Veins::TraCIMobility;
using Veins::TraCICommandInterface;
using Veins::AnnotationManager;

class vehDist : public BaseWaveApplLayer {
    public:
        virtual void initialize(int stage);

        enum WaveApplMessageKinds {
            SendEvtBeaconMessage, SendEvtGenerateBeaconMessage, SendEvtUpdateRateTimeToSendVeh
        };

    protected:
        TraCIMobility* mobility;
        TraCICommandInterface* traci;
        TraCICommandInterface::Vehicle* traciVehicle;

        cMessage* sendBeaconMessageEvt;
        cMessage* sendGenerateBeaconMessageEvt;
        cMessage* sendUpdateRateTimeToSendVeh;

        static double ttlBeaconStatus;
        static unsigned short int numVehToRandom;
        static unsigned short int timeLimitGenerateBeaconMessage;
        static unsigned short int msgDroppedbyTTL;
        static unsigned short int msgDroppedbyHop;
        static unsigned short int msgDroppedbyBuffer;
        static unsigned short int countMsgPacketSend;
        static unsigned short int msgBufferUseGeneral;
        static unsigned short int timeToUpdatePosition;
        static unsigned short int beaconMessageBufferSize;
        static unsigned short int beaconStatusBufferSize;
        static unsigned short int countMeetN;
        static unsigned short int countTwoCategoryN;
        static unsigned short int countMeetPBigerT;
        static unsigned short int countVehicleAll;

        static vector <string> numVehicles;
        static vector <string> vehGenerateMessage;
        static unsigned short int countMesssageDrop;
        static unsigned short int beaconMessageId;

        vector <string> messagesDelivered;
        vector <string> messagesOrderReceived;
        unordered_map<string, WaveShortMessage> messagesBuffer;
        unordered_map<string, WaveShortMessage> beaconStatusNeighbors;
        unordered_map<string, string> messagesSendLog;

        unsigned short int messageToSend;
        unsigned short int rateTimeToSend;
        unsigned short int msgBufferMaxUse;
        simtime_t timeToFinishLastStartSend;
        unsigned short int rateTimeToSendLimitTime;
        unsigned short int rateTimeToSendUpdateTime;
        unsigned short int rateTimeToSendDistanceControl;

        mt19937 mt_veh;
        double vehOffSet;
        string vehCategory;

        struct messagesDropStruct {
            unsigned short int byBuffer;
            unsigned short int byHop;
            unsigned short int byTime;
        };
        map <string, struct messagesDropStruct> messagesDrop;

        struct shortestDistance {
            Coord senderPos;
            double speedVeh;
            string categoryVeh;
            double distanceToTarget;
            double distanceToTargetCategory;
            double decisionValueDistanceSpeed;
            double decisionValueDistanceRateTimeToSend;
            double decisionValueDistanceSpeedRateTimeToSend;
            unsigned short int rateTimeToSendVeh;
        };

    protected:
        virtual void onBeacon(WaveShortMessage* wsm);
        virtual void onData(WaveShortMessage* wsm);

        void finish();
        void handleSelfMsg(cMessage* msg);
        void handleLowerMsg(cMessage* msg);

        void printBeaconStatusNeighbors();
        void onBeaconStatus(WaveShortMessage* wsm);
        WaveShortMessage* prepareBeaconStatusWSM(std::string name, int lengthBits, t_channel channel, int priority, int serial);

        void vehCreateEventTrySendBeaconMessage();
        void generateTarget();
        void sendBeaconMessage();
        void printMessagesBuffer();
        void generateBeaconMessage();
        void colorCarryMessage();
        WaveShortMessage* updateBeaconMessageWSM(WaveShortMessage* wsm, string rcvId);

        void trySendBeaconMessage();
        string neighborWithShortestDistanceToTarge(string key);

        void selectVehGenerateMessage();
        void vehGenerateBeaconMessageBegin();
        void vehGenerateBeaconMessageAfterBegin();

        void vehUpdateRateTimeToSend();
        void vehCreateUpdateRateTimeToSendEvent();

        void restartFilesResult();
        void vehInitializeVariables();
        void saveVehStartPosition(string fileNameLocation);

        void sendMessageNeighborsTarget(string beaconSource);
        void onBeaconMessage(WaveShortMessage* wsm);

        void insertMessageDrop(string ID, unsigned short int type);
        void printCountBeaconMessagesDrop();
        void removeOldestInputBeaconMessage();
        void removeOldestInputBeaconStatus();

        string choseCategory_RandomNumber1to100(unsigned short int percentP, string vehIdP, string vehIdT);
        string chosenByDistance(unordered_map<string, shortestDistance> vehShortestDistanceToTarget); // 1
        string chosenByDistance_Speed(unordered_map<string, shortestDistance> vehShortestDistanceToTarget); // 12
        string chosenByDistance_Category(unordered_map<string, shortestDistance> vehShortestDistanceToTarget, int percentP); // 13
        string chosenByDistance_RateTimeToSend(unordered_map<string, shortestDistance> vehShortestDistanceToTarget); // 14
        string chosenByDistance_Speed_Category(unordered_map<string, shortestDistance> vehShortestDistanceToTarget, int percentP); // 123
        string chosenByDistance_Speed_Category_RateTimeToSend(unordered_map<string, shortestDistance> vehShortestDistanceToTarget, int percentP); // 1234

        unsigned short int getVehHeading8();
        unsigned short int getVehHeading4();
};

double vehDist::ttlBeaconStatus;
vector <string> vehDist::numVehicles;
unsigned short int vehDist::countMeetN;
vector <string> vehDist::vehGenerateMessage;
unsigned short int vehDist::numVehToRandom;
unsigned short int vehDist::msgDroppedbyTTL;
unsigned short int vehDist::countVehicleAll;
unsigned short int vehDist::msgDroppedbyHop;
unsigned short int vehDist::beaconMessageId;
unsigned short int vehDist::countMeetPBigerT;
unsigned short int vehDist::countMesssageDrop;
unsigned short int vehDist::countTwoCategoryN;
unsigned short int vehDist::msgDroppedbyBuffer;
unsigned short int vehDist::countMsgPacketSend;
unsigned short int vehDist::msgBufferUseGeneral;
unsigned short int vehDist::timeToUpdatePosition;
unsigned short int vehDist::beaconStatusBufferSize;
unsigned short int vehDist::beaconMessageBufferSize;
unsigned short int vehDist::timeLimitGenerateBeaconMessage;

#endif
