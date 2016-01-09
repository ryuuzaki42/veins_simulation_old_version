//
// Copyright (C) 2006-2011 Christoph Sommer <christoph.sommer@uibk.ac.at>
//
// Documentation for these modules is at http://veins.car2x.org/
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

        source = findHost()->getFullName();
        hopLimit = par("hopLimit").longValue();
        repeatNumber = par("repeatNumber");

        vehSendData();
        vehUpdatePosition();
        restartFilesResult();
        setVehID();

        saveVehStartPosition();
        //cout << endl << findHost()->getFullName() << " entered in the scenario" << endl;
    }
}
// Add more messages on scenario [on buffer 60s (1.2 to 61.2 )]
// Generate them with static bool genetateMsg (true when messagesToGenerate is empty, turn false when one veh start generate

void vehDist::onBeaconStatus(WaveShortMessage* wsm) {
    //cout << "OnBeacon Id(veh): " << wsm->getVehicleId() << " timestamp: " << wsm->getTimestamp() << endl;

    removeOldestInput(&beaconNeighbors, par("ttlBeacon").doubleValue(), par("beaconBufferSize").doubleValue());

    auto it = beaconNeighbors.find(wsm->getSenderAddressString());
    if (it != beaconNeighbors.end()){
        //cout << "Update wsm beacon (Key: " << wsm->getSenderAddressString() << ", timestamp: " << beaconNeighbors[wsm->getSenderAddressString()].getTimestamp() << ")" << endl;
        wsm->setTimestamp(simTime());
        it->second = *wsm;
    } else {
        beaconNeighbors.insert(make_pair(wsm->getSenderAddressString(), *wsm));
    }
    //cout << "veh: " << findHost()->getFullName() << "beaconNeighbors.size(): " << beaconNeighbors.size() << endl;
    //printBeaconNeighbors();

    sendMessageNeighborsTarget();
}

void vehDist::onBeaconMessage(WaveShortMessage* wsm) {
    //findHost()->bubble("Received a Message");

    removeOldestInput(&messagesBuffer, par("ttlMessage").doubleValue(), par("messageBufferSize").doubleValue());

    //verify if this is the recipient of the message
    if (strcmp(wsm->getRecipientAddressString(), findHost()->getFullName()) == 0){

        if (!messagesDelivered.empty()){
            vector<string>::iterator itMessageDelivered;
            itMessageDelivered = find(messagesDelivered.begin(), messagesDelivered.end(), wsm->getGlobalMessageIdentificaton());

            if (itMessageDelivered != messagesDelivered.end()) { // message has been delivered to the target before.
                cout << "This message has been delivered to the target before." << endl;
                return;
            }
        }

        cout << "Save message from: " << wsm->getSenderAddressString() << " to " << findHost()->getFullName() << endl;
        saveMessagesOnFile(wsm, fileMessagesNameUnicast);

        if (wsm->getHopCount() == 0){
            unordered_map<string, int>::const_iterator searchDrop = messagesDrop.find(wsm->getGlobalMessageIdentificaton());
            if(messagesDrop.empty() || searchDrop == messagesDrop.end()){
                messagesDrop.insert(make_pair(wsm->getGlobalMessageIdentificaton(), 1));
            } else {
                messagesDrop[wsm->getGlobalMessageIdentificaton()] += 1;
            }
            return;
        }

        unordered_map<string, WaveShortMessage>::const_iterator search = messagesBuffer.find(wsm->getGlobalMessageIdentificaton());
        if (messagesBuffer.empty() || search == messagesBuffer.end()) { //verify if the message isn't in the buffer
            //cout << findHost()->getFullName() << " buffer empty or the vehicle don't have the message at simtime: " << simTime() << endl;

            //add the msg in the  vehicle buffer
            messagesBuffer.insert(make_pair(wsm->getGlobalMessageIdentificaton(),*wsm));
            colorCarryMessage();

            //cout << findHost()->getFullName() << " recive the message " << endl;
            //cout << "from (source): " << wsm->getSource() << endl;
            //cout << " sender: " << wsm->getSenderAddressString() << endl;
            //cout << " hopCount: " << wsm->getHopCount() << endl;
            //cout << " At: " << simTime() << endl;
        }else if(search != messagesBuffer.end()){
            cout<<findHost()->getFullName() << " message is on the buffer at simtime " << simTime() << endl;
        }
    } else { // to another veh
        //cout << "Save message from: " << wsm->getSenderAddressString() << " to " << findHost()->getFullName() << endl;
        saveMessagesOnFile(wsm, fileMessagesNameBroadcast);
    }
    //printmessagesBuffer();
}

void vehDist::colorCarryMessage(){
    if (messagesBuffer.empty()){
        findHost()->getDisplayString().updateWith("r=0,green");
    } else {
        findHost()->getDisplayString().updateWith("r=12,green");
    }
}

void vehDist::removeOldestInput(unordered_map<string, WaveShortMessage>* data, double timeValid, unsigned int bufferLimit){
    //cout << "veh: " << findHost()->getFullName() << " Buffercount: " << data->size() << endl;
    if (data->empty()) {
        //cout << "Data buffer from " << findHost()->getFullName() << " is empty now " << endl;
        return;
    } else {
        unordered_map<string, WaveShortMessage>::iterator itData;
        itData = data->begin();
        string type = itData->second.getName();
        simtime_t minTime = itData->second.getTimestamp();
        string key = itData->first;
        itData++;
        for (; itData != data->end(); itData++) {
            if (minTime > itData->second.getTimestamp()){
                minTime = itData->second.getTimestamp();
                key = itData->first;
            }
        }
        if((minTime + timeValid) < simTime()){
            //cout << findHost()->getFullName() << " remove one " << type << " by time " << "Timestamp: " << minTime << " at: " << simTime() << " timeValid: " << timeValid << endl;
            data->erase(key);
        } else if (data->size() > bufferLimit) {
            //cout << findHost()->getFullName() << " remove one " << type << " by space" << "Timestamp: " << minTime << " at: " << simTime() << " timeValid: " << timeValid << endl;
            data->erase(key);
        }
    }
    colorCarryMessage();
}

void vehDist::sendDataMessage() {

    removeOldestInput(&messagesBuffer, par("ttlMessage").doubleValue(), par("messageBufferSize").doubleValue());
    removeOldestInput(&beaconNeighbors, par("ttlBeacon").doubleValue(), par("beaconBufferSize").doubleValue());

    if (messagesBuffer.empty()) {
        //cout << findHost()->getFullName() << " messagesBuffer is empty at " << simTime() << endl;
        return;
    } else {
        cout << findHost()->getFullName() << " messagesBuffer with message(s) at " << simTime() << endl;

        if (beaconNeighbors.empty()) {
            cout << "beaconNeighbors on sendDataMessage from " << findHost()->getFullName() << " is empty now " << endl;
        } else {

            //cout << "Before sendBeaconNeighborsTarget messagesBuffer.size(): " << messagesBuffer.size() << endl;
            sendMessageNeighborsTarget();
            if (messagesBuffer.empty()) {
               return;
            }
            //cout << "After sendBeaconNeighborsTarget messagesBuffer.size(): " << messagesBuffer.size() << endl;

            string key = returnLastMessageInsert();

            bool send = false;

            unordered_map<string, WaveShortMessage>::iterator itBeacon;
            for (itBeacon = beaconNeighbors.begin(); itBeacon != beaconNeighbors.end(); itBeacon++){

                //test send distante e heading to target
                //cout => car[x] distance <is more, is less, not change> to [target](rsu[0]) by car[y]
                cout << findHost()->getFullName() << " distance";
                send = sendtoTargetbyVeh(itBeacon->second.getSenderPosBack(), itBeacon->second.getSenderPos(), itBeacon->second.getHeading(), messagesBuffer[key].getTargetPos());
                cout << "to " << messagesBuffer[key].getTarget() << " by " << itBeacon->second.getSenderAddressString() << endl;

                cout << endl;
                cout << " SenderPosBack: " << itBeacon->second.getSenderPosBack() << endl;
                cout << " SenderPos: " << itBeacon->second.getSenderPos() << endl;
                cout << " Heading: " << itBeacon->second.getHeading() << endl;
                cout << " Message id: " << key << endl;
                cout << " TargetPos: " << messagesBuffer[key].getTargetPos() << endl;
                cout << endl;
                if (send){
                    break;
                }
            }

            if (send){
                string rcvId = itBeacon->first;

                sendWSM(updateMessageWSM(messagesBuffer[key].dup(), rcvId));

                cout << findHost()->getFullName() << " send message to " << rcvId << " at "<< simTime() << endl;
                cout << " MessageID: " << messagesBuffer[key].getGlobalMessageIdentificaton() << endl;
                cout << " Source: " << messagesBuffer[key].getSource() << endl;
                cout << " Message Content: " << messagesBuffer[key].getWsmData() << endl;
                cout << " Target: " << messagesBuffer[key].getTarget() << endl;
                cout << " Timestamp: " << messagesBuffer[key].getTimestamp() << endl;
                cout << " HopCount:" << (messagesBuffer[key].getHopCount() - 1) << endl;
            }
            else{
                cout << endl << "Not send message to" << endl;
            }
            //printMessagesBuffer();
        }
    }
}

void vehDist:: finish(){
    printCountMessagesDrop();
}

// mudar criar outra função que imprime depois dessa
void vehDist::printCountMessagesDrop(){
    myfile.open (fileMessagesDrop, std::ios_base::app);

    if (messagesDrop.empty()) {
        myfile << "messagesDrop from " << findHost()->getFullName() << " is empty now" << endl;
        return;
    } else {
        myfile << "messagesDrop from " << findHost()->getFullName() << endl;
        //mudar para map
        unordered_map<string, int>::iterator it;
        for (it = messagesDrop.begin(); it != messagesDrop.end(); it++) {
            myfile << "Message Id: " << it->first << endl;
            myfile << "Count received: " << it->second << endl;
            countMesssageDrop += it->second;
        }
    }
    //mudar para Current count messages drop:
    myfile << "Current: " << countMesssageDrop << endl;
    myfile << endl;
    myfile.close();
}

string vehDist::returnLastMessageInsert(){
    unordered_map<string, WaveShortMessage>::iterator itMessage;
    itMessage = messagesBuffer.begin();
    simtime_t minTime;
    minTime = itMessage->second.getTimestamp();
    string key = itMessage->first;
    itMessage++;
    for (; itMessage != messagesBuffer.end(); itMessage++) {
        if (minTime > itMessage->second.getTimestamp()){
            minTime = itMessage->second.getTimestamp();
            key = itMessage->first;
        }
    }
    return key;
}

void vehDist::sendMessageNeighborsTarget(){

    unordered_map<string, WaveShortMessage>::iterator itMessage;
    vector<string> messageToRemove;

    for (itMessage = messagesBuffer.begin(); itMessage != messagesBuffer.end(); itMessage++){
        unordered_map<string, WaveShortMessage>::iterator itBeacon;
        for (itBeacon = beaconNeighbors.begin(); itBeacon != beaconNeighbors.end(); itBeacon++){
            if (strcmp(itMessage->second.getTarget(), itBeacon->second.getSenderAddressString()) == 0){
                string rcvId = itMessage->second.getTarget();
                cout << "Send message to:" << rcvId << endl;
                sendWSM(updateMessageWSM(itMessage->second.dup(), rcvId));
                messageToRemove.push_back(itMessage->second.getGlobalMessageIdentificaton());

                messagesDelivered.push_back(itMessage->second.getGlobalMessageIdentificaton());
            }
        }
    }
    vector<string>::iterator itVector;
    for (itVector = messageToRemove.begin(); itVector != messageToRemove.end(); ++itVector){
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
//
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
//
    else {
        DBG << "unknown message (" << wsm->getName() << ")  received\n";
    }
    delete(msg);
}

int vehDist::getCategory(){
    // ver como definir a categoria
    if (vehID < 13) {
        return 1;
    } else if ((vehID >= 13) && (vehID < 26)){
        return 2;
    } else if ((vehID >= 26) && (vehID < 39)){
        return 3;
    } else if ((vehID >= 39) && (vehID < 52)){
        return 4;
    } else {
        return 5;
    }
}

bool vehDist::sendtoTargetbyVeh(Coord vehicleRemoteCoordBack, Coord vehicleRemoteCoordNow, int vehicleRemoteHeading, Coord targetCoord){
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

     if(distanceNow < distanceBefore){
         cout << " is less ";
         return true;
     }else if(distanceNow == distanceBefore){
         cout << " not change ";
     }else{
         cout << " is more ";
     }
     return false;
}

//###################################################  OK Function ####################################################

void vehDist::saveVehStartPosition(){
    if (source.compare("car[0]") == 0) {
        if (repeatNumber == 0) {
            myfile.open ("results/vehicle_position_initialize.txt");
        } else {
            myfile.open ("results/vehicle_position_initialize.txt", std::ios_base::app);
        }
        printHeaderfileExecution();
        myfile << "Start Position Vehicles" << endl;
    } else {
        myfile.open ("results/vehicle_position_initialize.txt", std::ios_base::app);
    }
    myfile << findHost()->getFullName() << ": "<< traci->getCurrentPosition() << endl;
    myfile.close();
}

void vehDist::setVehID() {
    if (source.size() == 6){
        vehID = atoi(source.substr(4,1).c_str());
    } else if (source.size() == 7){
        vehID = atoi(source.substr(4,2).c_str());
    } else if (source.size() == 8){
        vehID = atoi(source.substr(4,3).c_str());
    }
    //cout << findHost()->getFullName() << " vehID: " << vehID << endl;
}

void vehDist::restartFilesResult(){

    fileMessagesNameBroadcast = "results/VehBroadcastMessages.txt";
    fileMessagesNameUnicast ="results/VehMessages.txt";
    fileMessagesCount = "results/VehMessagesCount.txt";
    fileMessagesDrop = "results/VehMessagesDrop.txt";

    if ((source.compare("car[0]") == 0) && (repeatNumber == 0)) {
        openFileAndClose(fileMessagesNameBroadcast, false);
        openFileAndClose(fileMessagesNameUnicast, false);
        openFileAndClose(fileMessagesCount, false);
        openFileAndClose(fileMessagesDrop, false);
    } else if ((source.compare("car[0]") == 0) && (repeatNumber != 0)) { // open just for append
        openFileAndClose(fileMessagesNameBroadcast, true);
        openFileAndClose(fileMessagesNameUnicast, true);
        openFileAndClose(fileMessagesCount, true);
        openFileAndClose(fileMessagesDrop, true);
    }
}

void vehDist::vehUpdatePosition(){
    vehPositionBack = traci->getCurrentPosition();
    //cout << "initial positionBack :" << vehPositionBack << endl;
    updatePosVeh = new cMessage("UpdatePos evt", SEND_updatePosVeh);
    //cout << findHost()->getFullName() << " at simtime "<< simTime() << "schedule created updatePosVeh" << endl;

    //simulate asynchronous channel access
    double offSet = (dblrand())/10;
    //cout << "UpdatePosition: " <<  findHost()->getFullName() << " offset: " << offSet << endl;
    scheduleAt((simTime()+ par("timeUpdatePosition").doubleValue() + offSet), updatePosVeh);
}

void vehDist::vehSendData(){
    if (sendData){
        sendDataEvt = new cMessage("data evt", SEND_DATA_EVT);
        string host = findHost()->getFullName();
        //if(strcmp(findHost()->getFullName(), "car[0]") == 0){
        // 012345
        // car[0]
        //if (host.compare(5,1, "]") == 0){
        if (host.compare(4,2, "0]") == 0){
            generateMessage();
        }

        //simulate asynchronous channel access
        double offSet = (dblrand())/10;
        //cout << "SendData: " << findHost()->getFullName() << " offset: " << offSet << endl;
        //cout << findHost()->getFullName() << " at simtime: "<< simTime() << "schedule created sendData to: "<< (simTime() + par("dataInterval").doubleValue() + par("timeUpdatePosition").doubleValue()+ offSet) << endl;
        scheduleAt((simTime() + offSet), sendDataEvt);
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
    wsm->setSenderAddressString(findHost()->getFullName());

    //beacon don't need
    //wsm->setRecipientAddressString(); => "BROADCAST"
    //wsm->setSource(source);
    //wsm->setTarget(target);

    wsm->setSenderPos(curPosition);
    wsm->setRoadId(traci->getRoadId().c_str());
    wsm->setSenderSpeed(traci->getSpeed());

    wsm->setCategory(getCategory());
    wsm->setSenderPosBack(vehPositionBack);

    // heading 1 to 4 or 1 to 8
    wsm->setHeading(getHeading4());
    //wsm->setHeading(getHeading8());

    DBG << "Creating BeaconStatus with Priority " << priority << " at Applayer at " << wsm->getTimestamp() << std::endl;
    return wsm;
}

WaveShortMessage* vehDist::updateMessageWSM(WaveShortMessage* wsm, string rcvId) {
    wsm->setSenderAddressString(findHost()->getFullName());

    wsm->setRecipientAddressString(rcvId.c_str());
    wsm->setSenderPos(curPosition);
    wsm->setRoadId(traci->getRoadId().c_str());
    wsm->setCategory(getCategory());
    wsm->setSenderSpeed(traci->getSpeed());
    wsm->setSenderPosBack(vehPositionBack);
    wsm->setHeading(getHeading4());
    wsm->setHopCount(wsm->getHopCount() -1);

    return wsm;
}

//Generate a target in order to send a message
void vehDist::generateTarget(){
    //Set the target node to whom my message has to be delivered
    target = "rsu[0]";
}

void vehDist::generateMessage(){
    WaveShortMessage* wsm = new WaveShortMessage("beaconMessage");

    //target = rsu[0] rsu[1] or car[*].
    generateTarget();

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
    //wsm->setTimestamp(simTime() + par("dataInterval").doubleValue());
    wsm->setTimestamp(simTime());

    wsm->setSenderAddressString(findHost()->getFullName());
    // define before send
    //wsm->setRecipientAddress();
    wsm->setRecipientAddressString("initial");
    wsm->setSource(source.c_str());
    wsm->setTarget(target.c_str());

    string data = "WSMData generated by ";
    data += findHost()->getFullName();
    wsm->setWsmData(data.c_str());
    wsm->setGlobalMessageIdentificaton(to_string(vehDist::messageId).c_str());
    vehDist::messageId++;
    wsm->setHopCount(hopLimit+1); // Is hopLimit+1 because hops count is equals to routes in the path, not hops.

    wsm->setTargetPos(Coord(par("target_x"), par("target_y"), 3));

    // Adding the message on the buffer
    messagesBuffer.insert(make_pair(wsm->getGlobalMessageIdentificaton(),*wsm));
    colorCarryMessage();
    cout << endl << findHost()->getFullName() << " generated one message at simTime: " << simTime() << endl;
}

void vehDist::handleSelfMsg(cMessage* msg) {
    switch (msg->getKind()) {
        case SEND_BEACON_EVT: {
            //sendWSM(prepareWSM("beacon", beaconLengthBits, type_CCH, beaconPriority, 0, -1));
            sendWSM(prepareBeaconStatusWSM("beaconStatus", beaconLengthBits, type_CCH, beaconPriority, 0, -1));
            scheduleAt(simTime() + par("beaconInterval").doubleValue(), sendBeaconEvt);
            break;
        }
        case SEND_DATA_EVT: {
            sendDataMessage();
            //cout << "create schedule for send_data_evt" << endl;
            scheduleAt(simTime() + par("dataInterval").doubleValue(), sendDataEvt);
            break;
        }
        case SEND_updatePosVeh: {
            updatePosition();
            scheduleAt(simTime() + par("timeUpdatePosition").doubleValue(), updatePosVeh);
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

void vehDist::printBeaconNeighbors() {
    if (beaconNeighbors.empty()) {
        //cout << "beaconNeighbors from " << findHost()->getFullName() << " is empty now: " << simTime() << endl;
    } else {
        cout << endl << "beaconNeighbors from " << findHost()->getFullName() << " at " << simTime() << endl;
        unordered_map<string, WaveShortMessage>::iterator it;
        for (it = beaconNeighbors.begin(); it != beaconNeighbors.end(); it++) {
            cout << " Id(veh): " << it->first << endl;
            cout << " PositionBack: " << it->second.getSenderPosBack() << endl;
            cout << " Position: " << it->second.getSenderPos() << endl;
            cout << " Speed: " << it->second.getSenderSpeed() << endl;
            cout << " Category: " << it->second.getCategory() << endl;
            cout << " RoadId: " << it->second.getRoadId() << endl;
            cout << " Heading: " << it->second.getHeading() << endl;
            cout << " Timestamp: " << it->second.getTimestamp()<< endl;
            cout << endl;
        }
    }
}

unsigned int vehDist::getHeading4(){
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

    if ((angle >= 315 && angle < 360) || (angle >= 0 && angle < 45)){
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

unsigned int vehDist::getHeading8(){
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

    if ((angle >= 337.5 && angle < 360) || (angle >= 0 && angle < 22.5)){
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

void vehDist::updatePosition(){
//    cout << "Vehicle: " << findHost()->getFullName();
//    cout << " Update position: " <<  "time: "<< simTime() << " next update: ";
//    cout << (simTime() + par("timeUpdatePosition").doubleValue()) << endl;
//    cout << "positionBack :" << vehPositionBack;
    vehPositionBack = traci->getPositionAt(simTime() - par("timeUpdatePosition").doubleValue());
//    cout << " and update positionBack :" << vehPositionBack << endl << endl;
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
