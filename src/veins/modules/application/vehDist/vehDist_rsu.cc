// Copyright (C) 2015-2016 João Batista <joao.b@usp.br>

#include "veins/modules/application/vehDist/vehDist_rsu.h"

Define_Module(vehDist_rsu);

void vehDist_rsu::initialize(int stage) {
    BaseWaveApplLayer::initialize_default_veins_TraCI(stage);
    if (stage == 0) {
        mobi = dynamic_cast<BaseMobility*> (getParentModule()->getSubmodule("mobility"));
        ASSERT(mobi);

        rsuInitializeVariables();
    }
}

void vehDist_rsu::rsuInitializeVariables() {
    generalInitializeVariables_executionByExpNumberVehDist();

    restartFilesResultRSU(getFolderResultVehDist(SexpSendbyDSCR));
    //cout << source << " entered in the scenario" << endl;
}

void vehDist_rsu::handleLowerMsg(cMessage* msg) {
    WaveShortMessage* wsm = dynamic_cast<WaveShortMessage*>(msg);
    ASSERT(wsm);

    if (wsm->getType() == 1) {
        onBeaconStatus(wsm);
    } else if (wsm->getType() == 2) {
        onBeaconMessage(wsm);
    } else {
        cout << "unknown message (" << wsm->getName() << ")  received\n";
        exit(1);
    }
    delete(msg);
}

void vehDist_rsu::onBeaconStatus(WaveShortMessage* wsm) {
}

void vehDist_rsu::onBeaconMessage(WaveShortMessage* wsm) {
    if (source.compare(wsm->getRecipientAddressTemporary()) == 0) {
        if (source.compare(wsm->getTarget()) == 0) {
            findHost()->bubble("Received Message");
            saveMessagesOnFile(wsm, fileMessagesUnicast);

            messagesReceivedMeasuringRSU(wsm);
        }/* else {
            real scenario save this message in the buffer
        }*/
    } else {
        saveMessagesOnFile(wsm, fileMessagesBroadcast);
    }
}

void vehDist_rsu::handleSelfMsg(cMessage* msg) {
    switch (msg->getKind()) {
        case SEND_BEACON_EVT: {
            sendWSM(prepareBeaconStatusWSM("beaconStatus", beaconLengthBits, type_CCH, beaconPriority, -1));
            scheduleAt(simTime() + par("beaconInterval").doubleValue(), sendBeaconEvt);
            break;
        }
        default: {
            if (msg) {
                DBG << "APP: Error: Got Self Message of unknown kind! Name: " << msg->getName() << endl;
                exit(2);
            }
            break;
        }
    }
}

WaveShortMessage* vehDist_rsu::prepareBeaconStatusWSM(string name, int lengthBits, t_channel channel, int priority, int serial) {
    WaveShortMessage* wsm = new WaveShortMessage(name.c_str());
    wsm->setType(1); // Beacon of Status
    wsm->addBitLength(headerLength);
    wsm->addBitLength(lengthBits);
    switch (channel) {
        case type_SCH: // Will be rewritten at Mac1609_4 to actual Service Channel. This is just so no controlInfo is needed
            wsm->setChannelNumber(Channels::SCH1);
            break;
        case type_CCH:
            wsm->setChannelNumber(Channels::CCH);
            break;
    }
    wsm->setPsid(0);
    wsm->setPriority(priority);
    wsm->setWsmVersion(1);
    wsm->setSerial(serial);
    wsm->setTimestamp(simTime());
    wsm->setSenderPos(curPosition);
    //wsm->setSenderPos(Coord(1000,1000,3));
    wsm->setSource(source.c_str());

    //beacon don't need
    //wsm->setSenderPosPrevious(curPosition); // RSU can't move
    //wsm->setRecipientAddressString(); => "BROADCAST"
    //wsm->setSenderAddressTemporary();
    //wsm->setTarget(); => "BROADCAST"
    //wsm->setRateTimeToSend();

    DBG << "Creating BeaconStatus with Priority " << priority << " at Applayer at " << wsm->getTimestamp() << std::endl;
    return wsm;
}

void vehDist_rsu::finish() {
    toFinishRSU();
}

// #####################################################################################################
void vehDist_rsu::onBeacon(WaveShortMessage* wsm) {
}

void vehDist_rsu::onData(WaveShortMessage* wsm) {
}
