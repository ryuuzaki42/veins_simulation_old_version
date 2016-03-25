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
        //virtual void receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj);

        enum WaveApplMessageKinds {
            SendEvtUpdatePositionVeh, SendEvtBeaconMessage, SendEvtGenerateBeaconMessage, SendEvtUpdateTimeToSendVeh
        };

    protected:
        TraCIMobility* mobility;
        TraCICommandInterface* traci;
        TraCICommandInterface::Vehicle* traciVehicle;
//        AnnotationManager* annotations;
//        simtime_t lastDroveAt;
//        bool sentMessage;
//        bool isParking;
//        bool sendWhileParking;
//        static const simsignalwrap_t parkingStateChangedSignal;

        cMessage* sendBeaconMessageEvt;
        cMessage* sendGenerateBeaconMessageEvt;
        cMessage* sendUpdatePosisitonVeh;
        cMessage* sendUpdateTimeToSendVeh;

        static unsigned short int countMesssageDrop;
        static unsigned short int beaconMessageId;
        static vector<string> numVehicles;
        static unordered_map<unsigned short int, bool> vehGenerateMessage;

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

        float vehOffSet;
        float ttlBeaconStatus;
        string vehCategory;
        float timeLimitGenerateBeaconMessage;
        unsigned short int beaconMessageBufferSize;
        unsigned short int beaconStatusBufferSize;

        struct messagesDropStruct {
            unsigned short int byBuffer;
            unsigned short int byHop;
            unsigned short int byTime;
        };
        map<string, struct messagesDropStruct> messagesDrop;

        struct shortestDistance {
            string categoryVeh;
            float speedVeh;
            unsigned short int rateTimeToSendVeh;
            unsigned short int distanceToTarget; // Int to ignore small differences
            unsigned short int decisionValueDistanceSpeed;
            unsigned short int decisionValueDistanceSpeedRateTimeToSend;
        };

    protected:
        virtual void onBeacon(WaveShortMessage* wsm);
        virtual void onData(WaveShortMessage* wsm);
//        virtual void handlePositionUpdate(cObject* obj);
//        virtual void handleParkingUpdate(cObject* obj);

        void finish();
        void handleSelfMsg(cMessage* msg);
        void handleLowerMsg(cMessage* msg);
        WaveShortMessage* prepareBeaconStatusWSM(std::string name, int lengthBits, t_channel channel, int priority, int serial);
        WaveShortMessage* updateBeaconMessageWSM(WaveShortMessage* wsm, string rcvId);

        void generateTarget();
        void generateBeaconMessage();
        void printMessagesBuffer();
        void printBeaconStatusNeighbors();
        void updateVehPosition();
        unsigned short int getVehHeading8();
        unsigned short int getVehHeading4();
        void saveVehStartPosition(string fileNameLocation);
        void restartFilesResult();
        void vehUpdatePosition();
        void vehCreateEventTrySendBeaconMessage();
        void sendMessageNeighborsTarget(string beaconSource);
        string returnLastMessageInserted();
        void printCountBeaconMessagesDrop();
        void colorCarryMessage();
        void onBeaconStatus(WaveShortMessage* wsm);
        void onBeaconMessage(WaveShortMessage* wsm);
        void vehGenerateBeaconMessageBegin();
        void vehGenerateBeaconMessageAfterBegin();
        void selectVehGenerateMessage();
        void vehInitializeVariables();
        void insertMessageDrop(string ID, int type);
        string getNeighborShortestDistanceToTarge(string key);
        string neighborWithShortestDistanceToTarge(string key);
        void removeOldestInputBeaconMessage();
        void removeOldestInputBeaconStatus();
        void vehUpdateTimeToSend();
        void vehCreateUpdateTimeToSendEvent();
        void sendBeaconMessage();
        void trySendBeaconMessage(string idMessage);
        string chosenByDistance(unordered_map<string, shortestDistance> vehShortestDistanceToTarget);
        string chosenByDistance_Speed(unordered_map<string, shortestDistance> vehShortestDistanceToTarget);
        string chosenByDistance_Speed_Category(unordered_map<string, shortestDistance> vehShortestDistanceToTarget, unsigned short int percentP);
        string chosenByDistance_Speed_Category_RateTimeToSend(unordered_map<string, shortestDistance> vehShortestDistanceToTarget, unsigned short int percentP);
};

unsigned short int vehDist::beaconMessageId = 1;
unsigned short int vehDist::countMesssageDrop = 0;
unordered_map<unsigned short int, bool> vehDist::vehGenerateMessage;
vector<string> vehDist::numVehicles;

#endif
