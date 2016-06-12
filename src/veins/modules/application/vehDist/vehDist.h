// Copyright (C) 2015-2016 João Batista <joao.b@usp.br>

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
            SendEvtBeaconMessage, SendEvtUpdateRateTimeToSendVeh
        };

    protected:
        TraCIMobility* mobility;
        TraCICommandInterface* traci;
        TraCICommandInterface::Vehicle* traciVehicle;

        cMessage* sendBeaconMessageEvt;
        cMessage* sendUpdateRateTimeToSendVeh;

        vector <string> messagesDelivered;

        unordered_map <string, string> messagesSendLog;

        unordered_map <string, WaveShortMessage> beaconStatusNeighbors;

        unsigned short int messageToSend, rateTimeToSend;
        unsigned short int rateTimeToSendLimitTime, rateTimeToSendUpdateTime, rateTimeToSendDistanceControl;

        simtime_t timeToFinishLastStartSend;

        struct shortestDistance {
            Coord senderPos;
            string categoryVeh;
            double distanceToTargetNow, distanceToTargetBefore, distanceToTargetCategory, decisionValueDistanceSpeed;
            double decisionValueDistanceRateTimeToSend, decisionValueDistanceSpeedRateTimeToSend, speedVeh;
            unsigned short int rateTimeToSendVeh;
        };

    protected:
        virtual void onBeacon(WaveShortMessage* wsm);
        virtual void onData(WaveShortMessage* wsm);

        void finish();
        void handleSelfMsg(cMessage* msg);
        void handleLowerMsg(cMessage* msg);
        void sendWSM(WaveShortMessage* wsm);

        void printBeaconStatusNeighbors();
        void onBeaconStatus(WaveShortMessage* wsm);
        WaveShortMessage* prepareBeaconStatusWSM(string name, int lengthBits, t_channel channel, int priority, int serial);

        void vehCreateEventTrySendBeaconMessage();
        void sendBeaconMessage();
        void printMessagesBuffer();
        WaveShortMessage* updateBeaconMessageWSM(WaveShortMessage* wsm, string rcvId);

        void trySendBeaconMessage();
        string neighborWithShortestDistanceToTarge(string idMessage);
        string neighborWithShortestDistanceToTargeOnlyDelivery(string idMessage);

        void vehUpdateRateTimeToSend();
        void vehCreateUpdateRateTimeToSendEvent();

        void vehInitializeVariablesVehDistVeh();

        void sendMessageToOneNeighborTarget(string beaconSource);
        bool sendOneNewMessageToOneNeighborTarget(WaveShortMessage wsm);
        void onBeaconMessage(WaveShortMessage* wsm);

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

vector <string> BaseWaveApplLayer::SnumVehicles, BaseWaveApplLayer::SvehGenerateMessage;

unordered_map <string, WaveShortMessage> BaseWaveApplLayer::SvehScenario;

unsigned short int BaseWaveApplLayer::SmsgDroppedbyTTL, BaseWaveApplLayer::SmsgDroppedbyCopy, BaseWaveApplLayer::SmsgDroppedbyBuffer;
unsigned short int BaseWaveApplLayer::ScountMsgPacketSend, BaseWaveApplLayer::SmsgBufferUseGeneral, BaseWaveApplLayer::SbeaconMessageHopLimit;
unsigned short int BaseWaveApplLayer::ScountMesssageDrop, BaseWaveApplLayer::ScountMeetN, BaseWaveApplLayer::ScountTwoCategoryN;
unsigned short int BaseWaveApplLayer::ScountMeetPshortestT, BaseWaveApplLayer::ScountVehicleAll;
unsigned short int BaseWaveApplLayer::SbeaconMessageId;

unsigned short int BaseWaveApplLayer::SrepeatNumber, BaseWaveApplLayer::SexpNumber, BaseWaveApplLayer::SexpSendbyDSCR;
unsigned short int BaseWaveApplLayer::ScountGenerateBeaconMessage, BaseWaveApplLayer::SttlBeaconMessage, BaseWaveApplLayer::SvehTimeLimitToAcceptGenerateMgs;

unsigned short int BaseWaveApplLayer::SbeaconStatusBufferSize, BaseWaveApplLayer::SttlBeaconStatus, BaseWaveApplLayer::SpercentP;
unsigned short int BaseWaveApplLayer::StimeLimitGenerateBeaconMessage, BaseWaveApplLayer::StimeToUpdatePosition, BaseWaveApplLayer::SbeaconMessageBufferSize;

string BaseWaveApplLayer::SprojectInfo;
bool BaseWaveApplLayer::SvehDistTrueEpidemicFalse, BaseWaveApplLayer::SusePathHistory, BaseWaveApplLayer::SallowMessageCopy;
bool BaseWaveApplLayer::SvehSendWhileParking, BaseWaveApplLayer::SselectFromAllVehicles, BaseWaveApplLayer::SuseMessagesSendLog;
bool BaseWaveApplLayer::SvehDistCreateEventGenerateMessage;

#endif
