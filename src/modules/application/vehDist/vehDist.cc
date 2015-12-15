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

        sendDataEvt = new cMessage("data evt", SEND_DATA_EVT);
        cout << endl << findHost()->getFullName() << " entered in the scenario" << endl;
        if(strcmp(findHost()->getFullName(), "car[0]") == 0 && sendData){
            cout << endl << findHost()->getFullName() << ": Generating the message" << endl;
            WaveShortMessage* wsm;
            wsm = generateMessage();
            sendWSM(wsm);
        }

        if (sendData) {
            cout << findHost()->getFullName()<<" at simtime "<<simTime()<<" creating the schedule to send data"<<endl;
            scheduleAt(simTime() + par("dataInterval").doubleValue(), sendDataEvt);
            //cout << "schedule created sendData" << endl;
        }

        vehPositionBack = traci->getCurrentPosition();
        //cout << "initial positionBack :" << vehPositionBack << endl;
        updatePosVeh = new cMessage("UpdatePos evt", SEND_updatePosVeh);
        sendUpdatePos =  par("sendUpdatePos").boolValue();
        if (sendUpdatePos){
            scheduleAt(simTime()+ par("timeUpdatePosition").doubleValue(), updatePosVeh);
            //cout << "schedule created sendUpdatePos" << endl;
        }

    }
}

void vehDist::onBeacon(WaveShortMessage* wsm) {
    //   countMessage++;
    //   contextLocalMessageBuffer.insert(make_pair(countMessage, *wsm));
    //   sendData();

    //cout << "before insert one beacon. Id(veh): " << wsm->getVehicleId() << " timestamp: " << wsm->getTimestamp() << endl;
    //printBeaconNeighbors();
    auto it = beaconNeighbors.find(to_string(wsm->getSenderAddress()));
    if (it != beaconNeighbors.end()){
        //cout << "Update wsm beacon (Key: " << to_string(wsm->getSenderAddress()) << ", timestamp: " << beaconNeighbors[to_string(wsm->getSenderAddress())].getTimestamp() << ")" << endl;
        it->second = *wsm;
    } else {
        beaconNeighbors.insert(make_pair(to_string(wsm->getSenderAddress()), *wsm));
    }
    //cout << "after insert one beacon" << endl;
    //printBeaconNeighbors();
}

//Generate a target in order to send a message
void vehDist::generateTarget(){
    //Set the target node to whom my message has to be delivered
    target = "rsu[0]";
    //cout << findHost()->getFullName() << "generating a RSU target to its message (" << source << " -> " << target << ")"<< endl;
}

WaveShortMessage* vehDist::generateMessage(){
    //Set the target node to whom my message has to be delivered
    //target = rsu[0] rsu[1] or car[*].
    generateTarget();

    WaveShortMessage* wsm = new WaveShortMessage("data");

    wsm->setName("data");
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
    wsm->setTimestamp(simTime());
    wsm->setSenderAddress(MACToInteger(myMac));
    wsm->setRecipientAddress(BROADCAST);
    wsm->setSource(source.c_str());
    wsm->setTarget(target.c_str());
    wsm->setSenderPos(curPosition);
    wsm->setSerial(2);
    wsm->setSummaryVector(false);
    string data = "WSMData generated by ";
    data += findHost()->getFullName();
    wsm->setWsmData(data.c_str());
    //wsm->setLocalMessageIdentificaton(to_string(vehDist::messageId).c_str());
    wsm->setGlobalMessageIdentificaton((to_string(vehDist::messageId) + to_string(MACToInteger(myMac))).c_str());
    wsm->setHopCount(hopCount);
    vehDist::messageId++;
    cout<<"Before adding the message on the buffer on generateMessage simtime "<<simTime()<<endl;
    contextLocalMessageBuffer.insert(make_pair(wsm->getGlobalMessageIdentificaton(),*wsm));
    cout<<"After adding the message on the buffer on generateMessage simtime "<<simTime()<<endl;

    cout<<endl<<"Message generated by " << findHost()->getFullName() << " at simTime " << simTime() << endl;
    return wsm;
}

unsigned int vehDist::MACToInteger(WaveAppToMac1609_4Interface* myMac){
    unsigned int macInt;
    std::stringstream ss;
    ss << std::hex << myMac;
    ss >> macInt;
    return macInt;
}

void vehDist::handleSelfMsg(cMessage* msg) {
    switch (msg->getKind()) {
        case SEND_BEACON_EVT: {
            //sendWSM(prepareWSM("beacon", beaconLengthBits, type_CCH, beaconPriority, 0, -1));
            //sendWSM(prepareWSM("beacon", beaconLengthBits, type_CCH, beaconPriority, BROADCAST,2));
            sendWSM(prepareBeaconWSM("beacon", beaconLengthBits, type_CCH, beaconPriority, BROADCAST,2));
            scheduleAt(simTime() + par("beaconInterval").doubleValue(), sendBeaconEvt);
            break;
        }
        case SEND_DATA_EVT:{
            cout<<endl<<findHost()->getFullName()<<" SEND_DATA_EVT"<<endl;
            //verify if the vehicle received any message since the beginning
            if(contextLocalMessageBuffer.empty()){
                cout<<findHost()->getFullName() <<"buffer is empty at simtime: "<< simTime()<<endl;
            }else if (!contextLocalMessageBuffer.empty()){
                cout<<findHost()->getFullName()<<" buffer isn't empty"<<" at "<<simTime()<<endl;
                cout<<"(sendData elseif)GlobalMessageID "<<contextLocalMessageBuffer.begin()->first<<" ";
                //cout<<"(sendData elseif)Source "<<contextLocalMessageBuffer.begin()->second->getSource()<<endl;
                //if the Buffer isn't empty send all the data in the buffer
                cout<<endl<<findHost()->getFullName()<<"before the iterator at "<<simTime()<<endl;
                map<string, WaveShortMessage>::iterator it;
                cout<<endl<<findHost()->getFullName()<<"before the for at "<<simTime()<<endl;
                for(it = contextLocalMessageBuffer.begin(); it != contextLocalMessageBuffer.end(); it++){
                    cout<<findHost()->getFullName()<<"beginning of the for at "<<simTime()<<endl;
                    cout<<findHost()->getFullName()<<" messageID "<<contextLocalMessageBuffer.begin()->first<<" "<<contextLocalMessageBuffer.begin()->second.getSource()<<endl;
                    printContextLocalMessageBuffer();
                    cout<<" MessageID: "<<contextLocalMessageBuffer.begin()->first<<" Source: " <<contextLocalMessageBuffer.begin()->second.getSource();
                    cout<<" Sender: "<< contextLocalMessageBuffer.begin()->second.getSenderAddress()<<"("<<findHost()->getFullName()<<")"<<endl;
                    t_channel channel = dataOnSch ? type_SCH : type_CCH;
                    switch (channel) {
                        case type_SCH: contextLocalMessageBuffer.begin()->second.setChannelNumber(Channels::SCH1); break; //will be rewritten at Mac1609_4 to actual Service Channel. This is just so no controlInfo is needed
                        case type_CCH: contextLocalMessageBuffer.begin()->second.setChannelNumber(Channels::CCH); break;
                    }
                    sendWSM(prepareWSM("data", dataLengthBits, channel, contextLocalMessageBuffer.begin()->second.getPriority(), contextLocalMessageBuffer.begin()->second.getRecipientAddress(),contextLocalMessageBuffer.begin()->second.getSerial()));
                }
            }else{
                cout<<endl<<"contextLocalMessageBuffer can't be seen in this function!"<<endl;
            }
            cout<<findHost()->getFullName()<<"Creating the schedule at "<<simTime()<<endl;
            scheduleAt(simTime() + par("dataInterval").doubleValue(), sendDataEvt);
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

void vehDist::printContextLocalMessageBuffer() {
    if (contextLocalMessageBuffer.empty()) {
        cout << "ContextLocalMessageBuffer from " << findHost()->getFullName() << " is empty now " << endl;
    } else {
        cout << "Printing the contextLocalMessageBuffer from " << findHost()->getFullName() << "(" << MACToInteger(myMac) <<"):" << endl;
        map<string, WaveShortMessage>::iterator it;
        for(it = contextLocalMessageBuffer.begin(); it != contextLocalMessageBuffer.end(); it++) {
            cout << " Id: " << it->first <<" "<< "- Message Content: " << it->second.getWsmData() << " Source: " << it->second.getSource() << " Target: " << it->second.getTarget() << " Timestamp: " << it->second.getTimestamp() << " HopCount: " << it->second.getHopCount() << endl;
        }
    }
}

void vehDist::printBeaconNeighbors() {
    if (beaconNeighbors.empty()) {
        cout << "beaconNeighbors from " << findHost()->getFullName() << " is empty now " << endl;
    } else {
        cout << "Printing the beaconNeighbors from " << findHost()->getFullName() << "(" << MACToInteger(myMac) <<"):" << endl;
        unordered_map<string, WaveShortMessage>::iterator it;
        for(it = beaconNeighbors.begin(); it != beaconNeighbors.end(); it++) {
            cout << " Id(veh): " << it->first << endl;
            cout << " VehicleId: " << it->second.getVehicleId() << endl;
            cout << " position: " << it->second.getSenderPos() << endl;
            cout << " positionBack: " << it->second.getSenderPosBack() << endl;
            cout << " speed: " << it->second.getSenderSpeed() << endl;
            cout << " category: " << it->second.getCategory() << endl;
            cout << " roadId: " << it->second.getRoadId() << endl;
            cout << " heading: " << it->second.getHeading() << endl;
            cout << " timestamp: " << it->second.getTimestamp()<< endl << endl;
        }
    }
}

void vehDist::sendDataTest() {
    sendWSM(prepareWSM("data", dataLengthBits, type_CCH, dataPriority, 0, -1));
}

void vehDist::onData(WaveShortMessage* wsm) {
    printContextLocalMessageBuffer();

    //    map <string, WaveShortMessage>::iterator it;
    //    cout << endl << "Print messages in buffer:";
    //    for (it = contextLocalMessageBuffer.begin(); it != contextLocalMessageBuffer.end(); it++){
    //        cout << " Key: " << it->first<< " Sender: " << it->second.getSenderAddress() << endl;
    //    }

    //default
    //    findHost()->getDisplayString().updateWith("r=16,green");
    //    annotations->scheduleErase(1, annotations->drawLine(wsm->getSenderPos(), traci->getPositionAt(simTime()), "blue"));
    //
    //    if (traci->getRoadId()[0] != ':') traci->commandChangeRoute(wsm->getWsmData(), 9999);
    //    if (!sentMessage) sendMessage(wsm->getWsmData());


    findHost()->bubble("Received data");
    cout<<findHost()->getFullName()<<" entering on onData"<<" at simtime: "<<simTime()<<endl;
    //verify if this is the recipient of the message
    if((wsm->getRecipientAddress() == 268435455)||(wsm->getRecipientAddress() == MACToInteger(myMac))){

        //verify if the message isn't in the buffer
        map<string, WaveShortMessage>::const_iterator search = contextLocalMessageBuffer.find(wsm->getGlobalMessageIdentificaton());
        if((contextLocalMessageBuffer.empty())||(search == contextLocalMessageBuffer.end())){
            cout<<findHost()->getFullName()<<" buffer empty or the vehicle don't have the message at simtime "<<simTime()<<endl;

            //duplicating the message, so this vehicle has his own copy of the message
            WaveShortMessage* wsmdup = wsm->dup();

            //cout<<endl<<findHost()->getFullName()<<"("<<MACToInteger()<<")"<<": inside of the onData"<<endl;
            cout<<"Message ID: "<<wsmdup->getGlobalMessageIdentificaton()<<" WSM data: "<<wsmdup->getWsmData()<<"Received at time: "<<simTime()<<endl;

            //measure the distance between this and the RSU
            Coord posCar = traci->getCurrentPosition();
            //Coord posRSU = Coord(240, 1000, 3);
            Coord posRSU = wsmdup->getTargetPos();
            double distance;
            distance = TraCIMobilityAccess().get(getParentModule())->commandDistanceRequest(posCar, posRSU, false);
            //cout<<"I am: "<< findHost()->getFullName() << " at position: "<< posCar <<" timeStamp: "<<simTime()<<" Dist to RSU: "<<distance<<endl;

            //before sending the message the hopCount and the senderAddress have to be setted
            //counting the hop from the previous node to this
            wsmdup->setHopCount(wsmdup->getHopCount()-1);

            //   if(distance <= 250){
            //      wsmdup->setRecipientAddress(MACToInt(wsmdup->getTarget()));
            //   }

            wsmdup->setSenderAddress(MACToInteger(myMac));
            //cout<<"New sender: "<<wsm->getSenderAddress()<<endl;

            //add the msg in the  vehicle buffer
            contextLocalMessageBuffer.insert(make_pair(wsmdup->getGlobalMessageIdentificaton(),*wsmdup));

            cout<<" (onData)GlobalMessageID "<<contextLocalMessageBuffer.begin()->first<<" ";
            cout<<" (onData)Source "<<contextLocalMessageBuffer.begin()->second.getSource()<<endl;

            cout << "I am: "<<findHost()->getFullName() << " forwarding the message from (source): "<<wsmdup->getSource()<<" sender: "<<wsmdup->getSenderAddress()<<"hopCount:"<< wsmdup->getHopCount()<<endl;
            delete wsmdup;
            //move in all the position of the data structure printing, //sending and excluding the message from the buffer
            // map<string, WaveShortMessage*>::iterator it;
            // for(it = contextLocalMessageBuffer.begin(); it != contextLocalMessageBuffer.end(); it++){

            //wsmReceived = it->second;
            //cout<<"Received: "<<wsmReceived->getSenderAddress()<<endl;
            //cout<<" Received updated! "<<endl;
            //sendWSM(it->second);
            //cout<<findHost()->getFullName()<<"sending the message at the time: "<<simTime()<<endl;
            //sendWSM(it->second);
            //contextLocalMessageBuffer.erase(it);

            //}
        }else{
            if(search != contextLocalMessageBuffer.end()){
                cout<<findHost()->getFullName()<<" message is on the buffer at simtime "<<simTime()<<endl;
            }else{
                cout<<"There's something wrong!"<<endl;
            }
        }
    }
    //cout<<"Printing the contextLocalMessageBuffer after erase a element"<<endl;
    //printContextLocalMessageBuffer();

    //sendWSM(wsm);

    //if(wsm->getRecipientAddress() == MACToInteger()){

    //Putting the message in the contextLocalMessageBuffer
    //contextLocalMessageBuffer.insert(make_pair<string, WaveShortMessage>(wsm->getGlobalMessageIdentificaton(),wsm));


    //diminuir hopCount

    //se hopcount >=1
    //se o alvo esta na lista de vizinhos, enviar pra ele e diminuir o hopcount


    //else escolher na lista de vizinhos o que tem menor distancia até o nó
    //percorrer todas as posições do vetor, calculando a distância até o target
    //substituindo o valor da variavel "menor" pelo index da posição correspondente do vetor.
    //o recipient da mensagem será o menor.

    //}


    //se target ta na lista de vizinhos, mandar pra ele
    //if(strcmp(findHost()->getFullName(), wsm->getTarget()) == 0){}

}

void vehDist::sendMessage(std::string blockedRoadId) {
    sentMessage = true;

    t_channel channel = dataOnSch ? type_SCH : type_CCH;
    WaveShortMessage* wsm = prepareWSM("data", dataLengthBits, channel, dataPriority, -1,2);
    wsm->setWsmData(blockedRoadId.c_str());
    sendWSM(wsm);
}
void vehDist::receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj) {
    Enter_Method_Silent();
    if (signalID == mobilityStateChangedSignal) {
        handlePositionUpdate(obj);
    }
    else if (signalID == parkingStateChangedSignal) {
        handleParkingUpdate(obj);
    }
}
void vehDist::handleParkingUpdate(cObject* obj) {
    isParking = traci->getParkingState();
    if (sendWhileParking == false) {
        if (isParking == true) {
            (FindModule<BaseConnectionManager*>::findGlobalModule())->unregisterNic(this->getParentModule()->getSubmodule("nic"));
        }
        else {
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
            findHost()->getDisplayString().updateWith("r=16,red");
            if (!sentMessage) sendMessage(traci->getRoadId());
        }
    }
    else {
        lastDroveAt = simTime();
    }
}
void vehDist::sendWSM(WaveShortMessage* wsm) {
    if (isParking && !sendWhileParking) return;
    sendDelayedDown(wsm,individualOffset);
}

WaveShortMessage* vehDist::prepareBeaconWSM(std::string name, int lengthBits, t_channel channel, int priority, unsigned int rcvId, int serial) {
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
    wsm->setTimestamp(simTime());
    wsm->setSenderAddress(MACToInteger(myMac));
    wsm->setRecipientAddress(rcvId);
    //wsm->setSource(source);
    //wsm->setTarget(target);
    wsm->setSenderPos(curPosition);
    wsm->setSerial(serial);
    wsm->setRoadId(traci->getRoadId().c_str());
    wsm->setSenderSpeed(traci->getSpeed());
    wsm->setVehicleId(traci->getId());

    // ver como definir o id, traci->getId(), e a categoria
    if (traci->getId() < 5) {
        wsm->setCategory(1);
    }
    else if (traci->getId() > 5) {
         wsm->setCategory(2);
    }
    else if (traci->getId() < 10) {
             wsm->setCategory(3);
    }
    else {
        wsm->setCategory(4);
    }

    //wsm->setTargetPos(Coord(par("target_x"), par("target_y"), 3));
    wsm->setSenderPosBack(vehPositionBack);
    wsm->setHeading(getHeading());

    if (name == "beacon") {
        DBG << "Creating Beacon with Priority " << priority << " at Applayer at " << wsm->getTimestamp() << std::endl;
    }else if (name == "data") {
        DBG << "Creating Data with Priority " << priority << " at Applayer at " << wsm->getTimestamp() << std::endl;
    }
    return wsm;
}

unsigned int vehDist::getHeading(){
    // return angle <0º - 359º>

    // marcospaiva.com.br/images/rosa_dos_ventos%2002.GIF
    // marcospaiva.com.br/localizacao.htm

    // (angle >= 337.5 && angle < 360) || (angle >= 0 && angle < 22.5) return 1; // L or E => 0º
    // angle >= 22.5 && angle < 67.5                                   return 2; // NE => 45º
    // angle >= 67.5  && angle < 112,5                                 return 3; // N => 90º
    // angle >= 112.5  && angle < 157.5                                return 4; // NO => 135º
    // angle >= 157,5  && angle < 202,5                                return 5; // O or W => 180º
    // angle >= 202.5  && angle < 247.5                                return 6; // SO => 225º
    // angle >= 292.5  && angle < 337.5                                return 8; // SE => 315º
    // angle >= 360 return 9; // Error

    double angle;
    if (traci->getAngleRad() < 0) // radians are negtive, so degrees negative
        angle = (((traci->getAngleRad() + 2*M_PI ) * 180)/ M_PI);
    else //radians are positive, so degrees positive
        angle = ((traci->getAngleRad() * 180) / M_PI);

    if ((angle >= 337.5 && angle < 360) || (angle >= 0 && angle < 22.5)){
        //cout << "Angle: " << angle << " Heading " << 1 << endl;
        return 1; // L or E => 0º
    }
    else if (angle >= 22.5 && angle < 67.5) {
        //cout << "Angle (getHeading): " << angle << " Heading " << 2 << endl;
        return 2; // NE => 45º
    }
    else if (angle >= 67.5  && angle < 112.5) {
        //cout << "Angle (getHeading): " << angle << " Heading " << 3 << endl;
        return 3; // N => 90º
    }
    else if (angle >= 112.5  && angle < 157.5) {
        //cout << "Angle (getHeading): " << angle << " Heading " << 4 << endl;
        return 4; // NO => 135º
    }
    else if (angle >= 157.5  && angle < 202.5) {
        //cout << "Angle (getHeading): " << angle << " Heading " << 5 << endl;
        return 5; // O or W => 180º
    }
    else if (angle >= 202.5  && angle < 247.5) {
        //cout << "Angle (getHeading): " << angle << " Heading " << 6 << endl;
        return 6; // SO => 225º
    }
    else if (angle >= 247.5  && angle < 292.5) {
        //cout << "Angle (getHeading): " << angle << " Heading " << 7 << endl;
        return 7; // S => 270º
    }
    else if (angle >= 292.5  && angle < 337.5) {
        //cout << "Angle (getHeading): " << angle << " Heading " << 8 << endl;
        return 8; // SE => 315º
    }
    else {
        //cout << "Error angle (getHeading): " << angle << " Heading of error " << 9 << endl;
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
