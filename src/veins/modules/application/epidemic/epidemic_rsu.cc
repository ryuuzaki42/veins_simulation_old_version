// Copyright (C) 2015-2016 João Batista <joao.b@usp.br>

#include "veins/modules/application/epidemic/epidemic_rsu.h"

using Veins::AnnotationManagerAccess;

Define_Module(epidemic_rsu);

void epidemic_rsu::initialize(int stage) {
    BaseWaveApplLayer::initialize_epidemic(stage);
    if (stage == 0) {
        mobi = dynamic_cast<BaseMobility*> (getParentModule()->getSubmodule("mobility"));
        ASSERT(mobi);

        epidemicInitializeVariables();
    }
}

void epidemic_rsu::epidemicInitializeVariables() {
    generalInitializeVariables_executionByExpNumberVehDist();

    restartFilesResultRSU(getFolderResultVehDist(SexpSendbyDSCR));

    //cout << source << " myMac: " << myMac << " MACToInteger: " << MACToInteger() << endl;
}

void epidemic_rsu::onBeacon(WaveShortMessage* wsm) {
    receivedOnBeaconEpidemic(wsm);
}

void epidemic_rsu::onData(WaveShortMessage* wsm) {
    receivedOnDataEpidemic(wsm);
}

void epidemic_rsu::finish() {
    toFinishRSU();
}
