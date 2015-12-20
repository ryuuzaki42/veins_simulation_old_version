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
          int HopsCount;
          string hops;
          int minHop;
          int maxHop;
          double averageHops;
          string wsmData;
        };

        unordered_map<string,  struct messages> messagesReceived;

    protected:
        virtual void onBeacon(WaveShortMessage* wsm);
        virtual void onData(WaveShortMessage* wsm);
        void sendMessage(std::string blockedRoadId);
        virtual void sendWSM(WaveShortMessage* wsm);

        WaveShortMessage* prepareBeaconWSM(std::string name, int lengthBits, t_channel channel, int priority, unsigned int rcvId, int serial);
        void saveMessagesOnFile(WaveShortMessage* wsm, string fileName);
        void fieldsToSave(WaveShortMessage* wsm);
        void handleSelfMsg(cMessage* msg);
        void restartFilesResult();
        virtual void finish();
        void printCountMessagesReceived();
        void messagesReceivedMeasuring(WaveShortMessage* wsm);
};

#endif
