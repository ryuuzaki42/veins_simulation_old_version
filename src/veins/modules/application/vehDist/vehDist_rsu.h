// Copyright (C) 2015-2016 João Batista <joao.b@usp.br>

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
