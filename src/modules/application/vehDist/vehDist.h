//
// Copyright (C) 2006-2011 Christoph Sommer <christoph.sommer@uibk.ac.at>
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
            SEND_updatePosVeh, SEND_DATA_EVT
        };

    protected:
        TraCIMobility* traci;
        AnnotationManager* annotations;
        simtime_t lastDroveAt;
        bool sentMessage;
        bool isParking;
        bool sendWhileParking;
        static const simsignalwrap_t parkingStateChangedSignal;

        static unsigned short int messageId;
        cMessage* sendDataEvt;
        Coord vehPositionBack;
        cMessage* updatePosVeh;
        unordered_map<string, WaveShortMessage> messagesBuffer;
        unordered_map<string, WaveShortMessage> beaconNeighbors;
        unordered_map<string, int> messagesDrop;
        static unsigned short int countMesssageDrop;

    protected:
        virtual void onBeacon(WaveShortMessage* wsm);
        virtual void onData(WaveShortMessage* wsm);
        virtual void handlePositionUpdate(cObject* obj);
        virtual void handleParkingUpdate(cObject* obj);
        virtual void sendWSM(WaveShortMessage* wsm);

        void sendMessage(std::string blockedRoadId);

        void sendDataMessage();
        void generateTarget();
        void generateMessage();
        void handleSelfMsg(cMessage* msg);
        void printMessagesBuffer();
        void printBeaconNeighbors();
        WaveShortMessage* prepareBeaconWSM(std::string name, int lengthBits, t_channel channel, int priority, unsigned int rcvId, int serial);
        WaveShortMessage* updateMessageWSM(WaveShortMessage* wsm, string rcvId);
        void updatePosition();
        unsigned int getHeading8();
        unsigned int getHeading4();
        bool sendtoTargetbyVeh(Coord vehicleRemoteCoordBack, Coord vehicleRemoteCoordNow, int vehicleRemoteHeading, Coord targetCoord);
        void saveVehStartPosition();
        void restartFilesResult();
        void vehUpdatePosition();
        void vehSendData();
        int getCategory();
        void saveMessagesOnFile(WaveShortMessage* wsm, string file);
        void removeOldestInput(unordered_map<string, WaveShortMessage>* data, double timeValid, unsigned int bufferLimit);
        void sendMessageNeighborsTarget();
        string returnLastMessageInsert();
        void printHeaderfileExecution();
        void finish();
        void printCountMessagesDrop();
};

unsigned short int vehDist::messageId = 0;
unsigned short int vehDist::countMesssageDrop=0;
#endif
