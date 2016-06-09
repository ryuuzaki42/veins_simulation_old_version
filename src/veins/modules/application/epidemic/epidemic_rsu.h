// Copyright (C) 2015-2016 João Batista <joao.b@usp.br>

#ifndef epidemic_rsu_H
#define epidemic_rsu_H

#include "veins/modules/application/ieee80211p/BaseWaveApplLayer.h"
#include "veins/modules/world/annotations/AnnotationManager.h"

using Veins::AnnotationManager;

class epidemic_rsu : public BaseWaveApplLayer {
    public:
        virtual void initialize(int stage);

    protected:
        BaseMobility* mobi;

    protected:
        void onBeacon(WaveShortMessage* wsm);
        void onData(WaveShortMessage* wsm);

        void finish();

        void epidemicInitializeVariables();
};

#endif
