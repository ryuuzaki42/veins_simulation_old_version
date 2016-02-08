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

#include "BaseWaveApplLayer.h"
#include "modules/mobility/traci/TraCIMobility.h"

using Veins::TraCIMobility;
using Veins::AnnotationManager;

class vehDist : public BaseWaveApplLayer {
    public:
        virtual void initialize(int stage);
        virtual void receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj);

        enum WaveApplMessageKinds {
            SendEvtUpdatePositionVeh, SendEvtBeaconMessage, SendEvtGenerateBeaconMessage
        };

    protected:
        TraCIMobility* traci;
        AnnotationManager* annotations;
        simtime_t lastDroveAt;
        bool sentMessage;
        bool isParking;
        bool sendWhileParking;
        static const simsignalwrap_t parkingStateChangedSignal;

        static unsigned short int beaconMessageId;
        cMessage* sendBeaconMessageEvt;
        cMessage* sendGenerateBeaconMessageEvt;
        cMessage* sendUpdatePosisitonVeh;
        Coord vehPositionPrevious;
        unordered_map<string, WaveShortMessage> messagesBuffer;
        unordered_map<string, WaveShortMessage> beaconStatusNeighbors;
        vector <string> messagesDelivered;
        static unordered_map<string, string> numVehicles;

        int vehNumber;
        double vehOffSet;
        static unordered_map<int, bool> vehGenerateMessage;

        struct messagesDropStruct {
            int byBuffer;
            int byHop;
            int byTime;
        };
        map<string, struct messagesDropStruct> messagesDrop;
        static unsigned short int countMesssageDrop;

    protected:
        virtual void onBeacon(WaveShortMessage* wsm);
        virtual void onData(WaveShortMessage* wsm);
        virtual void handlePositionUpdate(cObject* obj);
        virtual void handleParkingUpdate(cObject* obj);
        void sendMessage(std::string blockedRoadId);

        virtual void sendWSM(WaveShortMessage* wsm);

        void sendBeaconMessage();
        void generateTarget();
        void generateBeaconMessage();
        void handleSelfMsg(cMessage* msg);
        void printMessagesBuffer();
        void printBeaconStatusNeighbors();
        WaveShortMessage* prepareBeaconStatusWSM(std::string name, int lengthBits, t_channel channel, int priority, int serial);
        WaveShortMessage* updateBeaconMessageWSM(WaveShortMessage* wsm, string rcvId);
        void updateVehPosition();
        unsigned int getVehHeading8();
        unsigned int getVehHeading4();
        void saveVehStartPosition(string fileNameLocation);
        void restartFilesResult();
        void vehUpdatePosition();
        void vehCreateEventTrySendBeaconMessage();
        int getVehCategory();
        void removeOldestInput(unordered_map<string, WaveShortMessage>* data, double timeValid, unsigned int bufferLimit);
        void sendMessageNeighborsTarget(string key);
        string returnLastMessageInserted();
        void finish();
        void printCountBeaconMessagesDrop();
        void colorCarryMessage();
        void handleLowerMsg(cMessage* msg);
        void onBeaconStatus(WaveShortMessage* wsm);
        void onBeaconMessage(WaveShortMessage* wsm);
        void setVehNumber();
        void vehGenerateBeaconMessageBegin();
        void vehGenerateBeaconMessageAfterBegin();
        void selectVehGenerateMessage();
        void vehInitializeVariables();
        void insertMessageDrop(string ID, int type);
        string getNeighborSmallDistanceToTarge(string key);
};

unsigned short int vehDist::beaconMessageId = 1;
unsigned short int vehDist::countMesssageDrop = 0;
unordered_map<int, bool> vehDist::vehGenerateMessage;
unordered_map<string, string> vehDist::numVehicles;

#endif
