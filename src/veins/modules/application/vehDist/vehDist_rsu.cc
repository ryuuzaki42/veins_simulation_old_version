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
    generalInitializeVariables_executionByExpNumber();

    restartFilesResult();
    //cout << source << " entered in the scenario" << endl;
}

void vehDist_rsu::restartFilesResult() {
    string folderResult = getFolderResult(expSendbyDSCR);

    fileMessagesBroadcast = fileMessagesUnicast = fileMessagesCount = folderResult + source + "_";

    fileMessagesBroadcast += "Broadcast_Messages.r";
    fileMessagesUnicast += "Messages_Received.r";
    fileMessagesCount += "Count_Messages_Received.r";

    //fileMessagesDrop and fileMessagesGenerated // Not used yet to RSU

    if ((myId == 0) && (repeatNumber == 0)) { // Open a new file (blank)
        if (expNumber <= 4) { // Set the maxSpeed to 15 m/s in the expNumber 1 to 4
            string comand = "sed -i 's/maxSpeed=.* color/maxSpeed=\"15\" color/g' vehDist.rou.xml";
            system(comand.c_str());
            cout << endl << "Change the speed to 15 m/s, command: " << comand << endl;
        } else if (expNumber >= 5){ // Set the maxSpeed to 25 m/s in the expNumber 5 to 8
            string comand = "sed -i 's/maxSpeed=.* color/maxSpeed=\"25\" color/g' vehDist.rou.xml";
            system(comand.c_str());
            cout << endl << "Change the speed to 25 m/s, command: " << comand << endl;
        }

        string commandCreateFolder = "mkdir -p " + folderResult + " > /dev/null";
        cout << endl << "Created the folder, command: \"" << commandCreateFolder << "\"" << endl;
        cout << "repeatNumber: " << repeatNumber << endl;
        system(commandCreateFolder.c_str()); // Create a folder results

        openFileAndClose(fileMessagesBroadcast, false, ttlBeaconMessage, countGenerateBeaconMessage);
        openFileAndClose(fileMessagesUnicast, false, ttlBeaconMessage, countGenerateBeaconMessage);
        openFileAndClose(fileMessagesCount, false, ttlBeaconMessage, countGenerateBeaconMessage);
    } else { // repeatNumber != 0 just append
        openFileAndClose(fileMessagesBroadcast, true, ttlBeaconMessage, countGenerateBeaconMessage);
        openFileAndClose(fileMessagesUnicast, true, ttlBeaconMessage, countGenerateBeaconMessage);
        openFileAndClose(fileMessagesCount, true, ttlBeaconMessage, countGenerateBeaconMessage);
    }
}

void vehDist_rsu::handleLowerMsg(cMessage* msg) {
    WaveShortMessage* wsm = dynamic_cast<WaveShortMessage*>(msg);
    ASSERT(wsm);

    if (wsm->getType() == 1) {
        onBeaconStatus(wsm);
    } else if (wsm->getType() == 2) {
        onBeaconMessage(wsm);
    } else {
        DBG << "unknown message (" << wsm->getName() << ")  received\n";
        cout << "unknown message (" << wsm->getName() << ")  received\n";
        exit(1);
    }
    delete(msg);
}

void vehDist_rsu::onBeaconStatus(WaveShortMessage* wsm) {
}

void vehDist_rsu::onBeaconMessage(WaveShortMessage* wsm) {
    if (source.compare(wsm->getRecipientAddressTemporary()) == 0) {
        findHost()->bubble("Received Message");
        saveMessagesOnFile(wsm, fileMessagesUnicast);

        messagesReceivedMeasuringRSU(wsm);
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
