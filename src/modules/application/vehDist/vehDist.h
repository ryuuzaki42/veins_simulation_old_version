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

#include <map>
#include <unordered_map>
using namespace std;

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

        map<string, WaveShortMessage> contextLocalMessageBuffer;
        int countMessage=0;
        static unsigned short int messageId;
        cMessage* sendDataEvt;
        Coord vehPositionBack;
        cMessage* updatePosVeh;
        unordered_map<string, WaveShortMessage> beaconNeighbors;

    protected:
        virtual void onBeacon(WaveShortMessage* wsm);
        virtual void onData(WaveShortMessage* wsm);
        void sendMessage(std::string blockedRoadId);
        virtual void handlePositionUpdate(cObject* obj);
        virtual void handleParkingUpdate(cObject* obj);
        virtual void sendWSM(WaveShortMessage* wsm);

        void sendDataTest();
        void generateTarget();
        WaveShortMessage* generateMessage();
        void handleSelfMsg(cMessage* msg);
        void printContextLocalMessageBuffer();
        void printBeaconNeighbors();
        WaveShortMessage* prepareBeaconWSM(std::string name, int lengthBits, t_channel channel, int priority, unsigned int rcvId, int serial);
        void updatePosition();
        unsigned int getHeading();
        unsigned int MACToInteger(WaveAppToMac1609_4Interface* myMac);
};

unsigned short int vehDist::messageId = 0;
#endif
