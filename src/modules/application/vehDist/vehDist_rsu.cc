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

#include "application/vehDist/vehDist_rsu.h"

using Veins::AnnotationManagerAccess;

Define_Module(vehDist_rsu);

void vehDist_rsu::initialize(int stage) {
    BaseWaveApplLayer::initialize_default_veins_TraCI(stage);
    if (stage == 0) {
        mobi = dynamic_cast<BaseMobility*> (getParentModule()->getSubmodule("mobility"));
        ASSERT(mobi);
        annotations = AnnotationManagerAccess().getIfExists();
        ASSERT(annotations);

        rsuInitializeVariables();
    }
}

void vehDist_rsu::rsuInitializeVariables() {
    generalInitializeVariables_executionByExperimentNumber();

    restartFilesResult();
    //cout << " " << findHost()->getFullName() << " entered in the scenario" << endl;
}

void vehDist_rsu::onBeacon(WaveShortMessage* wsm) {
}

void vehDist_rsu::onBeaconStatus(WaveShortMessage* wsm) {
    // to do?
}

void vehDist_rsu::handleLowerMsg(cMessage* msg) {
    WaveShortMessage* wsm = dynamic_cast<WaveShortMessage*>(msg);
    ASSERT(wsm);

    if (std::string(wsm->getName()) == "beacon") {
        onBeacon(wsm);
    }
    else if (std::string(wsm->getName()) == "data") {
        onData(wsm);
    }
//#################################################################
    else if (std::string(wsm->getName()) == "data2veh") {
        cout << "data2veh now" << endl;
    }
    else if (std::string(wsm->getName()) == "data2rsu") {
        cout << "data2rsu now" << endl;
    }
    else if (std::string(wsm->getName()) == "beaconStatus") {
        onBeaconStatus(wsm);
    }
    else if (std::string(wsm->getName()) == "beaconMessage") {
        onBeaconMessage(wsm);
    }
//#################################################################
    else{
        DBG << "unknown message (" << wsm->getName() << ")  received\n";
    }
    delete(msg);
}

void vehDist_rsu::restartFilesResult() {
    stringTmp = "results/resultsEnd/E" + to_string(experimentNumber);
    stringTmp += "_" + to_string(ttlBeaconMessage) + "_" + to_string(countGenerateBeaconMessage) +"/";

    fileMessagesBroadcast = fileMessagesUnicast = fileMessagesCount = fileMessagesDrop = stringTmp;


    fileMessagesBroadcast += findHost()->getFullName();
    fileMessagesBroadcast += "_Broadcast_Messages.r";

    fileMessagesUnicast += findHost()->getFullName();
    fileMessagesUnicast += "_Messages_Received.r";

    fileMessagesCount += findHost()->getFullName();
    fileMessagesCount += "_Count_Messages_Received.r";

    //fileMessagesDrop = ...

    if (strcmp(findHost()->getFullName(), "rsu[0]") == 0) {
        if (repeatNumber == 0) { //Open a new file (blank)

            if (experimentNumber == 1) { // maxSpeed 15 m/s
                system("sed -i 's/maxSpeed=\"..\"/maxSpeed=\"15\"/g' test_end.rou.xml");
            } else if (experimentNumber == 5){  // maxSpeed 25 m/s
                system("sed -i 's/maxSpeed=\"..\"/maxSpeed=\"25\"/g' test_end.rou.xml");
            }

            //system("mkdir results 2> /dev/null"); //create a folder results
            stringTmp = "mkdir -p " + stringTmp + " > /dev/null";
            cout << endl << "Created the folder, comand: \"" << stringTmp << "\"" << endl;
            cout << "repeatNumber " << repeatNumber << endl;
            system(stringTmp.c_str()); //create a folder results

            openFileAndClose(fileMessagesBroadcast, false, ttlBeaconMessage, countGenerateBeaconMessage);
            openFileAndClose(fileMessagesUnicast, false, ttlBeaconMessage, countGenerateBeaconMessage);
            openFileAndClose(fileMessagesCount, false, ttlBeaconMessage, countGenerateBeaconMessage);
        } else{ // (repeatNumber != 0)) // for just append
            openFileAndClose(fileMessagesBroadcast, true, ttlBeaconMessage, countGenerateBeaconMessage);
            openFileAndClose(fileMessagesUnicast, true, ttlBeaconMessage, countGenerateBeaconMessage);
            openFileAndClose(fileMessagesCount, true, ttlBeaconMessage, countGenerateBeaconMessage);
        }
    }
}

void vehDist_rsu::onData(WaveShortMessage* wsm) {
}

void vehDist_rsu::onBeaconMessage(WaveShortMessage* wsm) {
    if (strcmp(wsm->getRecipientAddressTemporary(), findHost()->getFullName()) == 0) {
        findHost()->bubble("Received Message");
        saveMessagesOnFile(wsm, fileMessagesUnicast);

        messagesReceivedMeasuring(wsm);
    } else {
        saveMessagesOnFile(wsm, fileMessagesBroadcast);
    }
}

void vehDist_rsu::messagesReceivedMeasuring(WaveShortMessage* wsm) {
    map<string, struct messages>::iterator it = messagesReceived.find(wsm->getGlobalMessageIdentificaton());
    if (it != messagesReceived.end()) {
        it->second.hops += ", ";
        it->second.hops += to_string(beaconMessageHopLimit - wsm->getHopCount());
        it->second.copyMessage = it->second.copyMessage + 1;
        it->second.sumHops += (beaconMessageHopLimit - wsm->getHopCount());

        if ((beaconMessageHopLimit - wsm->getHopCount()) > it->second.maxHop) {
            it->second.maxHop = (beaconMessageHopLimit - wsm->getHopCount());
        }
        if ((beaconMessageHopLimit - wsm->getHopCount()) < it->second.minHop) {
             it->second.minHop = (beaconMessageHopLimit - wsm->getHopCount());
         }

        it->second.sumTimeRecived += (simTime() - wsm->getTimestamp());
        it->second.times += ", ";
        simtime_t tmpTime = (simTime() - wsm->getTimestamp());
        it->second.times += tmpTime.str();

    } else{
        struct messages m;
        m.copyMessage = 1;
        m.wsmData = wsm->getWsmData();
        m.hops = to_string(beaconMessageHopLimit - wsm->getHopCount());
        m.maxHop = m.minHop = m.sumHops = (beaconMessageHopLimit - wsm->getHopCount());
        m.sumTimeRecived = (simTime() - wsm->getTimestamp());
        simtime_t tmpTime = (simTime() - wsm->getTimestamp());
        m.times = tmpTime.str();
        messagesReceived.insert(make_pair(wsm->getGlobalMessageIdentificaton(), m));
    }
}

WaveShortMessage* vehDist_rsu::prepareBeaconStatusWSM(std::string name, int lengthBits, t_channel channel, int priority, int serial) {
    WaveShortMessage* wsm = new WaveShortMessage(name.c_str());
    wsm->addBitLength(headerLength);
    wsm->addBitLength(lengthBits);
    switch (channel) {
        case type_SCH: wsm->setChannelNumber(Channels::SCH1); break; //will be rewritten at Mac1609_4 to actual Service Channel. This is just so no controlInfo is needed
        case type_CCH: wsm->setChannelNumber(Channels::CCH); break;
    }
    wsm->setPsid(0);
    wsm->setPriority(priority);
    wsm->setWsmVersion(1);
    wsm->setSerial(serial);
    wsm->setTimestamp(simTime());
    wsm->setSenderPos(curPosition);
    wsm->setSource(source.c_str());

    //beacon don't need
    //wsm->setRecipientAddressString(); => "BROADCAST"
    // wsm->setSenderAddressTemporary();
    //wsm->setTarget(); => "BROADCAST"

    DBG << "Creating BeaconStatus with Priority " << priority << " at Applayer at " << wsm->getTimestamp() << std::endl;
    return wsm;
}

void vehDist_rsu::handleSelfMsg(cMessage* msg) {
    switch (msg->getKind()) {
        case SEND_BEACON_EVT: {
            //sendWSM(prepareWSM("beacon", beaconLengthBits, type_CCH, beaconPriority, 0, -1));
            sendWSM(prepareBeaconStatusWSM("beaconStatus", beaconLengthBits, type_CCH, beaconPriority, -1));
            scheduleAt(simTime() + par("beaconInterval").doubleValue(), sendBeaconEvt);
            break;
        }
        default: {
            if (msg)
                DBG << "APP: Error: Got Self Message of unknown kind! Name: " << msg->getName() << endl;
            break;
        }
    }
}
void vehDist_rsu::printCountMessagesReceived() {
    myfile.open (fileMessagesCount, std::ios_base::app);

    SimTime avgGeneralCopyMessageReceived = 0;
    double avgGeneralHopsMessage = 0;
    SimTime avgGeneralTimeMessageReceived = 0;

    if (!messagesReceived.empty()) {
        myfile << "messagesReceived from " << findHost()->getFullName() << endl;

        map<string, struct messages>::iterator it;
        for (it = messagesReceived.begin(); it != messagesReceived.end(); it++) {
            myfile << endl;
            myfile << "Message ID: " << it->first << endl;
            myfile << "Count received: " << it->second.copyMessage << endl;
            avgGeneralCopyMessageReceived += it->second.copyMessage;
            myfile << it->second.wsmData << endl;
            myfile << "Hops: " << it->second.hops << endl;
            myfile << "Sum hops: " << it->second.sumHops << endl;
            myfile << "Average hops: " << (it->second.sumHops/it->second.copyMessage) << endl;
            avgGeneralHopsMessage += it->second.sumHops;
            myfile << "Max hop: " << it->second.maxHop << endl;
            myfile << "Min hop: " << it->second.minHop << endl;
            myfile << "Times: " << it->second.times << endl;
            myfile << "Sum times: " << it->second.sumTimeRecived << endl;
            myfile << "Average time to received: " << (it->second.sumTimeRecived/it->second.copyMessage) << endl;
            avgGeneralTimeMessageReceived += (it->second.sumTimeRecived/it->second.copyMessage);
        }

        avgGeneralTimeMessageReceived = avgGeneralTimeMessageReceived/messagesReceived.size();
        avgGeneralCopyMessageReceived = avgGeneralCopyMessageReceived/messagesReceived.size();
        avgGeneralHopsMessage = avgGeneralHopsMessage/messagesReceived.size();
        myfile << endl << "Exp: " << experimentNumber << " ### Count Messages Received: " << messagesReceived.size() << endl;
        myfile << "Exp: " << experimentNumber << " ### avg time to receive: " << avgGeneralTimeMessageReceived << endl;
        myfile << "Exp: " << experimentNumber << " ### avg copy received: " << avgGeneralCopyMessageReceived << endl;
        myfile << "Exp: " << experimentNumber << " ### avg hops to received: " << avgGeneralHopsMessage << endl;
        // TODO: 34 geradas, mas só 2* recebidas
        myfile << endl;
    } else{
        myfile << "messagesReceived from " << findHost()->getFullName() << " is empty now" << endl;
    }
    myfile.close();
}

void vehDist_rsu:: finish() {
    printCountMessagesReceived();
    if (experimentNumber == 8) { // maxSpeed 15 m/s
        system("sed -i 's/maxSpeed=\"..\"/maxSpeed=\"15\"/g' test_end.rou.xml");
    }
}

void vehDist_rsu::sendWSM(WaveShortMessage* wsm) {
    sendDelayedDown(wsm,individualOffset);
}

void vehDist_rsu::sendMessage(std::string blockedRoadId) {
    sentMessage = true;
    t_channel channel = dataOnSch ? type_SCH : type_CCH;
    WaveShortMessage* wsm = prepareWSM("data2rsu", dataLengthBits, channel, dataPriority, -1,2);
    wsm->setWsmData(blockedRoadId.c_str());
    sendWSM(wsm);
}
