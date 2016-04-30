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
            SendEvtUpdatePositionVeh, SendEvtBeaconMessage, SendEvtGenerateBeaconMessage, SendEvtUpdateRateTimeToSendVeh
        };

    protected:
        TraCIMobility* mobility;
        TraCICommandInterface* traci;
        TraCICommandInterface::Vehicle* traciVehicle;

        cMessage* sendBeaconMessageEvt;
        cMessage* sendGenerateBeaconMessageEvt;
        cMessage* sendUpdatePosisitonVeh;
        cMessage* sendUpdateRateTimeToSendVeh;

        static double ttlBeaconStatus;
        static unsigned short int numVehToRandom;
        static unsigned short int beaconMessageBufferSize;
        static unsigned short int beaconStatusBufferSize;
        static int timeLimitGenerateBeaconMessage;

        static unsigned short int countMesssageDrop;
        static unsigned short int beaconMessageId;
        static vector<string> numVehicles;
        static vector <int> vehGenerateMessage;

        Coord vehPositionPrevious;
        unordered_map<string, WaveShortMessage> messagesBuffer;
        unordered_map<string, WaveShortMessage> beaconStatusNeighbors;
        vector <string> messagesDelivered;
        vector <string> messagesOrderReceived;

        unsigned short int rateTimeToSend;
        unsigned short int rateTimeToSendDistanceControl;
        unsigned short int rateTimeToSendLimitTime;
        simtime_t timeToFinishLastStartSend;
        unsigned short int rateTimeToSendUpdateTime;
        unsigned short int messageToSend;

        mt19937 mt_veh;
        double vehOffSet;
        string vehCategory;

        struct messagesDropStruct {
            unsigned short int byBuffer;
            unsigned short int byHop;
            unsigned short int byTime;
        };
        map<string, struct messagesDropStruct> messagesDrop;

        struct shortestDistance {
            string categoryVeh;
            double speedVeh;
            double distanceToTarget;
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

        void trySendBeaconMessage(string idMessage);
        string neighborWithShortestDistanceToTarge(string key);

        void selectVehGenerateMessage();
        void vehGenerateBeaconMessageBegin();
        void vehGenerateBeaconMessageAfterBegin();

        void updateVehPosition();
        void vehUpdatePosition();
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

        string choseCategory_RandomNumber1to100(int percentP, string vehIdP, string vehIdT);
        string chosenByDistance(unordered_map<string, shortestDistance> vehShortestDistanceToTarget); // 1
        string chosenByDistance_Speed(unordered_map<string, shortestDistance> vehShortestDistanceToTarget); // 12
        string chosenByDistance_Category(unordered_map<string, shortestDistance> vehShortestDistanceToTarget, int percentP); // 13
        string chosenByDistance_RateTimeToSend(unordered_map<string, shortestDistance> vehShortestDistanceToTarget); // 14
        string chosenByDistance_Speed_Category(unordered_map<string, shortestDistance> vehShortestDistanceToTarget, int percentP); // 123
        string chosenByDistance_Speed_Category_RateTimeToSend(unordered_map<string, shortestDistance> vehShortestDistanceToTarget, int percentP); // 1234

        string returnLastMessageInserted();
        string getNeighborShortestDistanceToTarge(string key);
        unsigned short int getVehHeading8();
        unsigned short int getVehHeading4();
};

unsigned short int vehDist::beaconMessageId;
unsigned short int vehDist::countMesssageDrop;
vector <int> vehDist::vehGenerateMessage;
vector<string> vehDist::numVehicles;
unsigned short int vehDist::numVehToRandom;
double vehDist::ttlBeaconStatus;
unsigned short int vehDist::beaconMessageBufferSize;
unsigned short int vehDist::beaconStatusBufferSize;
int vehDist::timeLimitGenerateBeaconMessage;

#endif
