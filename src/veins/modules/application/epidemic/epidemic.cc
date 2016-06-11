// Copyright (C) 2015-2016 João Batista <joao.b@usp.br>

#include "veins/modules/application/epidemic/epidemic.h"

using Veins::TraCIMobilityAccess;
using Veins::AnnotationManagerAccess;
using namespace std;

Define_Module(epidemic);

void epidemic::initialize(int stage) {
    BaseWaveApplLayer::initialize_epidemic(stage);
    if (stage == 0) {
        mobility = TraCIMobilityAccess().get(getParentModule());
        traci = mobility->getCommandInterface();
        traciVehicle = mobility->getVehicleCommandInterface();

        vehInitializeVariablesEpidemicVeh();
    }
}

void epidemic::vehInitializeVariablesEpidemicVeh() {
    vehInitializeValuesVehDist(traciVehicle->getTypeId(), mobility->getPositionAt(simTime() + 0.1)); // The same for Epidemic and VehDist
}

void epidemic::onBeacon(WaveShortMessage* wsm) {
    receivedOnBeaconEpidemic(wsm);
}

void epidemic::onData(WaveShortMessage* wsm) {
    receivedOnDataEpidemic(wsm);
}

void epidemic::sendWSM(WaveShortMessage* wsm) {
    bool isParking = mobility->getParkingState();
    if (isParking && !SvehSendWhileParking) {
        return;
    }

    sendDelayedDown(wsm, individualOffset);
}

void epidemic::finish() {
    toFinishVeh();
}
