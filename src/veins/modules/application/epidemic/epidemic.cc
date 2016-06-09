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
    generalInitializeVariables_executionByExpNumberVehDist();

    vehOffSet = double(myId)/1000; // Simulate asynchronous channel access. Values between 0.001, 0.002
    SnumVehicles.push_back(source);
    ScountVehicleAll++;

    //vehCategory = traciVehicle->getTypeId();

    WaveShortMessage wsmTmp;
    wsmTmp.setTimestamp(simTime());
    //w.setCategory(vehCategory.c_str());
    SvehScenario.insert(make_pair(source, wsmTmp));

    restartFilesResultVeh(SprojectInfo, mobility->getPositionAt(simTime() + 0.1));

    if (SvehDist_create_eventGenerateMessage) {
        vehGenerateBeaconMessageBeginVeh(vehOffSet); // Create Event to generate messages
    } else {
        generateMessageEpidemic();
    }

    cout << source <<  " myMac: " << myMac << " MACToInteger: " << MACToInteger() << " entered in the scenario" << endl;
}

void epidemic::onBeacon(WaveShortMessage* wsm) {
    receivedOnBeacon(wsm);
}

void epidemic::onData(WaveShortMessage* wsm) {
    receivedOnData(wsm);
}

void epidemic::sendWSM(WaveShortMessage* wsm) {
    bool isParking = mobility->getParkingState();
    if (isParking && !SvehSendWhileParking) {
        return;
    }

    sendDelayedDown(wsm, individualOffset);
}

void epidemic::finish() {
    finishVeh();
}
