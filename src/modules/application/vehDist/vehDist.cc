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
        hopCount = par("hopCount").longValue();

        vehSendData();
        vehUpdatePosition();
        restartFilesResult();

        //saveVehStartPosition();

        //cout << endl << findHost()->getFullName() << " entered in the scenario" << endl;
        //cout << endl << findHost()->getFullName() << " myId " <<  myId << endl;
    }
}

void vehDist::onBeacon(WaveShortMessage* wsm) {
    //cout << "before insert one beacon. Id(veh): " << wsm->getVehicleId() << " timestamp: " << wsm->getTimestamp() << endl;
    auto it = beaconNeighbors.find(wsm->getSenderAddressString());
    if (it != beaconNeighbors.end()){
        //cout << "Update wsm beacon (Key: " << to_string(wsm->getSenderAddress()) << ", timestamp: " << beaconNeighbors[to_string(wsm->getSenderAddress())].getTimestamp() << ")" << endl;
        it->second = *wsm;
    } else {
        countBeacon++;
        beaconNeighbors.insert(make_pair(wsm->getSenderAddressString(), *wsm));
        ordemBeacons.insert(make_pair(countBeacon, wsm->getSenderAddressString()));
//        if (countBeacon > par("beaconBufferSize").doubleValue()){
//            cout << "remove beacon" << endl;
//            printBeaconNeighbors();
//            removeBeaconOutSizeBuffer();
//            printBeaconNeighbors();
//            cout << "countBeacon" << countBeacon << endl;
//        }
    }

    //cout << "veh: " << findHost()->getFullName() << " countBeacon: " << countBeacon << endl;
    //printOrdemBeacons();
    //printBeaconNeighbors();
}

void vehDist::removeBeaconOutSizeBuffer() {
    cout << "on removeBeaconOutSizeBuffer" << endl;
    if (beaconNeighbors.empty()) {
        cout << "beaconNeighbors from " << findHost()->getFullName() << " is empty now " << endl;
    } else {
//        printMessagesBuffer();
//        printOrdemMessages();
        unordered_map<string, WaveShortMessage>::iterator it;
        for (it = beaconNeighbors.begin(); it != beaconNeighbors.end(); it++) {
            bool t = true;
            //if ((it->second.getTimestamp() + par("timeDeleteMessage").doubleValue()) <= simTime()){
            if (countBeacon >= par("beaconBufferSize").doubleValue()){
                if(!t)
                    cout << endl << endl << endl << "dois beacons para apagar? como?" << endl << endl << endl;

//                cout << " found search on for, simTime(): " << simTime() << endl;
//                cout << "messagesBuffer.size(): " << messagesBuffer.size() << endl;
//                cout << "ordemMessages.size(): " << ordemMessages.size() << endl;

                cout << "change this" << endl;
                if (it->second.getGlobalMessageIdentificaton() != ordemMessages[0]) {
//                    cout << "it->second.getGlobalMessageIdentificaton() != ordemMessages[0], message to delete is not the last" << endl;
//                    cout << "it " << it->second.getGlobalMessageIdentificaton() << " od " << ordemMessages[0] << endl;

                    unordered_map<int, string>::iterator it2;
                    int now = countBeacon;
                    for (it2 = ordemMessages.begin(); it2 != ordemMessages.end(); it2++) {
                    //for (int i = 1; i <= countMessage; i++) {
                        if (it2->second == messagesBuffer[ordemMessages[1]].getGlobalMessageIdentificaton()){
//                            cout << "Encontrado " << it2->first << " " << it2->second << endl;
                            now = it2->first;
                            break;
                        }
                    }
                    for (; now < countMessage; now++) {
//                        cout << "moving messages " << now  << " recive " << (now+1) << endl;
//                        cout << ordemMessages[now] << " " << ordemMessages[now+1] << endl;
//                        cout << "messagesBuffer[ordemMessages[now]] "<< " " << messagesBuffer[ordemMessages[now]] << endl;
                        messagesBuffer[ordemMessages[now]] = messagesBuffer[ordemMessages[now+1]];
                        ordemMessages[now] = ordemMessages[now+1];
//                        cout << "moved" << endl;
//                        printMessagesBuffer();
//                        printOrdemMessages();
//                        cout << ordemMessages[now] << " " << ordemMessages[now+1] << endl;
                    }
                }
//                cout << "countMessage: " << countMessage << endl;
//                cout << "messagesBuffer[ordemMessages[countMessage]].getGlobalMessageIdentificaton()  " << messagesBuffer[ordemMessages[countMessage]].getGlobalMessageIdentificaton() << endl;
//                cout << "ordemMessages[ordemMessages] " << ordemMessages[countMessage] << endl;
                messagesBuffer.erase(ordemMessages[countMessage]);
                ordemMessages.erase(countMessage);
                t = false;
//                cout << "messagesBuffer.size(): " << messagesBuffer.size() << endl;
//                cout << "ordemMessages.size(): " << ordemMessages.size() << endl;
                countMessage--;
//                cout << "countMessage: " << countMessage << endl;
//                printMessagesBuffer();
//                cout << "end" << endl;
                break;
            }
        }
    }
}

void vehDist::onData(WaveShortMessage* wsm) {
    //cout << " in onData" << endl;
    //printMessagesBuffer();

    if (strcmp(wsm->getRecipientAddressString(), findHost()->getFullName()) == 0){
        recordOnFileMessages(wsm);
    }else{
    //} else if (strcmp(wsm->getRecipientAddressString(), "BROADCAST") == 0){
        recordOnFileMessagesBroadcast(wsm);
    //}
    }



    //findHost()->bubble("Received data");
    //verify if this is the recipient of the message
    //if (strcmp(wsm->getRecipientAddressString(), "BROADCAST") == 0)
    if (strcmp(wsm->getRecipientAddressString(), findHost()->getFullName())) {

        //verify if the message isn't in the buffer
        unordered_map<string, WaveShortMessage>::const_iterator search = messagesBuffer.find(wsm->getGlobalMessageIdentificaton());
        if ((messagesBuffer.empty()) || (search == messagesBuffer.end())) {
            cout << findHost()->getFullName() << " buffer empty or the vehicle don't have the message at simtime: " << simTime() << endl;

            //duplicating the message, so this vehicle has his own copy of the message
            WaveShortMessage* wsmdup = wsm->dup();

            //cout<<endl<<findHost()->getFullName()<<"("<<MACToInteger()<<")"<<": inside of the onData"<<endl;
            //cout<<"Message ID: " << wsmdup->getGlobalMessageIdentificaton() << " WSM data: "<<wsmdup->getWsmData() << " Received at time: " << simTime() << endl;

//            //measure the distance between this and the RSU
//            Coord posCar = traci->getCurrentPosition();
//            //Coord posRSU = Coord(240, 1000, 3);
//            Coord posRSU = wsmdup->getTargetPos();
//            double distance;
//            distance = TraCIMobilityAccess().get(getParentModule())->commandDistanceRequest(posCar, posRSU, false);
//            //cout<<"I am: "<< findHost()->getFullName() << " at position: "<< posCar <<" timeStamp: "<<simTime()<<" Dist to RSU: "<<distance<<endl;

            //before sending the message the hopCount and the senderAddress have to be setted
            //counting the hop from the previous node to this
            //wsmdup->setHopCount(wsmdup->getHopCount()-1);

            //wsmdup->setSenderAddress(MACToInteger(myMac));

            // delete old messages
            deleteMessage();

            cout << "number os messages: " << countMessage << endl;
            if (countMessage >= par("messageBufferSize").doubleValue()){
                removeMessageOutSizeBuffer();
            }
            cout << "number os messages: " << countMessage << endl;

            //add the msg in the  vehicle buffer
            wsmdup->setTimestamp(simTime());
            messagesBuffer.insert(make_pair(wsmdup->getGlobalMessageIdentificaton(),*wsmdup));

            countMessage++;
            ordemMessages.insert(make_pair(countMessage, wsmdup->getGlobalMessageIdentificaton()));

//            cout<<" (onData)GlobalMessageID " << messagesBuffer.begin()->first<<" ";
//            cout<<" (onData)Source " << messagesBuffer.begin()->second.getSource()<<endl;

            cout << "I am: "<<findHost()->getFullName() << " recive the message from (source): ";
            cout << wsmdup->getSource() << " sender: " << wsmdup->getSenderAddressString() << " hopCount: " << wsmdup->getHopCount() << endl;
            delete wsmdup;
            //move in all the position of the data structure printing, //sending and excluding the message from the buffer
            // map<string, WaveShortMessage*>::iterator it;
            // for(it = messagesBuffer.begin(); it != messagesBuffer.end(); it++){

            //wsmReceived = it->second;
            //cout<<"Received: "<<wsmReceived->getSenderAddress()<<endl;
            //cout<<" Received updated! "<<endl;
            //sendWSM(it->second);
            //cout<<findHost()->getFullName()<<"sending the message at the time: "<<simTime()<<endl;
            //sendWSM(it->second);
            //messagesBuffer.erase(it);

            //}
        }else if(search != messagesBuffer.end()){
                cout<<findHost()->getFullName() << " message is on the buffer at simtime " << simTime() << endl;
        }
    }
    //cout<<"Printing the messagesBuffer after erase a element"<<endl;
    //printmessagesBuffer();

    //sendWSM(wsm);

    //if(wsm->getRecipientAddress() == MACToInteger()){

    //Putting the message in the messagesBuffer
    //messagesBuffer.insert(make_pair<string, WaveShortMessage>(wsm->getGlobalMessageIdentificaton(),wsm));


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

void vehDist::sendDataMessage() {
    //cout << endl<<findHost()->getFullName() << " in sendDataMessage" << endl;
    //verify if the vehicle received any message since the beginning
    if (messagesBuffer.empty()) {
        cout << findHost()->getFullName() << " messagesBuffer is empty at simtime "<< simTime() << endl;
    } else {
        cout << findHost()->getFullName() << " messagesBuffer with messages " << " at "<< simTime() << endl;

        if (beaconNeighbors.empty()) {
            cout << "beaconNeighbors on sendDataMessage from " << findHost()->getFullName() << " is empty now " << endl;
        } else {

            unordered_map<string, WaveShortMessage>::iterator messageIt;
            for (messageIt = messagesBuffer.begin(); messageIt != messagesBuffer.end(); messageIt++) {
                unordered_map<int, string>::iterator beaconIt;
                for (beaconIt = ordemBeacons.begin(); beaconIt != ordemBeacons.end(); beaconIt++) {
                    if (strcmp(messageIt->second.getTarget(), beaconIt->second.c_str()) == 0){
                        string rcvId = messageIt->second.getTarget();
                        cout << "Send message to:" << rcvId << endl;
                        sendWSM(updateMessageWSM(messageIt->second.dup(), rcvId));
                        //remove message send
                    }
                    cout << "aquiaqui: " << beaconIt->second.c_str() << endl;
                }
            }


            bool send = false;

            unordered_map<string, WaveShortMessage>::iterator it;
            for (it = beaconNeighbors.begin(); it != beaconNeighbors.end(); it++) {

                //test send distante e heading to target
                send = sendtoTargetbyVeh(it->second.getSenderPosBack(), it->second.getSenderPos(), it->second.getHeading(), messagesBuffer[ordemMessages[countMessage]].getTargetPos());

                cout << " SenderPosBack: " << it->second.getSenderPosBack() << endl;
                cout << " SenderPos: " << it->second.getSenderPos() << endl;
                cout << " Heading: " << it->second.getHeading() << endl;
                cout << " TargetPos: " << messagesBuffer[ordemMessages[countMessage]].getTargetPos() << endl;
                if (send)
                    break;
            }

            // unsigned int rcvId = BROADCAST;
            string id = ordemMessages[countMessage];
            if (send){
                string rcvId = it->second.getSenderAddressString();
                cout << "Send message to:" << rcvId << endl;
                sendWSM(updateMessageWSM(messagesBuffer[id].dup(), rcvId));
            }
            else{
                cout << "Not send message to: " << endl;
            }
            //printMessagesBuffer();
            cout << "("<<findHost()->getFullName()<<")" << endl;
            cout << " MessageID: " << messagesBuffer[id].getGlobalMessageIdentificaton() << endl;
            cout << " Source: " << messagesBuffer[id].getSource() << endl;
            cout << " SenderString: " << messagesBuffer[id].getSenderAddressString() << endl;
            cout << " Message Content: " << messagesBuffer[id].getWsmData() << endl;
            cout << " Target: " << messagesBuffer[id].getTarget() << endl;
            cout << " Timestamp: " << messagesBuffer[id].getTimestamp() << endl;
            cout << " MessageTimestampGenerate: " << messagesBuffer[id].getMessageTimestampGenerate() << endl;
            cout << " HopCount: " << messagesBuffer[id].getHopCount() << endl;
        }
    }
}

void vehDist::deleteMessage() {
    cout << "on deleteMessage" << endl;
    if (messagesBuffer.empty()) {
        cout << "messagesBuffer from " << findHost()->getFullName() << " is empty now " << endl;
    } else {
        //        printMessagesBuffer();
        //        printOrdemMessages();
        unordered_map<string, WaveShortMessage>::iterator it;
        for (it = messagesBuffer.begin(); it != messagesBuffer.end(); it++) {
            if ((it->second.getTimestamp() + par("timeDeleteMessage").doubleValue()) <= simTime()){
                //                cout << " found search on for, simTime(): " << simTime() << endl;
                //                cout << "messagesBuffer.size(): " << messagesBuffer.size() << endl;
                //                cout << "ordemMessages.size(): " << ordemMessages.size() << endl;

                if (it->second.getGlobalMessageIdentificaton() != ordemMessages[0]) {
                    //                    cout << "it->second.getGlobalMessageIdentificaton() != ordemMessages[0], message to delete is not the last" << endl;
                    //                    cout << "it " << it->second.getGlobalMessageIdentificaton() << " od " << ordemMessages[0] << endl;

                    unordered_map<int, string>::iterator it2;
                    int now = countMessage;
                    for (it2 = ordemMessages.begin(); it2 != ordemMessages.end(); it2++) {
                        //for (int i = 1; i <= countMessage; i++) {
                        if (it2->second == messagesBuffer[ordemMessages[1]].getGlobalMessageIdentificaton()){
                            //                            cout << "Encontrado " << it2->first << " " << it2->second << endl;
                            now = it2->first;
                            break;
                        }
                    }
                    for (; now < countMessage; now++) {
                        //                        cout << "moving messages " << now  << " recive " << (now+1) << endl;
                        //                        cout << ordemMessages[now] << " " << ordemMessages[now+1] << endl;
                        //                        cout << "messagesBuffer[ordemMessages[now]] "<< " " << messagesBuffer[ordemMessages[now]] << endl;
                        messagesBuffer[ordemMessages[now]] = messagesBuffer[ordemMessages[now+1]];
                        ordemMessages[now] = ordemMessages[now+1];
                        //                        cout << "moved" << endl;
                        //                        printMessagesBuffer();
                        //                        printOrdemMessages();
                        //                        cout << ordemMessages[now] << " " << ordemMessages[now+1] << endl;
                    }
                }
                //                cout << "countMessage: " << countMessage << endl;
                //                cout << "messagesBuffer[ordemMessages[countMessage]].getGlobalMessageIdentificaton()  " << messagesBuffer[ordemMessages[countMessage]].getGlobalMessageIdentificaton() << endl;
                //                cout << "ordemMessages[ordemMessages] " << ordemMessages[countMessage] << endl;
                messagesBuffer.erase(ordemMessages[countMessage]);
                ordemMessages.erase(countMessage);
                //                cout << "messagesBuffer.size(): " << messagesBuffer.size() << endl;
                //                cout << "ordemMessages.size(): " << ordemMessages.size() << endl;
                countMessage--;
                //                cout << "countMessage: " << countMessage << endl;
                //                printMessagesBuffer();
                //                cout << "end" << endl;
                break;
            }
        }
    }
}

void vehDist::removeMessageOutSizeBuffer() {
    cout << "on removeMessageOutSizeBuffer" << endl;
    if (messagesBuffer.empty()) {
        cout << "messagesBuffer from " << findHost()->getFullName() << " is empty now " << endl;
    } else {
//        printMessagesBuffer();
//        printOrdemMessages();
        unordered_map<string, WaveShortMessage>::iterator it;
        for (it = messagesBuffer.begin(); it != messagesBuffer.end(); it++) {
            bool t = true;
            //if ((it->second.getTimestamp() + par("timeDeleteMessage").doubleValue()) <= simTime()){
            if (countMessage >= par("messageBufferSize").doubleValue()){
                if(!t)
                    cout << endl << endl << endl << "duas mensagens para apagar? como?" << endl << endl << endl;

//                cout << " found search on for, simTime(): " << simTime() << endl;
//                cout << "messagesBuffer.size(): " << messagesBuffer.size() << endl;
//                cout << "ordemMessages.size(): " << ordemMessages.size() << endl;

                if (it->second.getGlobalMessageIdentificaton() != ordemMessages[0]) {
//                    cout << "it->second.getGlobalMessageIdentificaton() != ordemMessages[0], message to delete is not the last" << endl;
//                    cout << "it " << it->second.getGlobalMessageIdentificaton() << " od " << ordemMessages[0] << endl;

                    unordered_map<int, string>::iterator it2;
                    int now = countMessage;
                    for (it2 = ordemMessages.begin(); it2 != ordemMessages.end(); it2++) {
                    //for (int i = 1; i <= countMessage; i++) {
                        if (it2->second == messagesBuffer[ordemMessages[1]].getGlobalMessageIdentificaton()){
//                            cout << "Encontrado " << it2->first << " " << it2->second << endl;
                            now = it2->first;
                            break;
                        }
                    }
                    for (; now < countMessage; now++) {
//                        cout << "moving messages " << now  << " recive " << (now+1) << endl;
//                        cout << ordemMessages[now] << " " << ordemMessages[now+1] << endl;
//                        cout << "messagesBuffer[ordemMessages[now]] "<< " " << messagesBuffer[ordemMessages[now]] << endl;
                        messagesBuffer[ordemMessages[now]] = messagesBuffer[ordemMessages[now+1]];
                        ordemMessages[now] = ordemMessages[now+1];
//                        cout << "moved" << endl;
//                        printMessagesBuffer();
//                        printOrdemMessages();
//                        cout << ordemMessages[now] << " " << ordemMessages[now+1] << endl;
                    }
                }
//                cout << "countMessage: " << countMessage << endl;
//                cout << "messagesBuffer[ordemMessages[countMessage]].getGlobalMessageIdentificaton()  " << messagesBuffer[ordemMessages[countMessage]].getGlobalMessageIdentificaton() << endl;
//                cout << "ordemMessages[ordemMessages] " << ordemMessages[countMessage] << endl;
                messagesBuffer.erase(ordemMessages[countMessage]);
                ordemMessages.erase(countMessage);
                t = false;
//                cout << "messagesBuffer.size(): " << messagesBuffer.size() << endl;
//                cout << "ordemMessages.size(): " << ordemMessages.size() << endl;
                countMessage--;
//                cout << "countMessage: " << countMessage << endl;
//                printMessagesBuffer();
//                cout << "end" << endl;
                break;
            }
        }
    }
}

int vehDist::getCategory(){
    // ver como definir a categoria
    if (traci->getId() < 5) {
        return 1;
    }
    else if (traci->getId() > 5) {
        return 2;
    }
    else if (traci->getId() < 10) {
        return 3;
    }
    else {
        return 4;
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
         cout << "Distance to the target is less" << endl;
         return true;
     }else if(distanceNow == distanceBefore){
         cout << "Distance not change" << endl;
     }else{
         cout << "Distance to the target is more" << endl;
     }
     return false;
}

//###################################################  OK Function ####################################################

// Save vehicle start position
void vehDist::saveVehStartPosition(){
    int repeatNumber = par("repeatNumber");
    if (source.compare("car[0]") == 0) {
        if (repeatNumber == 0) {
            myfile.open ("results/vehicle_position_initialize.txt");
        } else {
            myfile.open ("results/vehicle_position_initialize.txt", std::ios_base::app);
        }
        myfile << "Start Position Vehicles, repeatNumber: " << repeatNumber << endl;
    } else {
        myfile.open ("results/vehicle_position_initialize.txt", std::ios_base::app);
    }
    myfile << findHost()->getFullName() << ": "<< traci->getCurrentPosition() << endl;
    myfile.close();
}


void vehDist::restartFilesResult(){
    int repeatNumber = par("repeatNumber");
    if ((source.compare("car[0]") == 0) && (repeatNumber == 0)) {
        //Open a new file for the current simulation
        myfile.open ("results/VehMessages.txt");
        myfile.close();

        myfile.open ("results/VehBroadcastMessages.txt");
        myfile.close();

//      //Open a new file for the current simulation
//      myfile.open ("results/LocalMessageBuffer_veh.txt");
//      myfile.close();
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
    //cout << "sendData: " << findHost()->getFullName() << sendData << endl;
    if (sendData){
        sendDataEvt = new cMessage("data evt", SEND_DATA_EVT);
        if(strcmp(findHost()->getFullName(), "car[0]") == 0){
            cout << endl << findHost()->getFullName() << ": Generating the message" << endl;
            generateMessage();
        }
        //simulate asynchronous channel access
        double offSet = (dblrand())/10;
        //cout << "SendData: " << findHost()->getFullName() << " offset: " << offSet << endl;
        //cout << findHost()->getFullName() << " at simtime "<< simTime() << "schedule created sendData" << endl;
        scheduleAt((simTime() + par("dataInterval").doubleValue() + offSet), sendDataEvt);
    }
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
    wsm->setSerial(serial);

    wsm->setTimestamp(simTime());
    wsm->setSenderAddressString(findHost()->getFullName());

    //beacon don't need
    //wsm->setRecipientAddressString("BROADCAST");
    //wsm->setSource(source);
    //wsm->setTarget(target);

    wsm->setSenderPos(curPosition);
    wsm->setRoadId(traci->getRoadId().c_str());
    wsm->setSenderSpeed(traci->getSpeed());
    wsm->setVehicleId(traci->getId());

    wsm->setCategory(getCategory());
    wsm->setSenderPosBack(vehPositionBack);

    // heading 1 to 4 or 1 to 8
    wsm->setHeading(getHeading4());
    //wsm->setHeading(getHeading8());

    if (name == "beacon") {
        DBG << "Creating Beacon with Priority " << priority << " at Applayer at " << wsm->getTimestamp() << std::endl;
    } else if (name == "data") {
        DBG << "Creating Data with Priority " << priority << " at Applayer at " << wsm->getTimestamp() << std::endl;
    }
    return wsm;
}


WaveShortMessage* vehDist::updateMessageWSM(WaveShortMessage* wsm, string rcvId) {
    wsm->setTimestamp(simTime());

    //wsm->setSenderAddress(MACToInteger(myMac));
    wsm->setSenderAddressString(findHost()->getFullName());

    wsm->setRecipientAddressString(rcvId.c_str());
    wsm->setSenderPos(curPosition);
    wsm->setRoadId(traci->getRoadId().c_str());
    wsm->setSenderSpeed(traci->getSpeed());
    wsm->setVehicleId(traci->getId());
    wsm->setSenderPosBack(vehPositionBack);
    wsm->setHopCount(wsm->getHopCount()-1);

    DBG << "Creating Data with Priority " << wsm->getPriority() << " at Applayer at " << wsm->getTimestamp() << std::endl;
    return wsm;
}


//Generate a target in order to send a message
void vehDist::generateTarget(){
    //Set the target node to whom my message has to be delivered
    target = "rsu[0]";
}


WaveShortMessage* vehDist::generateMessage(){
    WaveShortMessage* wsm = new WaveShortMessage("data");

    wsm->setName("data");

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
    wsm->setTimestamp(simTime());

    //wsm->setSenderAddress(MACToInteger(myMac));
    wsm->setSenderAddressString(findHost()->getFullName());

    // define before send
    //wsm->setRecipientAddress(BROADCAST);
    wsm->setRecipientAddressString("initial");

    wsm->setSource(source.c_str());
    wsm->setTarget(target.c_str());
    wsm->setSenderPos(curPosition);
    wsm->setSerial(2);

    string data = "WSMData generated by ";
    data += findHost()->getFullName();
    wsm->setWsmData(data.c_str());
    wsm->setGlobalMessageIdentificaton((to_string(vehDist::messageId) + to_string(MACToInteger(myMac))).c_str());
    vehDist::messageId++;

    wsm->setHopCount((hopCount - 1));
    wsm->setMessageTimestampGenerate(simTime());
    wsm->setTargetPos(Coord(par("target_x"), par("target_y"), 3));

    // Adding the message on the buffer
    messagesBuffer.insert(make_pair(wsm->getGlobalMessageIdentificaton(),*wsm));
    cout<<endl<<"Message generated by " << findHost()->getFullName() << " at simTime " << simTime() << endl;

    countMessage++;
    ordemMessages.insert(make_pair(countMessage, wsm->getGlobalMessageIdentificaton()));
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
            sendWSM(prepareBeaconWSM("beacon", beaconLengthBits, type_CCH, beaconPriority, 0, -1));
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
        cout << "messagesBuffer from " << findHost()->getFullName() << " is empty now " << endl;
    } else {
        cout << "Printing the messagesBuffer from " << findHost()->getFullName() << "(" << MACToInteger(myMac) <<"):" << endl;
        unordered_map<string, WaveShortMessage>::iterator it;
        for (it = messagesBuffer.begin(); it != messagesBuffer.end(); it++) {
            cout << " Id: " << it->second.getGlobalMessageIdentificaton()<< endl;
            cout << " Message Content: " << it->second.getWsmData() << endl;
            cout << " Source: " << it->second.getSource() << endl;
            cout << " Target: " << it->second.getTarget() << endl;
            cout << " Timestamp: " << it->second.getTimestamp() << endl;
            cout << " MessageTimestampGenerate: " << it->second.getMessageTimestampGenerate() << endl;
            cout << " HopCount: " << it->second.getHopCount() << endl << endl;
        }
    }
}


void vehDist::printOrdemMessages() {
    if (ordemMessages.empty()) {
        cout << "ordemMessages from " << findHost()->getFullName() << " is empty now " << endl;
    } else {
        cout << "Printing the ordemMmessages from " << findHost()->getFullName() << "(" << MACToInteger(myMac) <<"):" << endl;
        unordered_map<int, string>::iterator it;
        for (it = ordemMessages.begin(); it != ordemMessages.end(); it++) {
            cout << " it: " << it->first <<" valor: " << it->second << endl;
        }
    }
}


void vehDist::printOrdemBeacons() {
    if (ordemBeacons.empty()) {
        cout << "ordemordemBeacons from " << findHost()->getFullName() << " is empty now " << endl;
    } else {
        cout << "Printing the ordemBeacons from " << findHost()->getFullName() << "(" << MACToInteger(myMac) <<"):" << endl;
        unordered_map<int, string>::iterator it;
        for (it = ordemBeacons.begin(); it != ordemBeacons.end(); it++) {
            cout << " it: " << it->first <<" valor: " << it->second << endl;
        }
    }
}


void vehDist::printBeaconNeighbors() {
    cout << " in printBeaconNeighbors" << endl;
    if (beaconNeighbors.empty()) {
        cout << "beaconNeighbors from " << findHost()->getFullName() << " is empty now " << endl;
    } else {
        cout << "Printing the beaconNeighbors from " << findHost()->getFullName() << "(" << MACToInteger(myMac) <<"):" << endl;
        unordered_map<string, WaveShortMessage>::iterator it;
        for (it = beaconNeighbors.begin(); it != beaconNeighbors.end(); it++) {
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
     angle >= 247.5  && angle < 292.5                              return 7; // S => 270º
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


void vehDist::recordOnFileMessages(WaveShortMessage* wsm){
        //Open file for just apeend
        myfile.open ("results/VehMessages.txt", std::ios_base::app);

        fieldsToSave(wsm);
        myfile.close();
}


void vehDist::recordOnFileMessagesBroadcast(WaveShortMessage* wsm){
        //Open file for just apeend
        myfile.open ("results/VehBroadcastMessages.txt", std::ios_base::app);

        fieldsToSave(wsm);
        myfile.close();
}


void vehDist::fieldsToSave(WaveShortMessage* wsm){
    //Send "strings" to be saved on the file
    myfile << "Data from " << wsm->getSenderAddressString() << " at " << simTime();
    myfile << " to " << wsm->getRecipientAddressString() << endl;
    myfile << "wsm->getName(): " << wsm->getName() << endl;
    myfile << "wsm->getBitLength(): " << wsm->getBitLength() << endl;
    myfile << "wsm->getChannelNumber(): " << wsm->getChannelNumber() << endl;
    myfile << "wsm->getPsid(): " << wsm->getPsid() << endl;
    myfile << "wsm->getPriority(): " << wsm->getPriority() << endl;
    myfile << "wsm->getWsmVersion(): " << wsm->getWsmVersion() << endl;
    myfile << "wsm->getTimestamp(): " << wsm->getTimestamp() << endl;
    myfile << "wsm->getMessageTimestampGenerate(): " << wsm->getMessageTimestampGenerate() << endl;
    myfile << "wsm->getHeading(): " << wsm->getHeading() << endl;
    myfile << "wsm->getSenderAddressString(): " << wsm->getSenderAddressString() << endl;
    myfile << "wsm->getRecipientAddressString(): " << wsm->getRecipientAddressString() << endl;
    myfile << "wsm->getSource(): " << wsm->getSource() << endl;
    myfile << "wsm->getTarget(): " << wsm->getTarget() << endl;
    myfile << "findHost()->getFullName(): " << findHost()->getFullName() << endl;
    myfile << "wsm->getSenderPos(): " << wsm->getSenderPos() << endl;
    myfile << "wsm->getSerial(): " << wsm->getSerial() << endl;
    myfile << "wsm->getSummaryVector(): " << wsm->getSummaryVector() << endl;
    myfile << "wsm->getRequestMessages(): " << wsm->getRequestMessages() << endl;
    myfile << "wsm->getWsmData(): " << wsm->getWsmData() << endl;
    myfile << "Time to generate and recived: " << (wsm->getTimestamp() - wsm->getMessageTimestampGenerate()) << endl;
    myfile << endl << endl;
}


//########################################################  Default Function #############################################

void vehDist::sendWSM(WaveShortMessage* wsm) {
    if (isParking && !sendWhileParking)
        return;
    sendDelayedDown(wsm,individualOffset);
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
            findHost()->getDisplayString().updateWith("r=16,red");
            if (!sentMessage)
                sendMessage(traci->getRoadId());
        }
    }
    else {
        lastDroveAt = simTime();
    }
}
