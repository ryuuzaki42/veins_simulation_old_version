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

#include "veins/modules/mobility/traci/TraCIMobility.h"
#include "veins/modules/mobility/traci/TraCICommandInterface.h"
#include "veins/modules/application/ieee80211p/BaseWaveApplLayer.h"

using Veins::TraCIMobility;
using Veins::TraCICommandInterface;

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

        static unsigned short int numVehToRandom, timeLimitGenerateBeaconMessage, msgDroppedbyTTL;
        static unsigned short int msgDroppedbyHop, msgDroppedbyBuffer, countMsgPacketSend, msgBufferUseGeneral;
        static unsigned short int timeToUpdatePosition, beaconMessageBufferSize, beaconStatusBufferSize, countMeetN;
        static unsigned short int countTwoCategoryN, countMeetPshortestT, countVehicleAll, countMesssageDrop, beaconMessageId;
        static unsigned short int percentP;
        static bool usePathHistory;

        static vector <string> numVehicles, vehGenerateMessage;
        vector <string> messagesDelivered, messagesOrderReceived;

        unordered_map <string, string> messagesSendLog;
        static unordered_map <string, WaveShortMessage> vehScenario;

        unordered_map <string, WaveShortMessage> messagesBuffer, messagesOnlyDelivery, beaconStatusNeighbors;

        unsigned short int messageToSend, rateTimeToSend, msgBufferMaxUse;
        unsigned short int rateTimeToSendLimitTime, rateTimeToSendUpdateTime, rateTimeToSendDistanceControl;

        simtime_t timeToFinishLastStartSend;

        mt19937 mt_veh;
        double vehOffSet;
        string vehCategory;

        struct messagesDropStruct {
            unsigned short int byBuffer, byHop, byTime;
        };
        map <string, struct messagesDropStruct> messagesDrop;

        struct shortestDistance {
            Coord senderPos;
            string categoryVeh;
            double distanceToTarget, distanceToTargetCategory, decisionValueDistanceSpeed;
            double decisionValueDistanceRateTimeToSend, decisionValueDistanceSpeedRateTimeToSend, speedVeh;
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
        WaveShortMessage* prepareBeaconStatusWSM(string name, int lengthBits, t_channel channel, int priority, int serial);

        void vehCreateEventTrySendBeaconMessage();
        void generateTarget();
        void sendBeaconMessage();
        void printMessagesBuffer();
        void generateBeaconMessage();
        void colorCarryMessage();
        WaveShortMessage* updateBeaconMessageWSM(WaveShortMessage* wsm, string rcvId);

        void trySendBeaconMessage();
        string neighborWithShortestDistanceToTarge(string idMessage);

        void selectVehGenerateMessage();
        void vehGenerateBeaconMessageBegin();
        void vehGenerateBeaconMessageAfterBegin();

        void vehUpdateRateTimeToSend();
        void vehCreateUpdateRateTimeToSendEvent();

        void restartFilesResult();
        void vehInitializeVariables();
        void saveVehStartPosition(string fileNameLocation);

        void sendMessageToOneNeighborTarget(string beaconSource);
        void sendMessageToOneNeighborTargetOnlyDelivery(string beaconSource);
        bool sendOneNewMessageToOneNeighborTarget(WaveShortMessage wsm);
        void onBeaconMessage(WaveShortMessage* wsm);

        void insertMessageDrop(string ID, unsigned short int type);
        void printCountBeaconMessagesDrop();
        void removeOldestInputBeaconMessage();
        void removeOldestInputBeaconStatus();

        string choseCategory_RandomNumber1to100(unsigned short int percentP, string vehIdP, string vehIdT);
        string chosenByDistance(unordered_map <string, shortestDistance> vehShortestDistanceToTarget); // 0001
        string chosenByDistance_Speed(unordered_map <string, shortestDistance> vehShortestDistanceToTarget); // 0012
        string chosenByDistance_CategoryA(unordered_map <string, shortestDistance> vehShortestDistanceToTarget, int percentP); // 0013
        string chosenByDistance_RateTimeToSend(unordered_map <string, shortestDistance> vehShortestDistanceToTarget); // 0014
        string chosenByDistance_Speed_Category(unordered_map <string, shortestDistance> vehShortestDistanceToTarget, int percentP); // 0123
        string chosenByDistance_Speed_Category_RateTimeToSend(unordered_map <string, shortestDistance> vehShortestDistanceToTarget, int percentP); // 1234
        string chosenByDistance_CategoryB(unordered_map <string, shortestDistance> vehShortestDistanceToTarget, int percentP); // 0013 with uncomment

        unsigned short int getVehHeading8();
        unsigned short int getVehHeading4();
};

double vehDist::ttlBeaconStatus;

vector <string> vehDist::numVehicles, vehDist::vehGenerateMessage;

unordered_map <string, WaveShortMessage> vehDist::vehScenario;

unsigned short int vehDist::msgBufferUseGeneral, vehDist::timeLimitGenerateBeaconMessage;
unsigned short int vehDist::countTwoCategoryN, vehDist::msgDroppedbyBuffer, vehDist::countMsgPacketSend;
unsigned short int vehDist::timeToUpdatePosition, vehDist::beaconStatusBufferSize, vehDist::beaconMessageBufferSize;
unsigned short int vehDist::countMeetN, vehDist::numVehToRandom, vehDist::msgDroppedbyTTL, vehDist::countVehicleAll;
unsigned short int vehDist::msgDroppedbyHop, vehDist::beaconMessageId, vehDist::countMeetPshortestT, vehDist::countMesssageDrop;
unsigned short int vehDist::percentP;
bool vehDist::usePathHistory;

#endif
