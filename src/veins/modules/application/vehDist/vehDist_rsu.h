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

#include "veins/modules/application/ieee80211p/BaseWaveApplLayer.h"

class vehDist_rsu : public BaseWaveApplLayer {
    public:
        virtual void initialize(int stage);

    protected:
        BaseMobility* mobi;

    protected:
        virtual void onBeacon(WaveShortMessage* wsm);
        virtual void onData(WaveShortMessage* wsm);

        void finish();
        void handleSelfMsg(cMessage* msg);
        void handleLowerMsg(cMessage* msg);
        WaveShortMessage* prepareBeaconStatusWSM(string name, int lengthBits, t_channel channel, int priority, int serial);

        void rsuInitializeVariables();
        void onBeaconStatus(WaveShortMessage* wsm);
        void onBeaconMessage(WaveShortMessage* wsm);
};
#endif
