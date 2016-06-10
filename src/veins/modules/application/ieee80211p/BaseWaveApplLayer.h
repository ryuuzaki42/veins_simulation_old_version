// Copyright (C) 2015-2016 João Batista <joao.b@usp.br>

#ifndef BASEWAVEAPPLLAYER_H_
#define BASEWAVEAPPLLAYER_H_

#include "veins/base/modules/BaseApplLayer.h"
#include "veins/modules/utility/Consts80211p.h"
#include "veins/modules/messages/WaveShortMessage_m.h"
#include "veins/base/connectionManager/ChannelAccess.h"
#include "veins/modules/mac/ieee80211p/WaveAppToMac1609_4Interface.h"

#include <fstream>
#include <unordered_map>
#include <algorithm> // std::find

using namespace std;

#ifndef DBG
#define DBG EV
#endif

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
            SEND_BEACON_EVT_epidemic, Send_EpidemicMessageRequestEvt,
            SendEvtGenerateBeaconMessage
        };

    protected:
        static const simsignalwrap_t mobilityStateChangedSignal;

        virtual void handleLowerMsg(cMessage* msg); /** @brief handle messages from below */

        virtual void handleSelfMsg(cMessage* msg); /** @brief handle self messages */

        //virtual WaveShortMessage* prepareWSM(string name, int dataLengthBits, t_channel channel, int priority, int rcvId, int serial=0);
        virtual WaveShortMessage* prepareWSM(string name, int dataLengthBits, t_channel channel, int priority, unsigned int rcvId, int serial=0);

//######################################### vehDist #########################################
        //## Used to another projects
        void generalInitializeVariables_executionByExpNumberVehDist();
        string getFolderResultVehDist(unsigned short int experimentSendbyDSR);

        void toFinishRSU();
        void finishVeh();
        string boolToString(bool value);

        void restartFilesResultRSU(string folderResult);
        void restartFilesResultVeh(string projectInfo, Coord initialPos);
        void saveVehStartPositionVeh(string fileNameLocation, Coord initialPos);
        void insertMessageDropVeh(string ID, unsigned short int type, simtime_t timeGenarted);

        void printCountMessagesReceivedRSU();
        void messagesReceivedMeasuringRSU(WaveShortMessage* wsm);

        void vehGenerateBeaconMessageBeginVeh(double vehOffSet);
        void vehGenerateBeaconMessageAfterBeginVeh();
        void selectVehGenerateMessage();

        void generateBeaconMessageVehDist();
        void generateTarget();
        void colorCarryMessageVehDist(unordered_map <string, WaveShortMessage> bufferOfMessages);
        void printCountBeaconMessagesDropVeh();

        void saveMessagesOnFile(WaveShortMessage* wsm, string fileName);
        void printHeaderfileExecution();
        void openFileAndClose(string fileName, bool justForAppend);
//######################################### vehDist #########################################

//######################################### Epidemic #########################################
        virtual WaveShortMessage* prepareWSM_epidemic(string name, int dataLengthBits, t_channel channel, int priority, unsigned int rcvId, int serial=0);
        unsigned int MACToInteger();

        void receivedOnBeacon(WaveShortMessage* wsm);
        void receivedOnData(WaveShortMessage* wsm);

        void printWaveShortMessageEpidemic(WaveShortMessage* wsm);

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

        void generateMessageEpidemic();
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
        cMessage* sendGenerateBeaconMessageEvt;

        mt19937 mt_veh;

        double vehOffSet;

        vector <string> messagesOrderReceived;

        unsigned short int target_x, target_y, msgBufferMaxUse;

        unordered_map <string, WaveShortMessage> messagesBuffer;

        string fileMessagesUnicast, fileMessagesBroadcast, fileMessagesCount, fileMessagesDrop, fileMessagesGenerated;

        //## Used to another projects
        ofstream myfile; // record in file

        static unsigned short int SrepeatNumber, SexpNumber, SexpSendbyDSCR, ScountGenerateBeaconMessage, SttlBeaconMessage;

        static unsigned short int SmsgDroppedbyTTL, SmsgDroppedbyCopy, SmsgDroppedbyBuffer;
        static unsigned short int ScountMsgPacketSend, SmsgBufferUseGeneral, ScountMesssageDrop, SbeaconMessageHopLimit;
        static unsigned short int ScountMeetN, ScountTwoCategoryN, ScountMeetPshortestT, ScountVehicleAll, SbeaconMessageId;

        static string SprojectInfo;

        static vector <string> SnumVehicles, SvehGenerateMessage;

        static unordered_map <string, WaveShortMessage> SvehScenario;
        static bool SvehDistTrueEpidemicFalse, SusePathHistory, SallowMessageCopy, SvehSendWhileParking;
        static bool SselectFromAllVehicles, SuseMessagesSendLog, SvehDistCreateEventGenerateMessage;

        static unsigned short int SbeaconStatusBufferSize, SttlBeaconStatus, SpercentP;
        static unsigned short int StimeLimitGenerateBeaconMessage, StimeToUpdatePosition, SbeaconMessageBufferSize;

        struct messages {
          string firstSource, hops, wsmData, times;
          unsigned short int minHop, maxHop, sumHops, countT, countP, copyMessage;
          simtime_t sumTimeRecived;
        };
        map <string, struct messages> messagesReceived;

        struct messagesDropStruct {
            unsigned short int byBuffer, byTime, byCopy;
            simtime_t timeGenerate, timeDroped, timeDifference;
        };
        map <string, struct messagesDropStruct> messagesDrop;
//######################################### vehDist #########################################

//######################################### Epidemic #########################################
        unordered_map <string, WaveShortMessage> epidemicLocalMessageBuffer, epidemicMessageSend;

        unordered_map <string, bool> epidemicLocalSummaryVector, epidemicRemoteSummaryVector, epidemicRequestMessageVector;

        unordered_map <unsigned int, simtime_t> nodesRecentlyFoundList, nodesIRecentlySentSummaryVector;

        unsigned int sendSummaryVectorInterval, maximumEpidemicBufferSize;
        unsigned int nodesRecentlySendLocalSummaryVector = 0;
        simtime_t lastTimeSendLocalSummaryVector = 0;

        cMessage* sendEpidemicMessageRequestEvt;
//######################################### Epidemic #########################################
};

#endif
