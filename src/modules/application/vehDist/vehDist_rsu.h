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

#ifndef vehDist_rsu_H
#define vehDist_rsu_H


#include "BaseWaveApplLayer.h"
#include "modules/world/annotations/AnnotationManager.h"

using Veins::AnnotationManager;

class vehDist_rsu : public BaseWaveApplLayer {
    public:
        virtual void initialize(int stage);
    protected:
        AnnotationManager* annotations;
        BaseMobility* mobi;
        bool sentMessage;

        struct messages {
          unsigned short int copyMessage;
          string hops;
          unsigned short int minHop;
          unsigned short int maxHop;
          unsigned short int sumHops;
          string wsmData;
          simtime_t sumTimeRecived;
          string times;
        };
        map<string, struct messages> messagesReceived;

    protected:
        virtual void onBeacon(WaveShortMessage* wsm);
        virtual void onData(WaveShortMessage* wsm);
        virtual void sendWSM(WaveShortMessage* wsm);

        void sendMessage(std::string blockedRoadId);
        void handleSelfMsg(cMessage* msg);
        void finish();

        WaveShortMessage* prepareBeaconStatusWSM(std::string name, int lengthBits, t_channel channel, int priority, int serial);
        void restartFilesResult();
        void printCountMessagesReceived();
        void messagesReceivedMeasuring(WaveShortMessage* wsm);
        void handleLowerMsg(cMessage* msg);
        void onBeaconStatus(WaveShortMessage* wsm);
        void onBeaconMessage(WaveShortMessage* wsm);
        void rsuInitializeVariables();
};
#endif
