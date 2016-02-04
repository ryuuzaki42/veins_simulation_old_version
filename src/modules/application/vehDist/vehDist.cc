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

#include "application/vehDist/vehDist.h"

using Veins::TraCIMobilityAccess;
using Veins::AnnotationManagerAccess;

const simsignalwrap_t vehDist::parkingStateChangedSignal = simsignalwrap_t(TRACI_SIGNAL_PARKING_CHANGE_NAME);

Define_Module(vehDist);

void vehDist::initialize(int stage) {
    BaseWaveApplLayer::initialize_default_veins_TraCI(stage);
    if (stage == 0) {
        traci = TraCIMobilityAccess().get(getParentModule());
        annotations = AnnotationManagerAccess().getIfExists();
        ASSERT(annotations);

        sentMessage = false;
        lastDroveAt = simTime();
        findHost()->subscribe(parkingStateChangedSignal, this);
        isParking = false;
        sendWhileParking = par("sendWhileParking").boolValue();

        vehInitializeVariables();
    }
}

void vehDist::vehInitializeVariables() {
    generalInitializeVariables_executionByExperimentNumber();

    source = findHost()->getFullName();

    setVehNumber();
    if(vehNumber == 0) { // Veh must be the first to generate messages, so your offset is 0;
        vehOffSet = 0;
        //initialize random seed (Seed the RNG) # Inside of IF because must be executed one time (seed for the rand is "static")
        srand(repeatNumber + 1); // Instead srand(time(NULL)) for make the experiment more reproducible. srand(0) == srand(1), so reapeatNumber +1
    } else {
        //simulate asynchronous channel access
        vehOffSet = dblrand(); // Values betwen 0 and 1
        vehOffSet = vehOffSet + 0.001;
    }
    numVehicles = par("numVehicles").longValue();

    stringTmp = ev.getConfig()->getConfigValue("sim-time-limit");
    timeLimitGenerateBeaconMessage = atof(stringTmp.c_str());
    doubleTmp = par("ttlBeaconMessage_two").doubleValue();
    timeLimitGenerateBeaconMessage = timeLimitGenerateBeaconMessage - doubleTmp;

    if (vehNumber == 0) {
       cout << endl << "Experiment: " << experimentNumber << endl;
       cout << "ttlBeaconMessage: " << ttlBeaconMessage  << endl;
       cout << "countGenerateBeaconMessage: " << countGenerateBeaconMessage << endl;
       cout << "timeLimitGenerateMessage: " << timeLimitGenerateBeaconMessage << endl << endl;
    }

    restartFilesResult(); // Start the file for save results
    vehGenerateBeaconMessageBegin(); // Create Evt to generate messages
    vehUpdatePosition(); // Create Evt to update the position of vehicle
    vehCreateEventTrySendBeaconMessage(); // Create one Evt to try send messages in buffer

    //cout << endl << findHost()->getFullName() << " entered in the scenario at " << simTime() << endl;
}

void vehDist::onBeaconStatus(WaveShortMessage* wsm) {
    //cout << "OnBeacon Id(veh): " << wsm->getVehicleId() << " timestamp: " << wsm->getTimestamp() << endl;
    removeOldestInput(&beaconStatusNeighbors, par("ttlBeaconStatus").doubleValue(), par("beaconStatusBufferSize").longValue());

    auto it = beaconStatusNeighbors.find(wsm->getSenderAddressTemporary());
    if (it != beaconStatusNeighbors.end()) {
        //cout << "Update wsm beacon (Key: " << wsm->getSenderAddressString() << ", timestamp: " << beaconNeighbors[wsm->getSenderAddressString()].getTimestamp() << ")" << endl;
        wsm->setTimestamp(simTime());
        it->second = *wsm;
    } else {
        beaconStatusNeighbors.insert(make_pair(wsm->getSenderAddressTemporary(), *wsm));
    }
    //cout << "veh: " << findHost()->getFullName() << "beaconNeighbors.size(): " << beaconNeighbors.size() << endl;
    //printBeaconNeighbors();

    sendMessageNeighborsTarget();
}

void vehDist::onBeaconMessage(WaveShortMessage* wsm) {
    //findHost()->bubble("Received a Message");
    removeOldestInput(&messagesBuffer, ttlBeaconMessage, par("beaconMessageBufferSize"));

    //verify if this is the recipient of the message
    if (strcmp(wsm->getRecipientAddressTemporary(), findHost()->getFullName()) == 0) {

        if (!messagesDelivered.empty()) {
            if (messagesDelivered.end() != find(messagesDelivered.begin(), messagesDelivered.end(), wsm->getGlobalMessageIdentificaton())) { // message has been delivered to the target before.
                cout << "This message has been delivered to the target before." << endl;
                return;
            }
        }
        cout << "Saving message from: " << wsm->getSenderAddressTemporary() << " to " << findHost()->getFullName() << endl;
        saveMessagesOnFile(wsm, fileMessagesNameUnicast);

        if (wsm->getHopCount() == 0) {
            insertMessageDrop(wsm->getGlobalMessageIdentificaton(), 3); // by ttl (1 buffer, 2 ttl, 3 hop
            return;
        }

        if (messagesBuffer.empty() || messagesBuffer.find(wsm->getGlobalMessageIdentificaton()) == messagesBuffer.end()) { //verify if the message isn't in the buffer
            //cout << findHost()->getFullName() << " buffer empty or the vehicle don't have the message at simtime: " << simTime() << endl;
            messagesBuffer.insert(make_pair(wsm->getGlobalMessageIdentificaton(),*wsm)); //add the msg in the  vehicle buffer
            colorCarryMessage();

            //cout << findHost()->getFullName() << " received the message " << endl;
            //cout << "from (source): " << wsm->getSource() << endl;
            //cout << " sender: " << wsm->getSenderAddressString() << endl;
            //cout << " hopCount: " << wsm->getHopCount() << endl;
            //cout << " At: " << simTime() << endl;
        } else {
            cout<<findHost()->getFullName() << " message is on the buffer at simtime " << simTime() << endl;
        }
    } //else { // to another veh
        //cout << "Saving broadcast message from: " << wsm->getSenderAddressString() << " to " << findHost()->getFullName() << endl;
        //saveMessagesOnFile(wsm, fileMessagesNameBroadcast);
    //}
    //printmessagesBuffer();
}

void vehDist::colorCarryMessage() {
    if (messagesBuffer.empty()) {
        findHost()->getDisplayString().updateWith("r=0,green");
    } else{
        findHost()->getDisplayString().updateWith("r=12,green");
    }
}

void vehDist::removeOldestInput(unordered_map<string, WaveShortMessage>* data, double timeValid, unsigned int bufferLimit) {
    //cout << "veh: " << findHost()->getFullName() << " Buffercount: " << data->size() << endl;
    if (!data->empty()) {
        unordered_map<string, WaveShortMessage>::iterator itData;
        itData = data->begin();
        string type = itData->second.getName();
        simtime_t minTime = itData->second.getTimestamp();
        string key = itData->first;
        itData++;
        for (; itData != data->end(); itData++) {
            if (minTime > itData->second.getTimestamp()) {
                minTime = itData->second.getTimestamp();
                key = itData->first;
            }
        }

        if((minTime + timeValid) < simTime()) {
            //cout << findHost()->getFullName() << " remove one " << type << " by time Timestamp: " << minTime << " at: " << simTime() << " timeValid: " << timeValid << endl;
            if (strcmp(type.c_str(), "beaconMessage") == 0) {
                insertMessageDrop(key, 2); // by ttl (1 buffer, 2 ttl, 3 hop
            }

            data->erase(key);
        } else if (data->size() > bufferLimit) {
            //cout << findHost()->getFullName() << " remove one " << type << " by space Timestamp: " << minTime << " at: " << simTime() << " timeValid: " << timeValid << endl;
            if (strcmp(type.c_str(), "beaconMessage") == 0) {
                insertMessageDrop(key, 1); // by ttl (1 buffer, 2 ttl, 3 hop
            }
            data->erase(key);
        }

    } //else{
        //cout << "Data buffer from " << findHost()->getFullName() << " is empty now " << endl;
    //}
    colorCarryMessage();
}

void vehDist::insertMessageDrop(string ID, int type) {
    struct messagesDropStruct mD_tmp;
    mD_tmp.byBuffer = 0;
    mD_tmp.byHop = 0;
    mD_tmp.byTime = 0;

    if (type == 1) { // by buffer limit
        mD_tmp.byBuffer = 1;
    } else if (type == 2) { // by hop limit
        mD_tmp.byHop = 1;
    } else if (type == 3) { // by ttl
        mD_tmp.byTime = 1;
    }

    if(messagesDrop.empty() || (messagesDrop.find(ID) == messagesDrop.end())) {
        messagesDrop.insert(make_pair(ID, mD_tmp));
    } else if (type == 1) {
        messagesDrop[ID].byBuffer += 1;
    } else if (type == 2) {
        messagesDrop[ID].byHop += 1;
    } else if (type == 3) {
        messagesDrop[ID].byTime += 1;
    }
}

void vehDist::sendBeaconMessage() {
    removeOldestInput(&messagesBuffer, ttlBeaconMessage, par("beaconMessageBufferSize").longValue());
    removeOldestInput(&beaconStatusNeighbors, par("ttlBeaconStatus").doubleValue(), par("beaconStatusBufferSize").longValue());

    if (!messagesBuffer.empty()) {
        cout << findHost()->getFullName() << " messagesBuffer with message(s) at " << simTime() << endl;

        if (beaconStatusNeighbors.empty()) {
            cout << "beaconNeighbors on sendDataMessage from " << findHost()->getFullName() << " is empty now " << endl;
        } else{
            //cout << "Before sendBeaconNeighborsTarget messagesBuffer.size(): " << messagesBuffer.size() << endl;
            sendMessageNeighborsTarget();
            if (messagesBuffer.empty()) {
               return;
            }
            //cout << "After sendBeaconNeighborsTarget messagesBuffer.size(): " << messagesBuffer.size() << endl;

            // #to_do Procurar por um atigo em que afirme que enviar a última messagem recebeida é a melhor
            string key = returnLastMessageInsert();
            bool send = false;

            unordered_map<string, WaveShortMessage>::iterator itBeacon;
            for (itBeacon = beaconStatusNeighbors.begin(); itBeacon != beaconStatusNeighbors.end(); itBeacon++) {
                //test send distante e heading to target
                //cout => car[x] distance <is more, is less, not change> to [target](rsu[0]) by car[y]
                cout << findHost()->getFullName() << " distance";
                send = sendtoTargetbyVeh(itBeacon->second.getSenderPosPrevious(), itBeacon->second.getSenderPos(), itBeacon->second.getHeading(), messagesBuffer[key].getTargetPos());
                cout << "to " << messagesBuffer[key].getTarget() << " by " << itBeacon->second.getSenderAddressTemporary() << endl;

                cout << endl;
                cout << " SenderPosBack: " << itBeacon->second.getSenderPosPrevious() << endl;
                cout << " SenderPos: " << itBeacon->second.getSenderPos() << endl;
                cout << " Heading: " << itBeacon->second.getHeading() << endl;
                cout << " Message id: " << key << endl;
                cout << " TargetPos: " << messagesBuffer[key].getTargetPos() << endl;
                cout << endl;
                if (send) {
                    break;
                }
            }

            if (send) {
                string rcvId = itBeacon->first;
                sendWSM(updateBeaconMessageWSM(messagesBuffer[key].dup(), rcvId));

                cout << findHost()->getFullName() << " send message to " << rcvId << " at "<< simTime() << endl;
                cout << " MessageID: " << messagesBuffer[key].getGlobalMessageIdentificaton() << endl;
                cout << " Source: " << messagesBuffer[key].getSource() << endl;
                cout << " Message Content: " << messagesBuffer[key].getWsmData() << endl;
                cout << " Target: " << messagesBuffer[key].getTarget() << endl;
                cout << " Timestamp: " << messagesBuffer[key].getTimestamp() << endl;
                cout << " HopCount: " << (messagesBuffer[key].getHopCount() - 1) << endl;
            } else {
                cout << endl << "Not send message to" << endl;
            }
            //printMessagesBuffer();
        }
    }// else {
        //cout << findHost()->getFullName() << " messagesBuffer is empty at " << simTime() << endl;
        //return;
    //}
}

void vehDist:: finish() {
    printCountBeaconMessagesDrop();
}

// mudar criar outra função que imprime depois dessa
void vehDist::printCountBeaconMessagesDrop() {
    myfile.open (fileMessagesDrop, std::ios_base::app);

    if (messagesDrop.empty()) {
        myfile << "messagesDrop from " << findHost()->getFullName() << " is empty now" << endl;
        return;
    } else{
        myfile << "messagesDrop from " << findHost()->getFullName() << endl;

        map<string, struct messagesDropStruct>::iterator it;
        for (it = messagesDrop.begin(); it != messagesDrop.end(); it++) {
            myfile << "Message Id: " << it->first << endl;
            myfile << "By Buffer: " << it->second.byBuffer << endl;
            myfile << "By Hop: " << it->second.byHop << endl;
            myfile << "By Time: " << it->second.byTime << endl;
            countMesssageDrop += it->second.byBuffer + it->second.byHop + it->second.byTime;
        }
    }
    myfile << endl << "## Count messages drop: " << countMesssageDrop << endl << endl;
    myfile.close();
}

string vehDist::returnLastMessageInsert() {
    unordered_map<string, WaveShortMessage>::iterator itMessage;
    itMessage = messagesBuffer.begin();
    simtime_t minTime;
    minTime = itMessage->second.getTimestamp();
    string key = itMessage->first;
    itMessage++;
    for (; itMessage != messagesBuffer.end(); itMessage++) {
        if (minTime > itMessage->second.getTimestamp()) {
            minTime = itMessage->second.getTimestamp();
            key = itMessage->first;
        }
    }
    return key;
}

void vehDist::sendMessageNeighborsTarget() {
    unordered_map<string, WaveShortMessage>::iterator itMessage;
    vector<string> messageToRemove;

    for (itMessage = messagesBuffer.begin(); itMessage != messagesBuffer.end(); itMessage++) {
        unordered_map<string, WaveShortMessage>::iterator itBeacon;
        for (itBeacon = beaconStatusNeighbors.begin(); itBeacon != beaconStatusNeighbors.end(); itBeacon++) {
            if (strcmp(itMessage->second.getTarget(), itBeacon->second.getSenderAddressTemporary()) == 0) {
                string rcvId = itMessage->second.getTarget();
                cout << "Send message to: " << rcvId << endl;
                sendWSM(updateBeaconMessageWSM(itMessage->second.dup(), rcvId));
                messageToRemove.push_back(itMessage->second.getGlobalMessageIdentificaton());

                messagesDelivered.push_back(itMessage->second.getGlobalMessageIdentificaton());
            }
        }
    }
    vector<string>::iterator itVector;
    for (itVector = messageToRemove.begin(); itVector != messageToRemove.end(); ++itVector) {
        cout << "send and remove message" << endl;
        messagesBuffer.erase(*itVector);
    }
    colorCarryMessage();
    //messageToRemove.clear();
}

void vehDist::handleLowerMsg(cMessage* msg) {
    WaveShortMessage* wsm = dynamic_cast<WaveShortMessage*>(msg);
    ASSERT(wsm);

    if (std::string(wsm->getName()) == "beacon") {
        onBeacon(wsm);
    }
    else if (std::string(wsm->getName()) == "data") {
        onData(wsm);
    }
//###############################################################
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
//###############################################################
    else{
        DBG << "unknown message (" << wsm->getName() << ")  received\n";
    }
    delete(msg);
}

int vehDist::getVehCategory() {
    // ver como definir a categoria
    if (vehNumber < 13) {
        return 1;
    } else if ((vehNumber >= 13) && (vehNumber < 26)) {
        return 2;
    } else if ((vehNumber >= 26) && (vehNumber < 39)) {
        return 3;
    } else if ((vehNumber >= 39) && (vehNumber < 52)) {
        return 4;
    } else{
        return 5;
    }
}

bool vehDist::sendtoTargetbyVeh(Coord vehicleRemoteCoordBack, Coord vehicleRemoteCoordNow, int vehicleRemoteHeading, Coord targetCoord) {
  /*cout << endl;
    cout <<  "targetCoord.x: " << targetCoord.x << endl;
    cout <<  "targetCoord.y: " << targetCoord.y << endl;
    cout <<  "vehicleRemoteCoordBack.x: " << vehicleRemoteCoordBack.x << endl;
    cout <<  "vehicleRemoteCoordBack.y: " << vehicleRemoteCoordBack.y << endl;
    cout <<  "vehicleRemoteCoordNow.x: " << vehicleRemoteCoordNow.x << endl;
    cout <<  "vehicleRemoteCoord.y: " << vehicleRemoteCoordNow.y << endl;
    cout <<  "vehicleRemoteHeading: " << vehicleRemoteHeading << endl; */

   /*(angle >= 337.5 && angle < 360) || (angle >= 0 && angle < 22.5) return 1; // L or E => 0º
     angle >= 22.5 && angle < 67.5                                   return 2; // NE => 45º
     angle >= 67.5  && angle < 112,5                                 return 3; // N => 90º
     angle >= 112.5  && angle < 157.5                                return 4; // NO => 135º
     angle >= 157,5  && angle < 202,5                                return 5; // O or W => 180º
     angle >= 202.5  && angle < 247.5                                return 6; // SO => 225º
     angle >= 292.5  && angle < 337.5                                return 8; // SE => 315º
     angle >= 360 return 9; // Error */

     double distanceBefore = traci->commandDistanceRequest(vehicleRemoteCoordBack, targetCoord, false);
     double distanceNow = traci->commandDistanceRequest(vehicleRemoteCoordNow, targetCoord, false);

     if(distanceNow < distanceBefore) {
         cout << " is less ";
         return true;
     }else if(distanceNow == distanceBefore) {
         cout << " not change ";
     }else{
         cout << " is more ";
     }
     return false;
}

//###################################################  OK Function ####################################################

void vehDist::saveVehStartPosition(string fileNameLocation) {
    fileNameLocation += "vehicle_position_initialize.r";
    if (source.compare("car[0]") == 0) {
        if (repeatNumber == 0) {
            myfile.open (fileNameLocation);
        } else{
            myfile.open (fileNameLocation, std::ios_base::app);
        }
        printHeaderfileExecution(ttlBeaconMessage, countGenerateBeaconMessage);

        myfile << "Start Position Vehicles" << endl;
    } else{
        myfile.open (fileNameLocation, std::ios_base::app);
    }
    myfile << findHost()->getFullName() << ": "<< traci->getCurrentPosition() << endl;
    myfile.close();
}

void vehDist::setVehNumber() {
    string vehNumberString = (source.substr(source.find("[") + 1, source.find("]") - source.find("[") - 1));
    vehNumber = atoi(vehNumberString.c_str());
    //cout << findHost()->getFullName() << " vehID: " << vehNumber << endl;
}

void vehDist::restartFilesResult() {
    stringTmp = "results/resultsEnd/E" + to_string(experimentNumber);
    stringTmp += "_" + to_string(ttlBeaconMessage) + "_" + to_string(countGenerateBeaconMessage) +"/";

    saveVehStartPosition(stringTmp); // Save the start position of vehicle. Just for test the seed.

    fileMessagesNameUnicast = fileMessagesCount =  fileMessagesDrop = fileMessagesGenerated = fileMessagesNameBroadcast = stringTmp;

    //fileMessagesNameBroadcast += "VehBroadcastMessages.r";
    fileMessagesNameUnicast += "VehMessages.r";
    fileMessagesCount += "VehMessagesCount.r";
    fileMessagesDrop += "VehMessagesDrop.r";
    fileMessagesGenerated += "VehMessagesgenerated.r";

    if (vehNumber == 0) {
        if (repeatNumber == 0) {
            //openFileAndClose(fileMessagesNameBroadcast, false, ttlBeaconMessage, countGenerateBeaconMessage);
            openFileAndClose(fileMessagesNameUnicast, false, ttlBeaconMessage, countGenerateBeaconMessage);
            openFileAndClose(fileMessagesCount, false, ttlBeaconMessage, countGenerateBeaconMessage);
            openFileAndClose(fileMessagesDrop, false, ttlBeaconMessage, countGenerateBeaconMessage);
            openFileAndClose(fileMessagesGenerated, false, ttlBeaconMessage, countGenerateBeaconMessage);
        } else{ // (repeatNumber != 0)) // open just for append
            //openFileAndClose(fileMessagesNameBroadcast, true, ttlBeaconMessage, countGenerateBeaconMessage);
            openFileAndClose(fileMessagesNameUnicast, true, ttlBeaconMessage, countGenerateBeaconMessage);
            openFileAndClose(fileMessagesCount, true, ttlBeaconMessage, countGenerateBeaconMessage);
            openFileAndClose(fileMessagesDrop, true, ttlBeaconMessage, countGenerateBeaconMessage);
            openFileAndClose(fileMessagesGenerated, true, ttlBeaconMessage, countGenerateBeaconMessage);
        }
    }
}

void vehDist::vehUpdatePosition() {
    vehPositionPrevious = traci->getCurrentPosition();
    //cout << "initial positionBack: " << vehPositionBack << endl;
    sendUpdatePosisitonVeh = new cMessage("Event update position vehicle", SendEvtUpdatePositionVeh);

    //cout << findHost()->getFullName() << " at simtime: "<< simTime() << "schedule created UpdatePosition to: "<< (simTime() + vehOffSet) << endl;
    scheduleAt((simTime()+ par("vehTimeUpdatePosition").doubleValue() + vehOffSet), sendUpdatePosisitonVeh);
}

void vehDist::vehCreateEventTrySendBeaconMessage() {
    if (sendData) {
        sendBeaconMessageEvt = new cMessage("Event send beacon message", SendEvtBeaconMessage);
        //cout << findHost()->getFullName() << " at simtime: "<< simTime() << "schedule created sendData to: "<< (simTime() + vehOffSet) << endl;
        scheduleAt((simTime() + vehOffSet), sendBeaconMessageEvt);
    }
}

void vehDist::selectVehGenerateMessage() {
    if (vehNumber == 0) { // if true, some veh has (in past) selected the veh to generate messages
        if (simTime() <= timeLimitGenerateBeaconMessage) {
            int vehSelected;
            myfile.open(fileMessagesGenerated, std::ios_base::app); // To save infos (Id and veh generate) on fileMessagesGenerated
            for (int i = 0; i < countGenerateBeaconMessage;) { // select <countGenerateBeaconMessage> distinct veh to generate messages
                vehSelected = rand() % numVehicles; // random car to generate message
                if(vehDist::vehGenerateMessage.find(vehSelected) == vehDist::vehGenerateMessage.end()) {
                    vehDist::vehGenerateMessage.insert(make_pair(vehSelected,true));
                    cout << findHost()->getFullName() << " select the car[" << vehSelected << "] generate message at simTime: " << simTime() << endl;
                    myfile << findHost()->getFullName() << " select the car[" << vehSelected << "] to generated at simTime: " << simTime() << endl;
                    i++;
                }
            }
            myfile.close();
        }
    }
}

void vehDist::vehGenerateBeaconMessageBegin() {
    if (sendData) {
        sendGenerateBeaconMessageEvt = new cMessage("Event generate beacon message", SendEvtGenerateBeaconMessage);
        //cout << findHost()->getFullName() << " at simtime: "<< simTime() << "schedule created sendGenerateMessageEvt to: "<< (simTime() + vehOffSet) << endl;
        scheduleAt((simTime() + vehOffSet), sendGenerateBeaconMessageEvt);
    }
}

void vehDist::vehGenerateBeaconMessageAfterBegin() {
    selectVehGenerateMessage();

    if(vehDist::vehGenerateMessage.find(vehNumber) != vehDist::vehGenerateMessage.end()) { // if have vehDist::vehGenerateMessage[vehNumber], will generate one message
        generateBeaconMessage();
        vehDist::vehGenerateMessage.erase(vehNumber);
    }
}

WaveShortMessage* vehDist::prepareBeaconStatusWSM(std::string name, int lengthBits, t_channel channel, int priority, unsigned int rcvId, int serial) {
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
    wsm->setSenderAddressTemporary(findHost()->getFullName());

    //beacon don't need
    //wsm->setRecipientAddressString(); => "BROADCAST"
    //wsm->setSource(source);
    //wsm->setTarget(target);

    wsm->setSenderPos(curPosition);
    wsm->setRoadId(traci->getRoadId().c_str());
    wsm->setSenderSpeed(traci->getSpeed());

    wsm->setCategory(getVehCategory());
    wsm->setSenderPosPrevious(vehPositionPrevious);

    // heading 1 to 4 or 1 to 8
    wsm->setHeading(getVehHeading4());
    //wsm->setHeading(getVehHeading8());

    DBG << "Creating BeaconStatus with Priority " << priority << " at Applayer at " << wsm->getTimestamp() << std::endl;
    return wsm;
}

WaveShortMessage* vehDist::updateBeaconMessageWSM(WaveShortMessage* wsm, string rcvId) {
    wsm->setSenderAddressTemporary(findHost()->getFullName());
    wsm->setRecipientAddressTemporary(rcvId.c_str());
    wsm->setSenderPos(curPosition);
    wsm->setRoadId(traci->getRoadId().c_str());
    wsm->setCategory(getVehCategory());
    wsm->setSenderSpeed(traci->getSpeed());
    wsm->setSenderPosPrevious(vehPositionPrevious);
    wsm->setHeading(getVehHeading4());
    wsm->setHopCount(wsm->getHopCount() -1);
    return wsm;
}

//Generate a target in order to send a message
void vehDist::generateTarget() {
    //Set the target node to whom my message has to be delivered
    target = "rsu[0]";
    target_x = par("vehBeaconMessageTarget_x").longValue();
    target_y = par("vehBeaconMessageTarget_y").longValue();
}

void vehDist::generateBeaconMessage() {
    WaveShortMessage* wsm = new WaveShortMessage("beaconMessage");

    //target = rsu[0], rsu[1] or car[*] and the repective position.
    generateTarget();
    wsm->setTargetPos(Coord(target_x, target_y, 3));

    wsm->addBitLength(headerLength);
    wsm->addBitLength(dataLengthBits);
    t_channel channel = dataOnSch ? type_SCH : type_CCH;
    switch (channel) {
        case type_SCH: wsm->setChannelNumber(Channels::SCH1); break; //will be rewritten at Mac1609_4 to actual Service Channel. This is just so no controlInfo is needed
        case type_CCH: wsm->setChannelNumber(Channels::CCH); break;
    }
    wsm->setPsid(0);
    wsm->setPriority(dataPriority);
    wsm->setWsmVersion(1);
    wsm->setSenderPos(curPosition);
    wsm->setSerial(2);
    //wsm->setMessageTimestampGenerate(simTime());
    //wsm->setTimestamp(simTime() + par("beaconMessageInterval").doubleValue());
    wsm->setTimestamp(simTime());

    wsm->setSenderAddressTemporary(findHost()->getFullName());
    // defined before send
    //wsm->setRecipientAddress();
    wsm->setRecipientAddressTemporary("Initial");
    wsm->setSource(source.c_str());
    wsm->setTarget(target.c_str());

    string data = "WSMData generated by ";
    data += findHost()->getFullName();
    wsm->setWsmData(data.c_str());
    wsm->setGlobalMessageIdentificaton(to_string(vehDist::beaconMessageId).c_str());
    vehDist::beaconMessageId++;
    wsm->setHopCount(beaconMessageHopLimit+1); // Is beaconMessageHopLimit+1 because hops count is equals to routes in the path, not hops.

    // Adding the message on the buffer
    messagesBuffer.insert(make_pair(wsm->getGlobalMessageIdentificaton(),*wsm));
    colorCarryMessage();

    cout << findHost()->getFullName() << " generated the message ID:" << vehDist::beaconMessageId << " at simTime: " << simTime() << endl;

    // Save info (Id and veh generate) on fileMessagesGenerated
    myfile.open(fileMessagesGenerated, std::ios_base::app);
    myfile << "                                                   ";
    myfile << "                                                   ";
    myfile << "### " << findHost()->getFullName() << " generated the message ID:" << vehDist::beaconMessageId << " at simTime: " << simTime() << endl;
    myfile.close();
}

void vehDist::handleSelfMsg(cMessage* msg) {
    switch (msg->getKind()) {
        case SEND_BEACON_EVT: {
            //sendWSM(prepareWSM("beacon", beaconLengthBits, type_CCH, beaconPriority, 0, -1));
            sendWSM(prepareBeaconStatusWSM("beaconStatus", beaconLengthBits, type_CCH, beaconPriority, 0, -1));
            scheduleAt((simTime() + par("beaconInterval").doubleValue()), sendBeaconEvt);
            break;
        }
        case SendEvtBeaconMessage: {
            sendBeaconMessage();
            //cout << "create schedule for send_data_evt" << endl;
            scheduleAt((simTime() + par("beaconMessageInterval").doubleValue()), sendBeaconMessageEvt);
            break;
        }
        case SendEvtUpdatePositionVeh: {
            updateVehPosition();
            scheduleAt((simTime() + par("vehTimeUpdatePosition").doubleValue()), sendUpdatePosisitonVeh);
            break;
        }
        case SendEvtGenerateBeaconMessage: {
            vehGenerateBeaconMessageAfterBegin();
            scheduleAt((simTime() + par("timeGenerateBeaconMessage").doubleValue()), sendGenerateBeaconMessageEvt);
            break;
        }
        default: {
            if (msg)
                DBG << "APP: Error: Got Self Message of unknown kind! Name: " << msg->getName() << endl;
            break;
        }
    }
}

void vehDist::printMessagesBuffer() {
    if (messagesBuffer.empty()) {
        //cout << "messagesBuffer from " << findHost()->getFullName() << " is empty now: " << simTime() << endl;
        return;
    } else {
        cout << endl <<"messagesBuffer from " << findHost()->getFullName() << " at " << simTime() << endl;
        unordered_map<string, WaveShortMessage>::iterator it;
        for (it = messagesBuffer.begin(); it != messagesBuffer.end(); it++) {
            cout << " Id: " << it->second.getGlobalMessageIdentificaton()<< endl;
            cout << " Message Content: " << it->second.getWsmData() << endl;
            cout << " Source: " << it->second.getSource() << endl;
            cout << " Target: " << it->second.getTarget() << endl;
            cout << " Timestamp: " << it->second.getTimestamp() << endl;
            cout << " HopCount: " << it->second.getHopCount() << endl;
            cout << endl;
        }
    }
}

void vehDist::printBeaconStatusNeighbors() {
    if (!beaconStatusNeighbors.empty()) {
        cout << endl << "beaconNeighbors from " << findHost()->getFullName() << " at " << simTime() << endl;
        unordered_map<string, WaveShortMessage>::iterator it;
        for (it = beaconStatusNeighbors.begin(); it != beaconStatusNeighbors.end(); it++) {
            cout << " Id(veh): " << it->first << endl;
            cout << " PositionBack: " << it->second.getSenderPosPrevious() << endl;
            cout << " Position: " << it->second.getSenderPos() << endl;
            cout << " Speed: " << it->second.getSenderSpeed() << endl;
            cout << " Category: " << it->second.getCategory() << endl;
            cout << " RoadId: " << it->second.getRoadId() << endl;
            cout << " Heading: " << it->second.getHeading() << endl;
            cout << " Timestamp: " << it->second.getTimestamp()<< endl;
            cout << endl;
        }
    }
    else {
        //cout << "beaconNeighbors from " << findHost()->getFullName() << " is empty now: " << simTime() << endl;
    }
}

unsigned int vehDist::getVehHeading4() {
  /* return angle <0º - 359º>
     marcospaiva.com.br/images/rosa_dos_ventos%2002.GIF
     marcospaiva.com.br/localizacao.htm
     (angle >= 315 && angle < 360) || (angle >= 0 && angle < 45)     return 1; // L or E => 0º
     angle >= 45 && angle < 135                                      return 2; // N => 90º
     angle >= 135  && angle < 225                                    return 3; // O or W => 180º
     angle >= 225  && angle < 315                                    return 4; // S => 270º
     angle >= 360                                                    return 9; // Error */

    double angle;
    if (traci->getAngleRad() < 0) // radians are negtive, so degrees negative
        angle = (((traci->getAngleRad() + 2*M_PI ) * 180)/ M_PI);
    else //radians are positive, so degrees positive
        angle = ((traci->getAngleRad() * 180) / M_PI);

    if ((angle >= 315 && angle < 360) || (angle >= 0 && angle < 45)) {
        return 1; // L or E => 0º
    }
    else if (angle >= 45 && angle < 135) {
        return 2; // N => 90º
    }
    else if (angle >= 135  && angle < 225) {
        return 3; // O or W => 180º
    }
    else if (angle >= 225  && angle < 315) {
        return 4; // S => 270º
    }
    else {
        return 9; // Error
    }
}

unsigned int vehDist::getVehHeading8() {
  /* return angle <0º - 359º>
     marcospaiva.com.br/images/rosa_dos_ventos%2002.GIF
     marcospaiva.com.br/localizacao.htm
     (angle >= 337.5 && angle < 360) || (angle >= 0 && angle < 22.5) return 1; // L or E => 0º
     angle >= 22.5 && angle < 67.5                                   return 2; // NE => 45º
     angle >= 67.5  && angle < 112,5                                 return 3; // N => 90º
     angle >= 112.5  && angle < 157.5                                return 4; // NO => 135º
     angle >= 157,5  && angle < 202,5                                return 5; // O or W => 180º
     angle >= 247.5  && angle < 292.5                                return 7; // S => 270º
     angle >= 202.5  && angle < 247.5                                return 6; // SO => 225º
     angle >= 292.5  && angle < 337.5                                return 8; // SE => 315º
     angle >= 360                                                    return 9; // Error */

    double angle;
    if (traci->getAngleRad() < 0) // radians are negtive, so degrees negative
        angle = (((traci->getAngleRad() + 2*M_PI ) * 180)/ M_PI);
    else //radians are positive, so degrees positive
        angle = ((traci->getAngleRad() * 180) / M_PI);

    if ((angle >= 337.5 && angle < 360) || (angle >= 0 && angle < 22.5)) {
        return 1; // L or E => 0º
    }
    else if (angle >= 22.5 && angle < 67.5) {
        return 2; // NE => 45º
    }
    else if (angle >= 67.5  && angle < 112.5) {
        return 3; // N => 90º
    }
    else if (angle >= 112.5  && angle < 157.5) {
        return 4; // NO => 135º
    }
    else if (angle >= 157.5  && angle < 202.5) {
        return 5; // O or W => 180º
    }
    else if (angle >= 202.5  && angle < 247.5) {
        return 6; // SO => 225º
    }
    else if (angle >= 247.5  && angle < 292.5) {
        return 7; // S => 270º
    }
    else if (angle >= 292.5  && angle < 337.5) {
        return 8; // SE => 315º
    }
    else {
        return 9; // Error
    }
}

void vehDist::updateVehPosition() {
//    cout << "Vehicle: " << findHost()->getFullName();
//    cout << " Update position: " <<  "time: "<< simTime() << " next update: ";
//    cout << (simTime() + par("vehTimeUpdatePosition").doubleValue()) << endl;
//    cout << "positionBack: " << vehPositionBack;
    vehPositionPrevious = traci->getPositionAt(simTime() - par("vehTimeUpdatePosition").doubleValue());
//    cout << " and update positionBack: " << vehPositionBack << endl << endl;
}

//########################################################  Default Function #############################################

void vehDist::sendWSM(WaveShortMessage* wsm) {
    if (isParking && !sendWhileParking)
        return;
    sendDelayedDown(wsm,individualOffset);
}

void vehDist::receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj) {
    Enter_Method_Silent();
    if (signalID == mobilityStateChangedSignal) {
        handlePositionUpdate(obj);
    } else if (signalID == parkingStateChangedSignal) {
        handleParkingUpdate(obj);
    }
}

void vehDist::handleParkingUpdate(cObject* obj) {
    isParking = traci->getParkingState();
    if (sendWhileParking == false) {
        if (isParking == true) {
            (FindModule<BaseConnectionManager*>::findGlobalModule())->unregisterNic(this->getParentModule()->getSubmodule("nic"));
        } else {
            Coord pos = traci->getCurrentPosition();
            (FindModule<BaseConnectionManager*>::findGlobalModule())->registerNic(this->getParentModule()->getSubmodule("nic"), (ChannelAccess*) this->getParentModule()->getSubmodule("nic")->getSubmodule("phy80211p"), &pos);
        }
    }
}

void vehDist::handlePositionUpdate(cObject* obj) {
    BaseWaveApplLayer::handlePositionUpdate(obj);

    // stopped for for at least 10s?
    if (traci->getSpeed() < 1) {
        if (simTime() - lastDroveAt >= 10) {
            //findHost()->getDisplayString().updateWith("r=16,red");
            //if (!sentMessage)
            //    sendMessage(traci->getRoadId());
        }
    } else {
        lastDroveAt = simTime();
    }
}

void vehDist::sendMessage(std::string blockedRoadId) {
    sentMessage = true;
    t_channel channel = dataOnSch ? type_SCH : type_CCH;
    WaveShortMessage* wsm = prepareWSM("data2veh", dataLengthBits, channel, dataPriority, -1,2);
    wsm->setWsmData(blockedRoadId.c_str());
    sendWSM(wsm);
}

void vehDist::onData(WaveShortMessage* wsm) {
}

void vehDist::onBeacon(WaveShortMessage* wsm) {
}
