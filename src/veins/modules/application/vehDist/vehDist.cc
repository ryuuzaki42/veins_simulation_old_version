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

#include "veins/modules/application/vehDist/vehDist.h"

using Veins::TraCIMobilityAccess;
using Veins::AnnotationManagerAccess;

const simsignalwrap_t vehDist::parkingStateChangedSignal = simsignalwrap_t(TRACI_SIGNAL_PARKING_CHANGE_NAME);

Define_Module(vehDist);

void vehDist::initialize(int stage) {
    BaseWaveApplLayer::initialize_default_veins_TraCI(stage);
    if (stage == 0) { // traci - mobility, traci->getComand - traci, new traciVehice
        mobility = TraCIMobilityAccess().get(getParentModule());
        traci = mobility->getCommandInterface();
        traciVehicle = mobility->getVehicleCommandInterface();
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

    timeLimitGenerateBeaconMessage = atof(ev.getConfig()->getConfigValue("sim-time-limit"));
    float floatTmp = par("ttlBeaconMessage_two").doubleValue();
    timeLimitGenerateBeaconMessage = timeLimitGenerateBeaconMessage - floatTmp;
    beaconMessageBufferSize = par("beaconMessageBufferSize");
    beaconStatusBufferSize = par("beaconStatusBufferSize");
    ttlBeaconStatus = static_cast<float>(par("ttlBeaconStatus").doubleValue());
    vehCategory = traciVehicle->getTypeId();

    if(myId == 0) { // Veh must be the first to generate messages, so your offset is 0;
        vehOffSet = 0;
        //initialize random seed (Seed the RNG) # Inside of IF because must be executed one time (seed for the rand is "static")
        srand((repeatNumber + 1)); // Instead srand(time(NULL)) for make the experiment more reproducible. srand(0) == srand(1), so reapeatNumber +1
    } else {
        vehOffSet = static_cast<float>(dblrand()/10); //simulate asynchronous channel access. Values between 0 and 1
        vehOffSet += 0.1f;
    }
    if (myId == 0) {
        cout << endl << "Experiment: " << experimentNumber << endl;
        cout << "ttlBeaconMessage: " << ttlBeaconMessage  << endl;
        cout << "countGenerateBeaconMessage: " << countGenerateBeaconMessage << endl;
        cout << "timeLimitGenerateMessage: " << timeLimitGenerateBeaconMessage << endl << endl;
    }
    vehDist::numVehicles.push_back(source);

    restartFilesResult(); // Start the file for save results
    vehCreateUpdateTimeToSendEvent();
    vehGenerateBeaconMessageBegin(); // Create Evt to generate messages
    vehUpdatePosition(); // Create Evt to update the position of vehicle
    vehCreateEventTrySendBeaconMessage(); // Create one Evt to try send messages in buffer
    //cout << endl << findHost()->getFullName() << " entered in the scenario at " << simTime() << endl;
}

void vehDist::onBeaconStatus(WaveShortMessage* wsm) {
    auto it = beaconStatusNeighbors.find(wsm->getSource());
    if (it != beaconStatusNeighbors.end()) { // Update the beaconStatus
        it->second = *wsm;
    } else {
        if (beaconStatusNeighbors.size() >= beaconStatusBufferSize){
            removeOldestInputBeaconStatus();
        }
        beaconStatusNeighbors.insert(make_pair(wsm->getSource(), *wsm));
        sendMessageNeighborsTarget(wsm->getSource()); // Look in buffer it has messages for this new vehNeighbor
    }
    //printBeaconNeighbors();
}

void vehDist::onBeaconMessage(WaveShortMessage* wsm) {
    if (strcmp(wsm->getRecipientAddressTemporary(), findHost()->getFullName()) == 0) { //verify if this is the recipient of the message
        if (!messagesDelivered.empty()) {
            if (messagesDelivered.end() != find(messagesDelivered.begin(), messagesDelivered.end(), wsm->getGlobalMessageIdentificaton())) { // message has been delivered to the target before.
                cout << "This message has been delivered to the target before." << endl;
                return;
            }
        }

        cout << "Saving message from: " << wsm->getSenderAddressTemporary() << " to " << findHost()->getFullName() << endl;
        saveMessagesOnFile(wsm, fileMessagesUnicast);

        if (wsm->getHopCount() == 0) {
            insertMessageDrop(wsm->getGlobalMessageIdentificaton(), 3); // by ttl (1 buffer, 2 ttl, 3 hop)
            return;
        }

        if (messagesBuffer.empty() || messagesBuffer.find(wsm->getGlobalMessageIdentificaton()) == messagesBuffer.end()) { //verify if the message isn't in the buffer
            stringTmp = wsm->getWsmData();
            if (stringTmp.size() < 40){ // WSMData generated by car[x] and carry by [ T,
                stringTmp += " and carry by (";
            } else {
                stringTmp +=", ";
            }
            wsm->setWsmData(vehCategory.c_str());

            if(messagesBuffer.size() >= beaconMessageBufferSize){
                removeOldestInputBeaconMessage();
            }

            messagesBuffer.insert(make_pair(wsm->getGlobalMessageIdentificaton(),*wsm)); //add the msg in the  vehicle buffer
            messagesOrderReceived.push_back(wsm->getGlobalMessageIdentificaton());
            colorCarryMessage();
        } else {
            cout << findHost()->getFullName() << " message is on the buffer at: " << simTime() << endl;
        }
    }/*else { // to another veh
        cout << "Saving broadcast message from: " << wsm->getSenderAddressTemporary() << " to " << findHost()->getFullName() << endl;
        saveMessagesOnFile(wsm, fileMessagesNameBroadcast);
    }*/
    //printmessagesBuffer();
}

void vehDist::colorCarryMessage() {
    if (!messagesBuffer.empty()) {
        unordered_map<string, WaveShortMessage>::iterator itMessage;
        for (itMessage = messagesBuffer.begin(); itMessage != messagesBuffer.end(); itMessage++) {
            if (strcmp(itMessage->second.getSource(), findHost()->getFullName()) == 0) {
                findHost()->getDisplayString().updateWith("r=12,green"); // Has one message with himself was generated
                return;
            } else {
                findHost()->getDisplayString().updateWith("r=12,blue"); // Has only message with another was generated
            }
        }
    } else {
        findHost()->getDisplayString().updateWith("r=0"); // Remove the range color
    }
}

void vehDist::removeOldestInputBeaconMessage() {
    //printMessagesBuffer();

    if (!messagesBuffer.empty()) {
        string idMessage = messagesOrderReceived.front();
        cout << "idMessage: " << idMessage << endl;
        cout << "messagesOrderReceived.front(): " << messagesOrderReceived.front() << endl;
        cout << "messagesOrderReceived[0]: " << messagesOrderReceived[0] << endl;
        WaveShortMessage wsm = messagesBuffer[idMessage];
        simtime_t minTime = wsm.getTimestamp();

        unsigned short int typeRemoved = 0;
        if(simTime() > (minTime + ttlBeaconMessage)) {
            cout << findHost()->getFullName() << " remove one message (" << idMessage << ") by time, minTime: " << minTime << " at: " << simTime() << " ttlBeaconMessage: " << ttlBeaconMessage << endl;
            typeRemoved = 2; // by ttl (1 buffer, 2 ttl, 3 hop)
        } else if (messagesBuffer.size() >= beaconMessageBufferSize) {
            cout << findHost()->getFullName() << " remove one message (" << idMessage << ") by space, MessageBuffer.size(): " << messagesBuffer.size() << " at: " << simTime() << " beaconMessageBufferSize: " << beaconMessageBufferSize << endl;
            typeRemoved = 1; // by buffer (1 buffer, 2 ttl, 3 hop)
        }

        if(typeRemoved != 0){
            insertMessageDrop(idMessage, typeRemoved); // Removed by the value of tyRemoved (1 buffer, 2 ttl, 3 hop)
            messagesBuffer.erase(idMessage);
            messagesOrderReceived.erase(messagesOrderReceived.begin());
            colorCarryMessage();
            removeOldestInputBeaconMessage();
        }
    } /*else {
        cout << "messagesBuffer from " << findHost()->getFullName() << " is empty now" << endl;
    }*/
}

void vehDist::removeOldestInputBeaconStatus() {
    //printBeaconStatusNeighbors();

    if (!beaconStatusNeighbors.empty()) {
        unordered_map<string, WaveShortMessage>::iterator itBeaconStatus;
        itBeaconStatus = beaconStatusNeighbors.begin();
        simtime_t minTime = itBeaconStatus->second.getTimestamp();
        string idBeacon = itBeaconStatus->first;
        itBeaconStatus++;

        for (; itBeaconStatus != beaconStatusNeighbors.end(); itBeaconStatus++) {
            if (minTime > itBeaconStatus->second.getTimestamp()) {
                minTime = itBeaconStatus->second.getTimestamp();
                idBeacon = itBeaconStatus->first;
            }
        }

        if(simTime() > (minTime + ttlBeaconStatus)) {
            //cout << findHost()->getFullName() << " remove one beaconStatus (" << key << ") by time, minTime: " << minTime << " at: " << simTime() << " ttlBeaconStatus: " << ttlBeaconStatus << endl;
            beaconStatusNeighbors.erase(idBeacon);
            removeOldestInputBeaconStatus();
        } else if (beaconStatusNeighbors.size() >= beaconStatusBufferSize) {
            //cout << findHost()->getFullName() << " remove one beaconStatus (" << key << ") by space, beaconStatusNeighbors.size(): " << beaconStatusNeighbors.size() << " at: " << simTime() << " beaconMessageBufferSize: " << beaconMessageBufferSize << endl;
            beaconStatusNeighbors.erase(idBeacon);
            removeOldestInputBeaconStatus();
        }

        else {
            cout << findHost()->getFullName() << " removeOldBeacon, but did nothing!" << endl;
        }
    } /*else {
        cout << "beaconStatusNeighbors from " << findHost()->getFullName() << " is empty now" << endl;
    }*/
}

void vehDist::insertMessageDrop(string messageId, int type) {
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

    if(messagesDrop.empty() || (messagesDrop.find(messageId) == messagesDrop.end())) {
        messagesDrop.insert(make_pair(messageId, mD_tmp));
    } else if (type == 1) { // Increment the number of byBuffer (limit)
        messagesDrop[messageId].byBuffer += 1;
    } else if (type == 2) { // Increment the number of byHop (limit)
        messagesDrop[messageId].byHop += 1;
    } else if (type == 3) { // Increment the number of byTime (limit)
        messagesDrop[messageId].byTime += 1;
    }
}

void vehDist::sendBeaconMessage() {
//############################################################# run many times
    removeOldestInputBeaconStatus();
    removeOldestInputBeaconMessage();

    unordered_map<string, WaveShortMessage>::iterator itbeaconStatus;
    for (itbeaconStatus = beaconStatusNeighbors.begin(); itbeaconStatus != beaconStatusNeighbors.end(); itbeaconStatus++){
        sendMessageNeighborsTarget(itbeaconStatus->second.getSource()); // Look in buffer it has messages for this new veh neighbor
    }
//############################################################# run many times

    if (messageToSend < messagesOrderReceived.size()){
        string idMesssage = messagesOrderReceived[messageToSend];

        if( messagesBuffer.find(idMesssage) == messagesBuffer.end()){
            cout << "Error message " << idMesssage << " not found in messagesBuffer. messageToSend: " << messageToSend << endl;
            exit(1);
        }

        printMessagesBuffer();
        trySendBeaconMessage(idMesssage); // is are null?
        printMessagesBuffer();
        messageToSend++; // Move to next message
    }

    if (messageToSend >= messagesOrderReceived.size()){
        messageToSend = 0; // first position
        if (simTime() > timeToFinishLastStartSend){
            scheduleAt(simTime(), sendBeaconMessageEvt);
        } else {
            scheduleAt(timeToFinishLastStartSend, sendBeaconMessageEvt);
        }
        timeToFinishLastStartSend += timeSendLimitTime;
    } else {
        if(timeToSend < 0){
            cout << "timeToSend < 0, value:" << timeToSend << endl;
        }
        scheduleAt((simTime() + (timeToSend/1000)), sendBeaconMessageEvt);
    }
}

void vehDist::trySendBeaconMessage(string idMessage) {
    if (!messagesBuffer.empty()) { // needed?
        cout << findHost()->getFullName() << " messagesBuffer with message(s) at " << simTime() << endl;

        if (!beaconStatusNeighbors.empty()) {
            // TODO: Procurar por um artigo em que afirme que enviar a última mensagem recebida é a melhor
            //string key = returnLastMessageInserted(); // Return the ID of the last message inserted in the messageBuffer

            printBeaconStatusNeighbors();
            string rcvId;
            rcvId = neighborWithSmallDistanceToTarge(idMessage);

            //rcvId = getNeighborSmallDistanceToTarge(idMessage); // Look for a "good" veh to send the [key] (with is the last inserted) message
            if (strcmp(rcvId.c_str(), findHost()->getFullName()) != 0) {
                cout << "The chosed veh Will be send to vehicle " << rcvId << endl;
                sendWSM(updateBeaconMessageWSM(messagesBuffer[idMessage].dup(), rcvId));

                cout << findHost()->getFullName() << " send message to " << rcvId << " at "<< simTime() << endl;
                cout << " MessageID: " << messagesBuffer[idMessage].getGlobalMessageIdentificaton() << endl;
                cout << " Source: " << messagesBuffer[idMessage].getSource() << endl;
                cout << " Message Content: " << messagesBuffer[idMessage].getWsmData() << endl;
                cout << " Target: " << messagesBuffer[idMessage].getTarget() << endl;
                cout << " Timestamp: " << messagesBuffer[idMessage].getTimestamp() << endl;
                cout << " HopCount: " << (messagesBuffer[idMessage].getHopCount() - 1) << endl;
            } else {
                cout << endl << "Not send any message" << endl;
            }
        } else {
            cout << "beaconNeighbors on sendDataMessage from " << findHost()->getFullName() << " is empty now " << endl;
        }
    } /*else {
        cout << findHost()->getFullName() << " messagesBuffer is empty at " << simTime() << endl;
    }*/
}

string vehDist::neighborWithSmallDistanceToTarge(string key) {
    int neighborDistanceBefore, neighborDistanceNow, neighborDistanceLocalVeh; // They are Integer in way to ignore small differences
    unordered_map<string, smallDistance> vehSmallDistanceToTarget;
    unordered_map<string, smallDistance>::iterator itSmallDistance;
    unordered_map<string, WaveShortMessage>::iterator itBeaconNeighbors;
    int distance, smallDistanceT, smallDistanceP;
    string category, vehIdP, vehIdT;
    smallDistance sD;
    unsigned short int timeToSendVeh;

    unsigned short int percentP = 20; // 20 meaning 20%

    neighborDistanceLocalVeh = traci->getDistance(curPosition, messagesBuffer[key].getTargetPos(), false);
    for (itBeaconNeighbors = beaconStatusNeighbors.begin(); itBeaconNeighbors != beaconStatusNeighbors.end(); itBeaconNeighbors++) {
        neighborDistanceBefore = traci->getDistance(itBeaconNeighbors->second.getSenderPosPrevious(), messagesBuffer[key].getTargetPos(), false);
        neighborDistanceNow = traci->getDistance(itBeaconNeighbors->second.getSenderPos(), messagesBuffer[key].getTargetPos(), false);

        if (neighborDistanceNow > 720){
            cout << "id: " << key << " source: " <<itBeaconNeighbors->second.getSource() << endl;
            cout << "neighborDistanceNow > 720, value: " << neighborDistanceNow << " target: " << messagesBuffer[key].getTargetPos() << endl;
            exit(1);
        }

        if (neighborDistanceNow < neighborDistanceBefore) { // test if is closing to target
            if (neighborDistanceNow < neighborDistanceLocalVeh){ // test if is more close to the target than the veh with are trying to send
                sD.categoryVeh = itBeaconNeighbors->second.getCategory();
                sD.distanceToTarget = neighborDistanceNow;
                sD.timeToSendVeh = itBeaconNeighbors->second.getTimeToSend();

                // TODO Insert speed veh?
                vehSmallDistanceToTarget.insert(make_pair(itBeaconNeighbors->first, sD));
            } else {
                cout << itBeaconNeighbors->first << " don't has small distance to the target in comparison with " << findHost()->getFullName() << endl;
            }
        } else {
            cout << itBeaconNeighbors->first << " going to another direction" << endl;
        }
    }

    cout << "Print of vehSmallDistanceToTarget to " << findHost()->getFullName() << " at " << simTime() << endl;
    if (vehSmallDistanceToTarget.empty()){
        cout << "vehSmallDistanceToTarget is empty." << endl;
    } else {
        for (itSmallDistance = vehSmallDistanceToTarget.begin(); itSmallDistance != vehSmallDistanceToTarget.end(); itSmallDistance++) {
            cout << "Id: " << itSmallDistance->first << " Category: " << itSmallDistance->second.categoryVeh;
            cout << " Distance: " << itSmallDistance->second.distanceToTarget << "TimeToSend: " << itSmallDistance->second.timeToSendVeh << endl;
        }
    }

    if (vehSmallDistanceToTarget.empty()){ // If don't any veh going to target
        return findHost()->getFullName();
    }

    smallDistanceP = smallDistanceT = INT_MAX;
    // TODO Test timeToSend
    for(itSmallDistance = vehSmallDistanceToTarget.begin(); itSmallDistance != vehSmallDistanceToTarget.end(); itSmallDistance++) {
        category = itSmallDistance->second.categoryVeh;
        distance = itSmallDistance->second.distanceToTarget;
        if (strcmp(category.c_str(), "P") == 0){
            cout << itSmallDistance->first << " with P" << endl;
            if (smallDistanceP > distance){
                smallDistanceP = distance;
                vehIdP = itSmallDistance->first;
            }
        } else if (strcmp(category.c_str(), "T") == 0) {
            cout << itSmallDistance->first << " with T" << endl;
            if (smallDistanceT > distance) {
                smallDistanceT = distance;
                vehIdT = itSmallDistance->first;
            }
        }
    }

    if (smallDistanceT == INT_MAX) {
        return vehIdP;
    } else if (smallDistanceP == INT_MAX){
        return vehIdT;
    }

    int valRand = rand() % 100;
    if ((smallDistanceP != INT_MAX) && (smallDistanceT != INT_MAX)) {
        if (valRand < percentP) {
            return vehIdP;
        } else if (valRand >= percentP) {
           return vehIdT;
        }
    }
    return findHost()->getFullName();
}

string vehDist::getNeighborSmallDistanceToTarge(string key) {
    string vehID = findHost()->getFullName();
    int neighborSmallDistance, neighborDistanceBefore, neighborDistanceNow, senderTmpDistance; // They are Integer in way to ignore small differences

    senderTmpDistance = traci->getDistance(mobility->getCurrentPosition(), messagesBuffer[key].getTargetPos(), false);
    neighborSmallDistance = senderTmpDistance;

    unordered_map<string, WaveShortMessage>::iterator itBeacon;
    for (itBeacon = beaconStatusNeighbors.begin(); itBeacon != beaconStatusNeighbors.end(); itBeacon++) {
        neighborDistanceBefore = traci->getDistance(itBeacon->second.getSenderPosPrevious(), messagesBuffer[key].getTargetPos(), false);
        neighborDistanceNow = traci->getDistance(itBeacon->second.getSenderPos(), messagesBuffer[key].getTargetPos(), false);

        if (neighborDistanceNow < neighborDistanceBefore) { // Veh is going in direction to target
            if (neighborDistanceNow < senderTmpDistance) { // The distance of this veh to target is small than the carry veh now

                cout << " The distance is smaller to target " << endl;
                cout << " Position of this veh (" << findHost()->getFullName() << "): " << mobility->getCurrentPosition() << endl;
                cout << " Sender beacon pos previous: " << itBeacon->second.getSenderPosPrevious() << endl;
                cout << " Sender beacon pos now: " << itBeacon->second.getSenderPos() << endl;
                cout << " Message id: " << key << endl;
                cout << " TargetPos: " << messagesBuffer[key].getTargetPos() << endl;

                if (neighborDistanceNow < neighborSmallDistance) {
                    neighborSmallDistance = neighborDistanceNow; // Found one veh with small distance to target to send a [ke] message
                    vehID = itBeacon->first;
                    cout << " Selected one veh in the neighbor (" << vehID << ") with small distance to the target" << endl;
                } else if (neighborDistanceNow == neighborSmallDistance) {
                    if (beaconStatusNeighbors[itBeacon->first].getSenderSpeed() > beaconStatusNeighbors[vehID].getSenderSpeed()) {
                        neighborSmallDistance = neighborDistanceNow; // Found one veh with equal distance to target to another veh, but with > speed
                        vehID = itBeacon->first;
                        cout << "  Select another veh with the same distance, but with more speed [beaconNeighbors]" << vehID << endl;
                    }
                }
            }
        }
    }
    // if vehID != findHost()->getFullName(), will send to this Veh[ID], if equal will not send a message
    return vehID;
}

void vehDist:: finish() {
    printCountBeaconMessagesDrop();

    auto it = find(vehDist::numVehicles.begin(), vehDist::numVehicles.end(), source);
    if(it != vehDist::numVehicles.end()){
        vehDist::numVehicles.erase(it);
    } else {
        cout << "Error in vehDist::numVehicles, need to have the same entries as the number of vehicles" << endl;
        exit (1);
    }
}

void vehDist::printCountBeaconMessagesDrop() {
    myfile.open (fileMessagesDrop, std::ios_base::app);

    if (messagesDrop.empty()) {
        myfile << endl << "messagesDrop from " << findHost()->getFullName() << " is empty now" << endl;
        myfile << "### " << source << " dropped: " << 0 << endl;
    } else {
        myfile << endl << "messagesDrop from " << findHost()->getFullName() << endl;
        unsigned short int messageDropbyOneVeh = 0;
        map<string, struct messagesDropStruct>::iterator it;
        for (it = messagesDrop.begin(); it != messagesDrop.end(); it++) {
            myfile << "Message Id: " << it->first << endl;
            myfile << "By Buffer: " << it->second.byBuffer << endl;
            myfile << "By Hop: " << it->second.byHop << endl;
            myfile << "By Time: " << it->second.byTime << endl;
            messageDropbyOneVeh += it->second.byBuffer + it->second.byHop + it->second.byTime;
        }
        countMesssageDrop += messageDropbyOneVeh;
        myfile << "### " << source << " dropped: " << messageDropbyOneVeh << endl;
    }

    if (vehDist::numVehicles.size() == 1) {
        myfile << endl << "Exp: " << experimentNumber << " ### Final count messages drop: " << countMesssageDrop << endl << endl;
    }
    myfile.close();
}

string vehDist::returnLastMessageInserted() {
    unordered_map<string, WaveShortMessage>::iterator itMessage;
    itMessage = messagesBuffer.begin();
    simtime_t minTime;
    minTime = itMessage->second.getTimestamp();
    string idMessage = itMessage->first;
    itMessage++;
    for (; itMessage != messagesBuffer.end(); itMessage++) {
        if (minTime > itMessage->second.getTimestamp()) {
            minTime = itMessage->second.getTimestamp();
            idMessage = itMessage->first;
        }
    }
    return idMessage;
}

void vehDist::sendMessageNeighborsTarget(string beaconSource) {
    unsigned short int countMessage = messagesBuffer.size();
    unordered_map<string, WaveShortMessage>::iterator itMessage;
    itMessage = messagesBuffer.begin();
    string idMessage;
    while(countMessage > 0) {
        if (strcmp(itMessage->second.getTarget(), beaconSource.c_str()) == 0) {
            idMessage = itMessage->second.getGlobalMessageIdentificaton();
            cout << "Sending message: " << idMessage << " to: " << beaconSource << " and removing" << endl;
            sendWSM(updateBeaconMessageWSM(itMessage->second.dup(), beaconSource));
            messagesDelivered.push_back(idMessage);

            if (countMessage == 1){
                countMessage=0;
            } else {
                itMessage++;
                countMessage--;
            }

            messagesBuffer.erase(idMessage);
            auto it = find(messagesOrderReceived.begin(), messagesOrderReceived.end(), idMessage);
            if(it != messagesOrderReceived.end()) {
                messagesOrderReceived.erase(it);
            } else {
                cout << "Error in messagesOrderReceived, need to have the same entries as messagesBuffer" << endl;
                exit (1);
            }
            colorCarryMessage();
        } else {
            countMessage--;
            itMessage++;
        }
    }
    cout << "end of sendMessageNeighborsTarget()" << endl;
}

void vehDist::handleLowerMsg(cMessage* msg) {
    WaveShortMessage* wsm = dynamic_cast<WaveShortMessage*>(msg);
    ASSERT(wsm);

    if (std::string(wsm->getName()) == "beacon") {
        //onBeacon(wsm);
        cout << " Received one message with name \"beacon\"" << endl;
        exit(1);
    }
    else if (std::string(wsm->getName()) == "data") {
        //onData(wsm);
        cout << " Received one message with name \"data\"" << endl;
        exit(1);
    }
//###############################################################
    else if (wsm->getType() == 1) {
        onBeaconStatus(wsm);
    }
    else if (wsm->getType() == 2) {
        onBeaconMessage(wsm);
    }
//###############################################################
    else {
        DBG << "unknown message (" << wsm->getName() << ")  received\n";
        exit(1);
    }
    delete(msg);
}

//###################################################  OK Function ####################################################

void vehDist::saveVehStartPosition(string fileNameLocation) {
    fileNameLocation += "Veh_Position_Initialize.r";
    if (strcmp(source.c_str(), "car[0]") == 0) {
        if (repeatNumber == 0) {
            myfile.open (fileNameLocation);
        } else {
            myfile.open (fileNameLocation, std::ios_base::app);
        }
        printHeaderfileExecution(ttlBeaconMessage, countGenerateBeaconMessage);
        myfile << "Start Position Vehicles" << endl;
    } else {
        myfile.open (fileNameLocation, std::ios_base::app);
    }
    myfile << findHost()->getFullName() << ": "<< mobility->getCurrentPosition() << endl;
    myfile.close();
}

void vehDist::restartFilesResult() {
    stringTmp = "results/resultsEnd/E" + to_string(experimentNumber);
    stringTmp += "_" + to_string((static_cast<int>(ttlBeaconMessage))) + "_" + to_string(countGenerateBeaconMessage) +"/";

    saveVehStartPosition(stringTmp); // Save the start position of vehicle. Just for test of the seed.

    fileMessagesUnicast = fileMessagesDrop = fileMessagesGenerated = stringTmp;
    fileMessagesUnicast += "Veh_Unicast_Messages.r";
    fileMessagesDrop += "Veh_Messages_Drop.r";
    fileMessagesGenerated += "Veh_Messages_Generated.r";

    // fileMessagesBroadcast and fileMessagesCount not used yet to Car

    if (myId == 0) {
        if (repeatNumber == 0) {
            openFileAndClose(fileMessagesUnicast, false, ttlBeaconMessage, countGenerateBeaconMessage);
            openFileAndClose(fileMessagesDrop, false, ttlBeaconMessage, countGenerateBeaconMessage);
            openFileAndClose(fileMessagesGenerated, false, ttlBeaconMessage, countGenerateBeaconMessage);
        } else { // (repeatNumber != 0)) // open just for append
            openFileAndClose(fileMessagesUnicast, true, ttlBeaconMessage, countGenerateBeaconMessage);
            openFileAndClose(fileMessagesDrop, true, ttlBeaconMessage, countGenerateBeaconMessage);
            openFileAndClose(fileMessagesGenerated, true, ttlBeaconMessage, countGenerateBeaconMessage);
        }
    }
}

void vehDist::vehUpdatePosition() {
    vehPositionPrevious = mobility->getCurrentPosition();
    //cout << "initial positionPrevious: " << vehPositionPrevious << endl;
    sendUpdatePosisitonVeh = new cMessage("Event update position vehicle", SendEvtUpdatePositionVeh);
    //cout << findHost()->getFullName() << " at simtime: "<< simTime() << "schedule created UpdatePosition to: "<< (simTime() + par("vehTimeUpdatePosition").doubleValue()) << endl;
    scheduleAt((simTime()+ par("vehTimeUpdatePosition").doubleValue()), sendUpdatePosisitonVeh);
}

void vehDist::vehCreateUpdateTimeToSendEvent() {
    timeToSend = 100; // Send in: 100 ms or 0.1 s
    distanceTimeToSend = 10; // Equal to 10 m/s or 36 km/h
    timeSendLimitTime = par("beaconMessageInterval").doubleValue(); // #5; // Limit that timeToSend can be (one message by 5s), value must be (bufferMessage limit) * timeToSend, in this case 50 * 0.1 = 5
    timeToUpdateTimeToSend = 1; // Will update every one second

    sendUpdateTimeToSendVeh = new cMessage("Event update timeToSend vehicle", SendEvtUpdateTimeToSendVeh);
    cout << findHost()->getFullName() << " at: " << simTime() << " schedule created UpdateTimeToSend to: "<< (simTime() + timeToUpdateTimeToSend) << endl;
    scheduleAt((simTime() + timeToUpdateTimeToSend), sendUpdateTimeToSendVeh);
}

void vehDist::vehCreateEventTrySendBeaconMessage() {
    if (sendData) {
        sendBeaconMessageEvt = new cMessage("Event send beacon message", SendEvtBeaconMessage);
        timeToFinishLastStartSend = simTime() + vehOffSet;
        messageToSend = 0; // messagesOrderReceived.front();
        cout << findHost()->getFullName() << " at: "<< simTime() << " schedule created SendBeaconMessage to: "<< timeToFinishLastStartSend << endl;
        scheduleAt(timeToFinishLastStartSend, sendBeaconMessageEvt);
        timeToFinishLastStartSend += timeSendLimitTime;
    }
}

void vehDist::selectVehGenerateMessage() {
    if (myId == 0) { // if true, some veh has (in past) selected the veh to generate messages
        if (simTime() <= timeLimitGenerateBeaconMessage) {
            unsigned short int vehSelected;
            myfile.open(fileMessagesGenerated, std::ios_base::app); // To save info (Id and veh generate) on fileMessagesGenerated
            for (unsigned short int i = 0; i < countGenerateBeaconMessage;) { // select <countGenerateBeaconMessage> distinct veh to generate messages
                vehSelected = rand() % vehDist::numVehicles.size(); // random car to generate message
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
        //cout << findHost()->getFullName() << " at: "<< simTime() << "schedule created sendGenerateMessageEvt to: "<< (simTime() + vehOffSet) << endl;
        scheduleAt((simTime() + vehOffSet), sendGenerateBeaconMessageEvt);
    }
}

void vehDist::vehGenerateBeaconMessageAfterBegin() {
    selectVehGenerateMessage();

    if(vehDist::vehGenerateMessage.find(myId) != vehDist::vehGenerateMessage.end()) { // if have "vehNumber" on buffer, will generate one message
        generateBeaconMessage();
        vehDist::vehGenerateMessage.erase(myId);
    }
}

WaveShortMessage* vehDist::prepareBeaconStatusWSM(std::string name, int lengthBits, t_channel channel, int priority, int serial) {
    WaveShortMessage* wsm = new WaveShortMessage(name.c_str());
    wsm->setType(1);
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
    wsm->setSource(source.c_str());

    // beaconStatus don't need
    //wsm->setRecipientAddressTemporary();  // => "BROADCAST"
    //wsm->setSenderAddressTemporary(findHost()->getFullName());
    //wsm->setTarget(); // => "BROADCAST"

    wsm->setRoadId(mobility->getRoadId().c_str());
    wsm->setSenderSpeed(mobility->getSpeed());
    wsm->setCategory(vehCategory.c_str());
    //wsm->setSenderPos(curPosition);
    wsm->setSenderPos(mobility->getCurrentPosition());
    wsm->setSenderPosPrevious(vehPositionPrevious);
    wsm->setTimeToSend(timeToSend);

    // heading 1 to 4 or 1 to 8
    wsm->setHeading(getVehHeading4());
    //wsm->setHeading(getVehHeading8());

    DBG << "Creating BeaconStatus with Priority " << priority << " at Applayer at " << wsm->getTimestamp() << std::endl;
    return wsm;
}

WaveShortMessage* vehDist::updateBeaconMessageWSM(WaveShortMessage* wsm, string rcvId) {
    wsm->setSenderAddressTemporary(findHost()->getFullName());
    wsm->setRecipientAddressTemporary(rcvId.c_str());
    wsm->setRoadId(mobility->getRoadId().c_str());
    wsm->setCategory(vehCategory.c_str());
    wsm->setSenderSpeed(mobility->getSpeed());
    wsm->setSenderPos(curPosition);
    wsm->setSenderPosPrevious(vehPositionPrevious);
    wsm->setHeading(getVehHeading4());
    wsm->setHopCount(wsm->getHopCount() -1);
    return wsm;
}

void vehDist::generateTarget() { //Generate a target in order to send a message
    target = "rsu[0]"; // Set the target node to who the message has to be delivered
    target_x = par("vehBeaconMessageTarget_x").longValue();
    target_y = par("vehBeaconMessageTarget_y").longValue();
}

void vehDist::generateBeaconMessage() {
    WaveShortMessage* wsm = new WaveShortMessage("beaconMessage");
    wsm->setType(2);
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
    wsm->setTimestamp(simTime());

    wsm->setSenderAddressTemporary(findHost()->getFullName());
    //wsm->setRecipientAddress(); // defined before send
    wsm->setRecipientAddressTemporary("Initial");
    wsm->setSource(source.c_str());

    generateTarget(); //target = rsu[0], rsu[1] or car[*] and the respective position.
    wsm->setTargetPos(Coord(target_x, target_y, 3));
    wsm->setTarget(target.c_str());
    wsm->setHopCount(beaconMessageHopLimit+1); // Is beaconMessageHopLimit+1 because hops count is equals to routes in the path, not hops.

    stringTmp = "WSMData generated by ";
    stringTmp += findHost()->getFullName();
    wsm->setWsmData(stringTmp.c_str());

    myfile.open(fileMessagesGenerated, std::ios_base::app); // Save info (Id and veh generate) on fileMessagesGenerated
    myfile << "                                                                     ";
    if (vehDist::beaconMessageId < 10) {
        stringTmp = '0' + to_string(vehDist::beaconMessageId);
        wsm->setGlobalMessageIdentificaton(stringTmp.c_str()); // Id 01 to 09
    } else {
        wsm->setGlobalMessageIdentificaton(to_string(vehDist::beaconMessageId).c_str()); // Id 10 and keep going
    }
    myfile << "### " << findHost()->getFullName() << " generated the message ID: " << wsm->getGlobalMessageIdentificaton() << " at simTime: " << simTime() << endl;
    myfile.close();

    messagesBuffer.insert(make_pair(wsm->getGlobalMessageIdentificaton(),*wsm)); // Adding the message on the buffer
    messagesOrderReceived.push_back(wsm->getGlobalMessageIdentificaton());
    colorCarryMessage(); // Change the range-color in the veh (GUI)
    vehDist::beaconMessageId++;
}

void vehDist::handleSelfMsg(cMessage* msg) {
    switch (msg->getKind()) {
        case SEND_BEACON_EVT: {
            sendWSM(prepareBeaconStatusWSM("beaconStatus", beaconLengthBits, type_CCH, beaconPriority, -1));
            scheduleAt((simTime() + par("beaconInterval").doubleValue()), sendBeaconEvt);
            break;
        }
        case SendEvtBeaconMessage: {
            sendBeaconMessage();
            break;
        }
        case SendEvtUpdatePositionVeh: {
            updateVehPosition();
            scheduleAt((simTime() + par("vehTimeUpdatePosition").doubleValue()), sendUpdatePosisitonVeh);
            break;
        }
        case SendEvtUpdateTimeToSendVeh: {
            vehUpdateTimeToSend();
            scheduleAt((simTime() + timeToUpdateTimeToSend), sendUpdateTimeToSendVeh);
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

void vehDist::vehUpdateTimeToSend() {
    cout << "Vehicle: " << findHost()->getFullName() << " timeToSend: " << timeToSend;
    double distance = traci->getDistance(mobility->getPositionAt(simTime() - timeToUpdateTimeToSend), curPosition, false);

//    if (distance > distanceTimeToSend){
//        if(fabs(timeToSend - 0.1) > 0.0000001f) { // timeToSend > 0.1f
//            timeToSend -= 0.1;
//
//            if (fabs(timeToSend - 0.1) < -0.0000001f) {
//                cout << endl << "Error timeToSend:" << timeToSend << " is less than 0.1" << endl;
//                exit(1);
//            }
//        }
//    } else {
//        if (timeToSend < timeSendLimitTime){
//            timeToSend += 0.1;
//        }
//    }

    if (distance > distanceTimeToSend){
         if(timeToSend > 100) { // timeToSend > 0.1f
             timeToSend -= 100;

             if (timeToSend < 100) {
                 cout << endl << "Error timeToSend:" << timeToSend << " is less than 1" << endl;
                 exit(1);
             }
         }
     } else {
         if (timeToSend < timeSendLimitTime){
             timeToSend += 100;
         }
     }
    cout << " Updated to: " << timeToSend << " at: " << simTime() << " by: " <<  distance << " traveled ["<< mobility->getPositionAt(simTime() - timeToUpdateTimeToSend) << " " << curPosition << "]" << endl;
}

void vehDist::printMessagesBuffer() {
    if (!messagesBuffer.empty()) {
        cout << endl << "messagesBuffer from " << findHost()->getFullName() << " at " << simTime() << endl;
        unordered_map<string, WaveShortMessage>::iterator it;
        for (it = messagesBuffer.begin(); it != messagesBuffer.end(); it++) {
            cout << " Id: " << it->second.getGlobalMessageIdentificaton() << endl;
            cout << " Message Content: " << it->second.getWsmData() << endl;
            cout << " Source: " << it->second.getSource() << endl;
            cout << " Target: " << it->second.getTarget() << endl;
            cout << " Timestamp: " << it->second.getTimestamp() << endl;
            cout << " HopCount: " << it->second.getHopCount() << endl << endl;
        }
    } else {
        cout << "messagesBuffer from " << findHost()->getFullName() << " is empty now: " << simTime() << endl;
    }
}

void vehDist::printBeaconStatusNeighbors() {
    if (!beaconStatusNeighbors.empty()) {
        cout << endl << "beaconNeighbors from " << findHost()->getFullName() << " at " << simTime() << " Position: " << curPosition <<endl;
        unordered_map<string, WaveShortMessage>::iterator it;
        for (it = beaconStatusNeighbors.begin(); it != beaconStatusNeighbors.end(); it++) {
            cout << " Id(veh): " << it->first << endl;
            cout << " PositionPrevious: " << it->second.getSenderPosPrevious() << endl;
            cout << " Position: " << it->second.getSenderPos() << endl;
            cout << " Speed: " << it->second.getSenderSpeed() << endl;
            cout << " Category: " << it->second.getCategory() << endl;
            cout << " RoadId: " << it->second.getRoadId() << endl;
            cout << " Heading: " << it->second.getHeading() << endl;
            cout << " Timestamp: " << it->second.getTimestamp() << endl << endl;
        }
    }
    else {
        cout << "beaconNeighbors from " << findHost()->getFullName() << " is empty now: " << simTime() << endl;
    }
}

unsigned short int vehDist::getVehHeading4() {
     /*
     return angle <0º - 359º>
     marcospaiva.com.br/images/rosa_dos_ventos%2002.GIF
     marcospaiva.com.br/localizacao.htm
     (angle >= 315 && angle < 360) || (angle >= 0 && angle < 45)     return 1; // L or E => 0º
     angle >= 45 && angle < 135                                      return 2; // N => 90º
     angle >= 135  && angle < 225                                    return 3; // O or W => 180º
     angle >= 225  && angle < 315                                    return 4; // S => 270º
     angle >= 360                                                    return 9; // Error
     */

    double angle;
    if (mobility->getAngleRad() < 0) // radians are negative, so degrees negative
        angle = (((mobility->getAngleRad() + 2*M_PI ) * 180)/ M_PI);
    else //radians are positive, so degrees positive
        angle = ((mobility->getAngleRad() * 180) / M_PI);

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

unsigned short int vehDist::getVehHeading8() {
     /*
     return angle <0º - 359º>
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
     angle >= 360                                                    return 9; // Error
     */

    double angle;
    if (mobility->getAngleRad() < 0) // radians are negative, so degrees negative
        angle = (((mobility->getAngleRad() + 2*M_PI ) * 180)/ M_PI);
    else //radians are positive, so degrees positive
        angle = ((mobility->getAngleRad() * 180) / M_PI);

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
    //cout << "Vehicle: " << findHost()->getFullName();
    //cout << " Update position: " <<  "time: "<< simTime() << " next update: ";
    //cout << (simTime() + par("vehTimeUpdatePosition").doubleValue()) << endl;
    //cout << "positionPrevious: " << vehPositionPrevious;
    vehPositionPrevious = mobility->getPositionAt(simTime() - par("vehTimeUpdatePosition").doubleValue());
    //cout << " and update positionPrevious: " << vehPositionPrevious << endl << endl;
}

//########################################################  Default Function #############################################

void vehDist::sendWSM(WaveShortMessage* wsm) {
    //if (isParking && !sendWhileParking)
    //    return;
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
    isParking = mobility->getParkingState();
    if (sendWhileParking == false) {
        if (isParking == true) {
            (FindModule<BaseConnectionManager*>::findGlobalModule())->unregisterNic(this->getParentModule()->getSubmodule("nic"));
        } else {
            Coord pos = mobility->getCurrentPosition();
            (FindModule<BaseConnectionManager*>::findGlobalModule())->registerNic(this->getParentModule()->getSubmodule("nic"), (ChannelAccess*) this->getParentModule()->getSubmodule("nic")->getSubmodule("phy80211p"), &pos);
        }
    }
}

void vehDist::handlePositionUpdate(cObject* obj) {
    BaseWaveApplLayer::handlePositionUpdate(obj);

    // stopped for for at least 10s?
    if (mobility->getSpeed() < 1) {
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
    WaveShortMessage* wsm = prepareWSM("data", dataLengthBits, channel, dataPriority, -1,2);
    wsm->setWsmData(blockedRoadId.c_str());
    sendWSM(wsm);
}

void vehDist::onData(WaveShortMessage* wsm) {
}

void vehDist::onBeacon(WaveShortMessage* wsm) {
    if (wsm->getType() == 1){ 
        onBeaconStatus(wsm); // Beacon of status
    } else if (wsm->getType() == 2){
        onBeaconMessage(wsm); // Beacon of Message
    }
}
