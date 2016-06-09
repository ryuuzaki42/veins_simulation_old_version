// Copyright (C) 2015-2016 João Batista <joao.b@usp.br>

#ifndef epidemic_H
#define epidemic_H

#include "veins/modules/application/ieee80211p/BaseWaveApplLayer.h"
#include "veins/modules/mobility/traci/TraCICommandInterface.h"
#include "veins/modules/mobility/traci/TraCIMobility.h"

using Veins::TraCIMobility;
using Veins::TraCICommandInterface;
using Veins::AnnotationManager;

class epidemic : public BaseWaveApplLayer {
    public:
        virtual void initialize(int stage);

    protected:
        TraCIMobility* mobility;
        TraCICommandInterface* traci;
        TraCICommandInterface::Vehicle* traciVehicle;

    protected:
        void onBeacon(WaveShortMessage* wsm);
        void onData(WaveShortMessage* wsm);

        void finish();

        void sendWSM(WaveShortMessage* wsm);

        void vehInitializeVariablesEpidemicVeh();
};

#endif
