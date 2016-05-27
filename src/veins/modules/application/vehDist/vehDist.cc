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

Define_Module(vehDist);

void vehDist::initialize(int stage) {
    BaseWaveApplLayer::initialize_default_veins_TraCI(stage);
    if (stage == 0) { // traci - mobility, traci->getComand - traci, new traciVehice
        mobility = TraCIMobilityAccess().get(getParentModule());
        traci = mobility->getCommandInterface();
        traciVehicle = mobility->getVehicleCommandInterface();

        vehInitializeVariables();
    }
}

void vehDist::vehInitializeVariables() {
    generalInitializeVariables_executionByExpNumber();

    vehCategory = traciVehicle->getTypeId();
    msgBufferMaxUse = 0;

    if (myId == 0) { // Vehicle must be the first to generate messages, so your offset is 0;
        vehDist::countMesssageDrop = vehDist::countMsgPacketSend = vehDist::msgBufferUseGeneral = vehDist::countVehicleAll = 0;
        vehDist::msgDroppedbyTTL = vehDist::msgDroppedbyHop = vehDist::msgDroppedbyBuffer = 0;
        vehDist::countMeetPshortestT = vehDist::countTwoCategoryN = vehDist::countMeetN = 0;
        vehDist::beaconMessageId = 1;

        vehDist::ttlBeaconStatus = par("ttlBeaconStatus");
        vehDist::beaconMessageBufferSize = par("beaconMessageBufferSize");
        vehDist::beaconStatusBufferSize = par("beaconStatusBufferSize");
        vehDist::timeToUpdatePosition = par("vehTimeUpdatePosition");
        vehDist::timeLimitGenerateBeaconMessage = par("timeLimitGenerateBeaconMessage");

        //initialize random seed (Seed the RNG) # Inside of IF because must be executed one time (the seed is "static")
        mt_veh.seed(repeatNumber); // Instead another value, for make the experiment more reproducible, so seed = reapeatNumber
        srand(repeatNumber + 1); // repeatNumber + 1, because srand(0) == srand(1)

//        mt_veh.seed(1);
//        srand(1);

        cout << endl << "Experiment: " << expNumber << " repeatNumber: " << repeatNumber << endl;
        cout << "ttlBeaconMessage: " << ttlBeaconMessage  << endl;
        cout << "countGenerateBeaconMessage: " << countGenerateBeaconMessage << endl;
        cout << "timeLimitGenerateMessage: " << vehDist::timeLimitGenerateBeaconMessage << endl << endl;
    }

    vehOffSet = double(myId)/1000; // Simulate asynchronous channel access. Values between 0.001 ... 0.010
    vehDist::numVehicles.push_back(source);
    WaveShortMessage w;
    w.setTimestamp(simTime());
    vehDist::vehScenario.insert(make_pair(source, w));

    vehDist::countVehicleAll++;

    restartFilesResult(); // Start the file for save results
    vehCreateUpdateRateTimeToSendEvent(); // Create Evt to update the rateTimeToSend

    vehGenerateBeaconMessageBegin(); // Create Evt to generate messages

//    generateBeaconMessage();

    vehCreateEventTrySendBeaconMessage(); // Create one Evt to try send messages in buffer
    cout << endl << source << " entered in the scenario at " << simTime() << " whit OffSet: " << vehOffSet << endl;
}

void vehDist::onBeaconStatus(WaveShortMessage* wsm) {
    unordered_map <string, WaveShortMessage>::iterator itStatusNeighbors = beaconStatusNeighbors.find(wsm->getSource());
    if (itStatusNeighbors != beaconStatusNeighbors.end()) { // Update the beaconStatus
        itStatusNeighbors->second = *wsm;
    } else {
        if (beaconStatusNeighbors.size() >= vehDist::beaconStatusBufferSize) {
            removeOldestInputBeaconStatus();
        }
        beaconStatusNeighbors.insert(make_pair(wsm->getSource(), *wsm));
        sendMessageToOneNeighborTarget(wsm->getSource()); // Look in buffer it has messages for this new vehNeighbor
        sendMessageToOneNeighborTargetOnlyDelivery(wsm->getSource()); // Look in buffer of messageOnlyDelivery for this new vehNeighbor
    }
    //printBeaconNeighbors();
}

void vehDist::onBeaconMessage(WaveShortMessage* wsm) {
    if (source.compare(wsm->getRecipientAddressTemporary()) == 0) { //verify if this is the recipient of the message

        // Real scenario
        //if (source.compare(wsm->getTarget()) == 0) { // Message to this vehicle
            //
        //} else {

        // test if message has been delivered to the target before.
        // TODO Alguma diferença no resultado final de mensagens recebidas?
        if (messagesDelivered.empty() || (find(messagesDelivered.begin(), messagesDelivered.end(), wsm->getGlobalMessageIdentificaton()) == messagesDelivered.end())) {
            saveMessagesOnFile(wsm, fileMessagesUnicast);

            // Look in neigborStatus buffer if has the target of this message
            bool insert = sendOneNewMessageToOneNeighborTarget(*wsm);
            if (insert) {
                if (wsm->getHopCount() > 1) {
                    if (messagesBuffer.empty() || messagesBuffer.find(wsm->getGlobalMessageIdentificaton()) == messagesBuffer.end()) { //verify if the message isn't in the buffer
                        cout << source << " saving message from " << wsm->getSenderAddressTemporary() << " with Id: " << wsm->getGlobalMessageIdentificaton() << " at: " << simTime() << endl;
                        string wsmDataTmp = wsm->getWsmData();
                        if (wsmDataTmp.size() < 42) { // WSMData generated by car[x] and carry by [ T, P
                            wsmDataTmp += " and carry by (";
                        } else {
                            wsmDataTmp += ", ";
                        }
                        wsmDataTmp += vehCategory;
                        wsm->setWsmData(wsmDataTmp.c_str());

                        if (messagesBuffer.size() >= vehDist::beaconMessageBufferSize) {
                            removeOldestInputBeaconMessage();
                        }

                        messagesBuffer.insert(make_pair(wsm->getGlobalMessageIdentificaton(), *wsm)); // Add the message in the vehicle buffer
                        if (messagesBuffer.size() > msgBufferMaxUse) {
                            msgBufferMaxUse = messagesBuffer.size();
                        }
                        messagesOrderReceived.push_back(wsm->getGlobalMessageIdentificaton());

                        colorCarryMessage();
                        printMessagesBuffer();
                    } else {
                        cout << source << " message is on the buffer at: " << simTime() << endl;
                    }
                } else if (wsm->getHopCount() == 1) { // wsm->getHopCount() == 0 // Verificar RFC
                    messagesOnlyDelivery.insert(make_pair(wsm->getGlobalMessageIdentificaton(), *wsm)); // Add the message to only delivery
                    //insertMessageDrop(wsm->getGlobalMessageIdentificaton(), 2); // by ttl (1 buffer, 2 hop, 3 time)
                }
            }
        } else {
            cout << source << " message with Id: " << wsm->getGlobalMessageIdentificaton() << " from " << wsm->getSenderAddressTemporary() << " has been delivered to the target before" << endl;
        }
    }/*else { // To another vehicle
        cout << "Saving broadcast message from: " << wsm->getSenderAddressTemporary() << " to " << source << endl;
        saveMessagesOnFile(wsm, fileMessagesNameBroadcast);
    }*/
    //printMessagesBuffer();
}

void vehDist::colorCarryMessage() {
    if (!messagesBuffer.empty()) {
        unordered_map<string, WaveShortMessage>::iterator itMessage = messagesBuffer.begin();
        for (unsigned int i = 0; i < messagesBuffer.size(); i++) {
            if (source.compare(itMessage->second.getSource()) == 0) {
                findHost()->getDisplayString().updateWith("r=12,green"); // Has message(s) with himself was generated
                i = messagesBuffer.size();
            } else {
                findHost()->getDisplayString().updateWith("r=12,blue"); // Has (only) message(s) with another was generated
            }
            itMessage++;
        }
    } else {
        findHost()->getDisplayString().updateWith("r=0"); // Remove the range color
    }
}

void vehDist::removeOldestInputBeaconMessage() {
    //printMessagesBuffer();

    if (!messagesBuffer.empty()) {
        unsigned short int typeRemoved = 0;
        unordered_map <string, WaveShortMessage>::iterator itMessage = messagesBuffer.begin();
        simtime_t minTime = itMessage->second.getTimestamp();
        string idMessage = itMessage->first;
        itMessage++;

        for (; itMessage != messagesBuffer.end(); itMessage++) {
            if (minTime > itMessage->second.getTimestamp()) {
                minTime = itMessage->second.getTimestamp();
                idMessage = itMessage->first;
            }
        }

        if (simTime() > (minTime + ttlBeaconMessage)) {
            //cout << source << " remove one message (" << idMessage << ") by time, minTime: " << minTime << " at: " << simTime() << " ttlBeaconMessage: " << ttlBeaconMessage << endl;
            typeRemoved = 3; // by ttl (1 buffer, 2 hop, 3 time)
        } else if (messagesBuffer.size() >= vehDist::beaconMessageBufferSize) {
            //cout << source << " remove one message (" << idMessage << ") by space, MessageBuffer.size(): " << messagesBuffer.size() << " at: " << simTime() << " vehDist::beaconMessageBufferSize: " << vehDist::beaconMessageBufferSize << endl;
            typeRemoved = 1; // by buffer (1 buffer, 2 hop, 3 time)
        }

        if (typeRemoved != 0) {
            insertMessageDrop(idMessage, typeRemoved); // Removed by the value of tyRemoved (1 buffer, 2 hop, 3 time)
            messagesBuffer.erase(idMessage);
            auto itMsg = find(messagesOrderReceived.begin(), messagesOrderReceived.end(), idMessage);
            messagesOrderReceived.erase(itMsg);

            colorCarryMessage();
            removeOldestInputBeaconMessage();
        }
    } /*else {
        cout << "messagesBuffer from " << source << " is empty now" << endl;
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

        if (simTime() > (minTime + vehDist::ttlBeaconStatus)) {
            //cout << source << " remove one beaconStatus (" << idBeacon << ") by time, minTime: " << minTime << " at: " << simTime() << " vehDist::ttlBeaconStatus: " << vehDist::ttlBeaconStatus << endl;
            beaconStatusNeighbors.erase(idBeacon);
            removeOldestInputBeaconStatus();
        } else if (beaconStatusNeighbors.size() >= vehDist::beaconStatusBufferSize) {
            //cout << source << " remove one beaconStatus (" << idBeacon << ") by space, beaconStatusNeighbors.size(): " << beaconStatusNeighbors.size() << " at: " << simTime() << " vehDist::beaconMessageBufferSize: " << vehDist::beaconMessageBufferSize << endl;
            beaconStatusNeighbors.erase(idBeacon);
            removeOldestInputBeaconStatus();
        }
    } /*else {
        cout << "beaconStatusNeighbors from " << source << " is empty now" << endl;
    }*/
}

void vehDist::insertMessageDrop(string messageId, unsigned short int type) {
    if (messagesDrop.empty() || (messagesDrop.find(messageId) == messagesDrop.end())) {
        struct messagesDropStruct mD_tmp;
        mD_tmp.byBuffer = mD_tmp.byHop = mD_tmp.byTime = 0;

        if (type == 1) { // by buffer limit
            mD_tmp.byBuffer = 1;
        } else if (type == 2) { // by hop limit
            mD_tmp.byHop = 1;
        } else { // type == 3 // by TTL/time limit
            mD_tmp.byTime = 1;
        }

        messagesDrop.insert(make_pair(messageId, mD_tmp));
    } else {
        if (type == 1) { // Increment the number of byBuffer (limit)
            messagesDrop[messageId].byBuffer++;
        } else if (type == 2) { // Increment the number of byHop (limit)
            messagesDrop[messageId].byHop++;
        } else { // type == 3 // Increment the number of byTime (limit)
            messagesDrop[messageId].byTime++;
        }
    }
}

void vehDist::sendBeaconMessage() {
    cout << source << " try to send message at: " << simTime() << endl;
//############################################################# run many times
    removeOldestInputBeaconStatus();
    removeOldestInputBeaconMessage();

//    unordered_map<string, WaveShortMessage>::iterator itbeaconStatus;
//    for (itbeaconStatus = beaconStatusNeighbors.begin(); itbeaconStatus != beaconStatusNeighbors.end(); itbeaconStatus++){
//        sendMessageNeighborsTarget(itbeaconStatus->second.getSource()); // Look in buffer it has messages for this new vehicle neighbor
//    }
//############################################################# run many times

    if (messageToSend >= messagesOrderReceived.size() || messagesOrderReceived.empty()) {
        messageToSend = 0;
    }

    //printMessagesBuffer();
    trySendBeaconMessage();
    //printMessagesBuffer();
    messageToSend++; // Move to next message

    if (messageToSend >= messagesOrderReceived.size() || messagesOrderReceived.empty()) {
        if (simTime() > timeToFinishLastStartSend) {
            cout << source << " 1_schedule at: " << simTime() << " to: " << (simTime() + double(rateTimeToSend)/1000);
            scheduleAt((simTime() + double(rateTimeToSend)/1000), sendBeaconMessageEvt);
        } else {
            scheduleAt(timeToFinishLastStartSend, sendBeaconMessageEvt);
            cout << source << " 2_schedule at: " << simTime() << " to: " << timeToFinishLastStartSend;
        }

        timeToFinishLastStartSend += double(rateTimeToSendLimitTime)/1000;
    } else {
        scheduleAt((simTime() + double(rateTimeToSend)/1000), sendBeaconMessageEvt);
        cout << source << " 3_schedule at: " << simTime() << " to: " << (simTime() + double(rateTimeToSend)/1000) << " rateTimeToSend: " << rateTimeToSend;
    }

    cout << " timeToFinishLastStartSend: " << timeToFinishLastStartSend << endl;
    cout << "                               " << source << " expSendbyDSCR: " << expSendbyDSCR << " at: " << simTime() << endl << endl;
}

void vehDist::trySendBeaconMessage() {
    if (!messagesBuffer.empty()) {
        cout << source << " messagesBuffer with "<< messagesBuffer.size() << " message(s) at " << simTime() << endl;
        // Test with hopCount 0, send only

        if (!beaconStatusNeighbors.empty()) {
            //printBeaconStatusNeighbors();

            // Test to send RSU
            // Assumimos o mesmo Range (rsu e veh)

            string idMessage = messagesOrderReceived[messageToSend];
            string rcvId = neighborWithShortestDistanceToTarge(idMessage);

            string catVeh;
            if (source.compare(rcvId) != 0) {
                unordered_map <string, WaveShortMessage>::iterator itBeaconN;
                itBeaconN = beaconStatusNeighbors.find(rcvId);
                catVeh = itBeaconN->second.getCategory();

                cout << source << " chose the vehicle " << rcvId << " to be a next hop to the " << idMessage << " message" << endl;
                cout << "    " << source << " send message to " << rcvId << " with category " << catVeh << " at "<< simTime() << endl;
                cout << "    MessageToSend: " << messageToSend << endl;
                cout << "    MessageID: " << idMessage << endl;
                cout << "    Source: " << messagesBuffer[idMessage].getSource() << endl;
                cout << "    Message content: " << messagesBuffer[idMessage].getWsmData() << endl;
                cout << "    Target: " << messagesBuffer[idMessage].getTarget() << endl;
                cout << "    Timestamp: " << messagesBuffer[idMessage].getTimestamp() << endl;
                cout << "    HopCount: " << messagesBuffer[idMessage].getHopCount() << endl << endl;

                sendWSM(updateBeaconMessageWSM(messagesBuffer[idMessage].dup(), rcvId));
                vehDist::countMsgPacketSend++;

                if (messagesSendLog.empty()) {
                    messagesSendLog.insert(make_pair(idMessage, rcvId));
                } else {
                    messagesSendLog[idMessage] += ", " + rcvId;
                }

            } else {
                cout << source << " not send any message" << endl << endl;
            }
        } else {
            cout << source << " beaconNeighbors on sendDataMessage is empty now" << endl << endl;
        }
    } /*else {
        cout << source << " messagesBuffer is empty at " << simTime() << endl;
    }*/
}

string vehDist::choseCategory_RandomNumber1to100(unsigned short int percentP, string vehIdP, string vehIdT) {
    unsigned short int valRand = rand() % 100 + 1;
    if (valRand <= percentP) {
        return vehIdP;
    } else { // valRand > percentP
        return vehIdT;
    }
}

string vehDist::neighborWithShortestDistanceToTarge(string idMessage) {
    double neighborDistanceBefore, neighborDistanceNow;
    unordered_map<string, shortestDistance> vehShortestDistanceToTarget;
    unordered_map<string, shortestDistance>::iterator itShortestDistance;
    unordered_map<string, WaveShortMessage>::iterator itBeaconNeighbors;
    string category, vehId;;
    shortestDistance sD;

    unsigned short int percentP = par("porcentP"); // from ini file, 20 meaning 20%
    vehId = source;

    unsigned short int t, p;
    p = t = 0;

    for (itBeaconNeighbors = beaconStatusNeighbors.begin(); itBeaconNeighbors != beaconStatusNeighbors.end(); itBeaconNeighbors++) {
        if (!messagesSendLog.empty() && (messagesSendLog[idMessage].find(itBeaconNeighbors->first) != std::string::npos)){
            cout << source << " has been send a message " << idMessage << " to " << itBeaconNeighbors->first << endl;
        } else {

            neighborDistanceBefore = traci->getDistance(itBeaconNeighbors->second.getSenderPosPrevious(), messagesBuffer[idMessage].getTargetPos(), false);
            neighborDistanceNow = traci->getDistance(itBeaconNeighbors->second.getSenderPos(), messagesBuffer[idMessage].getTargetPos(), false);

            if (neighborDistanceBefore > neighborDistanceNow) { // Test if is closing to target
                sD.categoryVeh = itBeaconNeighbors->second.getCategory();
                sD.distanceToTarget = neighborDistanceNow;
                sD.senderPos = Coord(itBeaconNeighbors->second.getSenderPos());
                sD.speedVeh = itBeaconNeighbors->second.getSenderSpeed();
                sD.rateTimeToSendVeh = itBeaconNeighbors->second.getRateTimeToSend();

                if (sD.categoryVeh.compare("T") == 0) {
                    sD.distanceToTargetCategory = sD.distanceToTarget + sD.distanceToTarget * 0.1;
                    t = 1;
                } else if (sD.categoryVeh.compare("P") == 0) {
                    sD.distanceToTargetCategory = sD.distanceToTarget + sD.distanceToTarget * 0.3;
                    p =1;
                }

                sD.decisionValueDistanceSpeed = sD.distanceToTarget - (sD.speedVeh);
                sD.decisionValueDistanceRateTimeToSend = sD.distanceToTarget + (double(sD.rateTimeToSendVeh)/100);
                sD.decisionValueDistanceSpeedRateTimeToSend = sD.distanceToTarget - (sD.speedVeh) + (double(sD.rateTimeToSendVeh)/100);

                // Distance = [0 - 125] - 720 m // vehicle Speed = 0 - (15/25)84 m/s // rateTimeToSend = 100 to 5000 ms
                // DecisonValueDS = distance - speed
                // DecisonValueDSCR = distance - speed + rateTimeToSend/100 (0.1 * 10)

                vehShortestDistanceToTarget.insert(make_pair(itBeaconNeighbors->first, sD));
            } /*else {
                cout << itBeaconNeighbors->first << " going to another direction" << endl;
            }*/
        }
    }

    if (!vehShortestDistanceToTarget.empty()) {
        if ((p == 1) && (t == 1)) {
            vehDist::countTwoCategoryN++;
            cout << endl << "Meet of vehicles of two categories" << endl;
        } else if ((p == 1) || (t == 1)) {
            vehDist::countMeetN++;
        }

        cout << endl << "Source vehicle informations" << endl;
        cout << "    Id: " << source << endl;
        cout << "    Message Id: " << messagesBuffer[idMessage].getGlobalMessageIdentificaton() << endl;
        cout << "    Category: " << vehCategory << endl;
        cout << "    Distance: " << traci->getDistance(curPosition, messagesBuffer[idMessage].getTargetPos(), false) << endl;
        cout << "    Speed: " << mobility->getSpeed() << endl;
        cout << "    Position: " << mobility->getCurrentPosition() << endl;
        cout << "    RateTimeToSend: " << rateTimeToSend << endl;

        cout << endl << endl << "Printing vehShortestDistanceToTarget to " << source << " at " << simTime() << endl << endl;
        for (itShortestDistance = vehShortestDistanceToTarget.begin(); itShortestDistance != vehShortestDistanceToTarget.end(); itShortestDistance++) {
            cout << "    Id: " << itShortestDistance->first << endl;
            cout << "    Position: " << itShortestDistance->second.senderPos << endl;
            cout << "    Category: " << itShortestDistance->second.categoryVeh << endl;
            cout << "    Distance: " << itShortestDistance->second.distanceToTarget << endl;
            cout << "    DistanceCategory: " << itShortestDistance->second.distanceToTargetCategory << endl;
            cout << "    Speed: " << itShortestDistance->second.speedVeh << endl;
            cout << "    RateTimeToSend: " << itShortestDistance->second.rateTimeToSendVeh << endl;
            cout << "    DecisionValueDistanceSpeed: " << itShortestDistance->second.decisionValueDistanceSpeed << endl;
            cout << "    DecisionValueDistanceSpeedRateTimeToSend: " << itShortestDistance->second.decisionValueDistanceSpeedRateTimeToSend << endl << endl;
        }
    } else {
        cout << endl << "vehShortestDistanceToTarget to " << source << " at " << simTime() << " vehShortestDistanceToTarget is empty." << endl << endl;
    }

    if (!vehShortestDistanceToTarget.empty()) { // If don't any veh going to target
        switch (expSendbyDSCR) {
        case 1:
            vehId = chosenByDistance(vehShortestDistanceToTarget);
            break;
        case 12:
            vehId = chosenByDistance_Speed(vehShortestDistanceToTarget);
            break;
        case 13:
            vehId = chosenByDistance_Category(vehShortestDistanceToTarget, percentP);
            break;
        case 14:
            vehId = chosenByDistance_RateTimeToSend(vehShortestDistanceToTarget);
            break;
        case 123:
            vehId = chosenByDistance_Speed_Category(vehShortestDistanceToTarget, percentP);
            break;
////######################################################################################################
//        case 124:
//            vehId = chosenByDistance_Speed_RateTimeToSend(vehShortestDistanceToTarget, percentP);
//            break;
//        case 134:
//            vehId = chosenByDistance_Category_RateTimeToSend(vehShortestDistanceToTarget, percentP);
//            break;
////######################################################################################################
        case 1234:
            vehId = chosenByDistance_Speed_Category_RateTimeToSend(vehShortestDistanceToTarget, percentP);
            break;
        default:
            cout << "Error! expSendbyDSCR: " << expSendbyDSCR << "not defined, class in vehDist.cc";
            DBG << "Error! expSendbyDSCR: " << expSendbyDSCR << "not defined, class in vehDist.cc";
            exit(1);
        }
    }

    return vehId;
}

string vehDist::chosenByDistance(unordered_map<string, shortestDistance> vehShortestDistanceToTarget) {
    unordered_map<string, shortestDistance>::iterator itShortestDistance;
    double distanceToTarget, shortestDistanceToTarget;

    string vehId = source;
    shortestDistanceToTarget = DBL_MAX;
    for (itShortestDistance = vehShortestDistanceToTarget.begin(); itShortestDistance != vehShortestDistanceToTarget.end(); itShortestDistance++) {
        distanceToTarget = itShortestDistance->second.distanceToTarget;
        if (shortestDistanceToTarget > distanceToTarget) {
            shortestDistanceToTarget = distanceToTarget;
            vehId = itShortestDistance->first;
        }
    }
    return vehId;
}

string vehDist::chosenByDistance_Speed(unordered_map<string, shortestDistance> vehShortestDistanceToTarget) {
    unordered_map<string, shortestDistance>::iterator itShortestDistance;
    double distanceSpeedValue, shortestDistanceSpeedValue;

    string vehId = source;
    shortestDistanceSpeedValue = DBL_MAX;
    for (itShortestDistance = vehShortestDistanceToTarget.begin(); itShortestDistance != vehShortestDistanceToTarget.end(); itShortestDistance++) {
        distanceSpeedValue = itShortestDistance->second.decisionValueDistanceSpeed;
        if (shortestDistanceSpeedValue > distanceSpeedValue) {
            shortestDistanceSpeedValue = distanceSpeedValue;
            vehId = itShortestDistance->first;
        }
    }
    return vehId;
}

string vehDist::chosenByDistance_Category(unordered_map<string, shortestDistance> vehShortestDistanceToTarget, int percentP) {
    unordered_map<string, shortestDistance>::iterator itShortestDistance;
//
//    bool go1 = true;
//    if (go1) {
        double distanceToTarget, shortestDistanceT, shortestDistanceP;
        string category, vehIdP, vehIdT;

        vehIdP = vehIdT = source;
        shortestDistanceP = shortestDistanceT = DBL_MAX;
        for(itShortestDistance = vehShortestDistanceToTarget.begin(); itShortestDistance != vehShortestDistanceToTarget.end(); itShortestDistance++) {
            category = itShortestDistance->second.categoryVeh;
            distanceToTarget = itShortestDistance->second.distanceToTarget;
            if (category.compare("P") == 0) {
                if (shortestDistanceP > distanceToTarget) {
                    shortestDistanceP = distanceToTarget;
                    vehIdP = itShortestDistance->first;
                }
            } else if (category.compare("T") == 0) {
                if (shortestDistanceT > distanceToTarget) {
                    shortestDistanceT = distanceToTarget;
                    vehIdT = itShortestDistance->first;
                }
            }
        }

        if (shortestDistanceP == DBL_MAX) {
            return vehIdT;
        } else if (shortestDistanceT == DBL_MAX) {
            return vehIdP;
        }

        if (int(shortestDistanceT) > int(shortestDistanceP)) {
            vehDist::countMeetPshortestT++;
            return choseCategory_RandomNumber1to100(percentP, vehIdP, vehIdT);
        } else {
            return vehIdT;
        }

//    } else {
//
//        double shortestDistanceCategory = DBL_MAX;
//        double distanceToTargetCategory;
//        string vehId = source;
//        for(itShortestDistance = vehShortestDistanceToTarget.begin(); itShortestDistance != vehShortestDistanceToTarget.end(); itShortestDistance++) {
//            distanceToTargetCategory = itShortestDistance->second.distanceToTargetCategory;
//
//            if (shortestDistanceCategory > distanceToTargetCategory) {
//                shortestDistanceCategory = distanceToTargetCategory;
//                vehId = itShortestDistance->first;
//            }
//        }
//        return vehId;
//
//    }
}

string vehDist::chosenByDistance_RateTimeToSend(unordered_map<string, shortestDistance> vehShortestDistanceToTarget) {
    unordered_map<string, shortestDistance>::iterator itShortestDistance;
    double distanceRateTimeToSendValue, shortestDistanceRateTimeToSendValue;
    string vehId = source;

    shortestDistanceRateTimeToSendValue = DBL_MAX;
    for (itShortestDistance = vehShortestDistanceToTarget.begin(); itShortestDistance != vehShortestDistanceToTarget.end(); itShortestDistance++) {
        distanceRateTimeToSendValue = itShortestDistance->second.decisionValueDistanceRateTimeToSend;
        if (shortestDistanceRateTimeToSendValue > distanceRateTimeToSendValue) {
            shortestDistanceRateTimeToSendValue = distanceRateTimeToSendValue;
            vehId = itShortestDistance->first;
        }
    }
    return vehId;
}

string vehDist::chosenByDistance_Speed_Category(unordered_map<string, shortestDistance> vehShortestDistanceToTarget, int percentP) {
    unordered_map<string, shortestDistance>::iterator itShortestDistance;
    double distanceSpeedValue, shortestDistanceT, shortestDistanceP;
    string category, vehIdP, vehIdT;

    vehIdP = vehIdT = source;
    shortestDistanceP = shortestDistanceT = DBL_MAX;
    for(itShortestDistance = vehShortestDistanceToTarget.begin(); itShortestDistance != vehShortestDistanceToTarget.end(); itShortestDistance++) {
        category = itShortestDistance->second.categoryVeh;
        distanceSpeedValue = itShortestDistance->second.decisionValueDistanceSpeed;
        if (category.compare("P") == 0) {
            if (shortestDistanceP > distanceSpeedValue) {
                shortestDistanceP = distanceSpeedValue;
                vehIdP = itShortestDistance->first;
            }
        } else if (category.compare("T") == 0) {
            if (shortestDistanceT > distanceSpeedValue) {
                shortestDistanceT = distanceSpeedValue;
                vehIdT = itShortestDistance->first;
            }
        }
    }

    if (shortestDistanceP == DBL_MAX) {
        return vehIdT;
    } else if (shortestDistanceT == DBL_MAX) {
        return vehIdP;
    }

    if (int(shortestDistanceT) > int(shortestDistanceP)) {
        vehDist::countMeetPshortestT++;
        return choseCategory_RandomNumber1to100(percentP, vehIdP, vehIdT);
    } else {
        return vehIdT;
    }
}

string vehDist::chosenByDistance_Speed_Category_RateTimeToSend(unordered_map<string, shortestDistance> vehShortestDistanceToTarget, int percentP) {
    unordered_map<string, shortestDistance>::iterator itShortestDistance;
    double valueDSCR, shortestDistanceT, shortestDistanceP;
    string category, vehIdP, vehIdT;

    vehIdP = vehIdT = source;
    shortestDistanceP = shortestDistanceT = DBL_MAX;
    for(itShortestDistance = vehShortestDistanceToTarget.begin(); itShortestDistance != vehShortestDistanceToTarget.end(); itShortestDistance++) {
        category = itShortestDistance->second.categoryVeh;
        valueDSCR = itShortestDistance->second.decisionValueDistanceSpeedRateTimeToSend;
        if (category.compare("P") == 0) {
            if (shortestDistanceP > valueDSCR) {
                shortestDistanceP = valueDSCR;
                vehIdP = itShortestDistance->first;
            }
        } else if (category.compare("T") == 0) {
            if (shortestDistanceT > valueDSCR) {
                shortestDistanceT = valueDSCR;
                vehIdT = itShortestDistance->first;
            }
        }
    }

    if (shortestDistanceP == DBL_MAX) {
        return vehIdT;
    } else if (shortestDistanceT == DBL_MAX) {
        return vehIdP;
    }

    if (int(shortestDistanceT) > int(shortestDistanceP)) {
        vehDist::countMeetPshortestT++;
        return choseCategory_RandomNumber1to100(percentP, vehIdP, vehIdT);
    } else {
        return vehIdT;
    }
}

void vehDist:: finish() {
    printCountBeaconMessagesDrop();

    auto it = find(vehDist::numVehicles.begin(), vehDist::numVehicles.end(), source);
    if (it != vehDist::numVehicles.end()) {
        vehDist::numVehicles.erase(it);
        vehDist::vehScenario.erase(source);
    } else {
        cout << "Error in vehDist::numVehicles, need to have the same entries as the number of vehicles" << endl;
        exit (1);
    }
}

void vehDist::printCountBeaconMessagesDrop() {
    myfile.open (fileMessagesDrop, std::ios_base::app);

    if (!messagesDrop.empty()) {
        myfile << endl << "messagesDrop from " << source << endl;
        unsigned short int messageDropbyOneVeh = 0;
        map<string, struct messagesDropStruct>::iterator it;
        for (it = messagesDrop.begin(); it != messagesDrop.end(); it++) {
            myfile << "Message Id: " << it->first << endl;
            myfile << "By Buffer: " << it->second.byBuffer << endl;
            myfile << "By Hop: " << it->second.byHop << endl;
            myfile << "By Time: " << it->second.byTime << endl << endl;
            messageDropbyOneVeh += it->second.byBuffer + it->second.byHop + it->second.byTime;
            vehDist::msgDroppedbyTTL += it->second.byTime;
            vehDist::msgDroppedbyHop += it->second.byHop;
            vehDist::msgDroppedbyBuffer += it->second.byBuffer;
        }

        vehDist::countMesssageDrop += messageDropbyOneVeh;
        myfile << "### " << source << " dropped: " << messageDropbyOneVeh << endl;
        myfile << "### " << source << " use message buffer: " << msgBufferMaxUse << endl;
        vehDist::msgBufferUseGeneral += msgBufferMaxUse;
    } else {
        myfile << endl << "messagesDrop from " << source << " is empty now" << endl;
        myfile << "### " << source << " dropped: " << 0 << endl;
    }

    if (vehDist::numVehicles.size() == 1) {
        myfile << endl << "Exp: " << expNumber << " ### Final count messages drop: " << vehDist::countMesssageDrop << endl;
        myfile << "Exp: " << expNumber << " ### Final count message dropped by buffer: " << vehDist::msgDroppedbyBuffer << endl;
        myfile << "Exp: " << expNumber << " ### Final count message dropped by hop: " << vehDist::msgDroppedbyHop << endl;
        myfile << "Exp: " << expNumber << " ### Final count message dropped by ttl: " << vehDist::msgDroppedbyTTL << endl;
        myfile << "Exp: " << expNumber << " ### Final average buffer use: " << double(vehDist::msgBufferUseGeneral)/vehDist::countVehicleAll << endl;
        myfile << "Exp: " << expNumber << " ### Count of vehicle in the scenario: " << vehDist::countVehicleAll << endl;
        myfile << "Exp: " << expNumber << " ### Count meets: " << (vehDist::countTwoCategoryN + vehDist::countMeetN) << endl;
        myfile << "Exp: " << expNumber << " ### Count meet two category: " << vehDist::countTwoCategoryN << endl;
        myfile << "Exp: " << expNumber << " ### Count meet another: " << vehDist::countMeetN << endl;
        myfile << "Exp: " << expNumber << " ### Count meet with real difference (Only valid with Category test): " << vehDist::countMeetPshortestT << endl;
        myfile << "Exp: " << expNumber << " ### Final count packets messages send: " << vehDist::countMsgPacketSend << endl << endl;
    }
    myfile.close();
}

void vehDist::sendMessageToOneNeighborTarget(string beaconSource) {
    unsigned short int countMessage = messagesBuffer.size();
    unordered_map <string, WaveShortMessage>::iterator itMessage = messagesBuffer.begin();
    string idMessage;
    while(countMessage > 0) {
        if (beaconSource.compare(itMessage->second.getTarget()) == 0) {
            idMessage = itMessage->second.getGlobalMessageIdentificaton();
            cout << source <<" sending MessageToOneNeighborTarget: " << idMessage << " to: " << beaconSource << " and removing from messagesBuffer" << endl;
            sendWSM(updateBeaconMessageWSM(itMessage->second.dup(), beaconSource));
            vehDist::countMsgPacketSend++;
            messagesDelivered.push_back(idMessage);

            if (countMessage == 1) {
                countMessage = 0;
            } else {
                countMessage--;
                itMessage++;
            }

            messagesBuffer.erase(idMessage);
            auto it = find(messagesOrderReceived.begin(), messagesOrderReceived.end(), idMessage);
            if (it != messagesOrderReceived.end()) {
                messagesOrderReceived.erase(it);
            } else {
                cout << source <<  "Error in messagesOrderReceived, need to have the same entries as messagesBuffer" << endl;
                exit(1);
            }
            colorCarryMessage();
        } else {
            countMessage--;
            itMessage++;
        }
    }
}

void vehDist::sendMessageToOneNeighborTargetOnlyDelivery(string beaconSource) {
    unsigned short int countMessage = messagesOnlyDelivery.size();
    unordered_map <string, WaveShortMessage>::iterator itMessage = messagesOnlyDelivery.begin();
    string idMessage;
    while(countMessage > 0) {
        if (beaconSource.compare(itMessage->second.getTarget()) == 0) {
            idMessage = itMessage->second.getGlobalMessageIdentificaton();
            cout << source <<" sending MessageToOneNeighborTarget: " << idMessage << " to: " << beaconSource << " and removing from messagesOnlyDelivery" << endl;
            sendWSM(updateBeaconMessageWSM(itMessage->second.dup(), beaconSource));
            vehDist::countMsgPacketSend++;
            messagesDelivered.push_back(idMessage);

            if (countMessage == 1) {
                countMessage = 0;
            } else {
                countMessage--;
                itMessage++;
            }

            messagesOnlyDelivery.erase(idMessage);
            colorCarryMessage();
        } else {
            countMessage--;
            itMessage++;
        }
    }
}

bool vehDist::sendOneNewMessageToOneNeighborTarget(WaveShortMessage wsm) {
    unordered_map <string, WaveShortMessage>::iterator itBeaconStatus = beaconStatusNeighbors.begin();
    string targetMessage = wsm.getTarget();

    while (itBeaconStatus != beaconStatusNeighbors.end()) {
        if (targetMessage.compare(itBeaconStatus->first) == 0) { // Test if this neighbor is the target of the last message received
            cout << source <<" sending OneNewMessageToOneNeighborTarget: " << wsm.getGlobalMessageIdentificaton() << " to: " << itBeaconStatus->first << " and removing" << endl;
            sendWSM(updateBeaconMessageWSM(wsm.dup(), itBeaconStatus->first));
            vehDist::countMsgPacketSend++;
            messagesDelivered.push_back(wsm.getGlobalMessageIdentificaton());

//            messagesBuffer.erase(idMessage);
//            auto it = find(messagesOrderReceived.begin(), messagesOrderReceived.end(), idMessage);
//            if (it != messagesOrderReceived.end()) {
//                messagesOrderReceived.erase(it);
//            } else {
//                cout << source <<  "Error in messagesOrderReceived, need to have the same entries as messagesBuffer" << endl;
//                exit(1);
//            }
//            colorCarryMessage();

            //itBeaconStatus = beaconStatusNeighbors.end();
            return false;
        } else { // Go to next message
            itBeaconStatus++;
        }
    }
    return true;
}

void vehDist::handleLowerMsg(cMessage* msg) {
    WaveShortMessage* wsm = dynamic_cast<WaveShortMessage*>(msg);
    ASSERT(wsm);

    if (wsm->getType() == 1) {
        onBeaconStatus(wsm);
    } else if (wsm->getType() == 2) {
        onBeaconMessage(wsm);
    } else {
        DBG << "unknown message (" << wsm->getName() << ") received" << endl;
        cout << "unknown message (" << wsm->getName() << ") received" << endl;
        exit(1);
    }

    delete(msg);
}

void vehDist::restartFilesResult() {
    string resultFolder = getFolderResult(expSendbyDSCR);
    saveVehStartPosition(resultFolder); // Save the start position of vehicle. Just for test of the seed.

    fileMessagesUnicast = fileMessagesDrop = fileMessagesGenerated = resultFolder;
    fileMessagesUnicast += "Veh_Unicast_Messages.r";
    fileMessagesDrop += "Veh_Messages_Drop.r";
    fileMessagesGenerated += "Veh_Messages_Generated.r";

    // fileMessagesBroadcast and fileMessagesCount not used yet to vehicle

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

void vehDist::saveVehStartPosition(string fileNameLocation) {
    fileNameLocation += "Veh_Position_Initialize.r";
    if (source.compare("car[0]") == 0) {
        if (repeatNumber == 0) {
            myfile.open(fileNameLocation);
        } else {
            myfile.open(fileNameLocation, std::ios_base::app);
        }
        printHeaderfileExecution(ttlBeaconMessage, countGenerateBeaconMessage);
        myfile << "Start Position Vehicles" << endl;
    } else {
        myfile.open(fileNameLocation, std::ios_base::app);
    }
    myfile << source << " : " << mobility->getPositionAt(simTime() + 0.1) << endl;
    myfile.close();
}

void vehDist::vehCreateUpdateRateTimeToSendEvent() {
    rateTimeToSend = 100; // Send in: 100 ms
    rateTimeToSendDistanceControl = 10; // Equal to 10 m in 1 s
    rateTimeToSendLimitTime = par("beaconMessageInterval").longValue(); // #5
    rateTimeToSendLimitTime = rateTimeToSendLimitTime * 1000; // #5000 // Limit that rateTimeToSend can be (one message by 5000 ms), value must be (bufferMessage limit) * rateTimeToSend, in this case 50 * 100 = 5000
    rateTimeToSendUpdateTime = 1; // Will update every 1 s

    sendUpdateRateTimeToSendVeh = new cMessage("Event update rateTimeToSend vehicle", SendEvtUpdateRateTimeToSendVeh);
    //cout << source << " at: " << simTime() << " schedule created UpdateRateTimeToSend to: "<< (simTime() + rateTimeToSendUpdateTime) << endl;
    scheduleAt((simTime() + rateTimeToSendUpdateTime), sendUpdateRateTimeToSendVeh);
}

void vehDist::vehGenerateBeaconMessageBegin() {
    if (sendData) {
        sendGenerateBeaconMessageEvt = new cMessage("Event generate beacon message", SendEvtGenerateBeaconMessage);
        //cout << source << " at: " << simTime() << " schedule created sendGenerateMessageEvt to: "<< (simTime() + vehOffSet) << endl;
        scheduleAt((simTime() + vehOffSet), sendGenerateBeaconMessageEvt);
    }
}

void vehDist::vehCreateEventTrySendBeaconMessage() {
    if (sendData) {
        sendBeaconMessageEvt = new cMessage("Event send beacon message", SendEvtBeaconMessage);
        timeToFinishLastStartSend = simTime() + vehOffSet;
        messageToSend = 0; // messagesOrderReceived.front();
        //cout << source << " at: "<< simTime() << " schedule created SendBeaconMessage to: "<< timeToFinishLastStartSend << endl;
        scheduleAt(timeToFinishLastStartSend, sendBeaconMessageEvt);
        timeToFinishLastStartSend += double(rateTimeToSendLimitTime)/1000; // /100 because works with s instead ms
    }
}

void vehDist::vehGenerateBeaconMessageAfterBegin() {
    selectVehGenerateMessage();

    auto it = find(vehDist::vehGenerateMessage.begin(), vehDist::vehGenerateMessage.end(), source);
    if (it != vehDist::vehGenerateMessage.end()) { // if have "vehNumber" on buffer, will generate one message
        generateBeaconMessage();
        vehDist::vehGenerateMessage.erase(it);
    }
}

void vehDist::selectVehGenerateMessage() {
    if (myId == 0) { // if true, some vehicle has (in past) selected the vehicle to generate messages
        if (simTime() <= vehDist::timeLimitGenerateBeaconMessage) { // Modify the generate message and test vehDist::timeLimitGenerateBeaconMessage
            cout << source << " at " << simTime() << " vehDist::timeLimitGenerateBeaconMessage: " << vehDist::timeLimitGenerateBeaconMessage << endl;

            unsigned short int vehSelected;
            myfile.open(fileMessagesGenerated, std::ios_base::app); // To save info (Id and vehicle generate) on fileMessagesGenerated
            unsigned short int trySelectVeh = 0;
            for (unsigned short int i = 0; i < countGenerateBeaconMessage;) { // select <countGenerateBeaconMessage> distinct vehicle to generate messages
                //vehSelected = intuniform(1, (vehDist::numVehicles -1), repeatNumber); // Get random number
                //cout << getRNG(0) << endl; // Get seed used on random number

                //uniform_int_distribution <int> dist(0, (vehDist::numVehicles.size() - 1));

                // TODO Need of new way to generate messages
                //uniform_int_distribution <int> dist(0, 9);
                uniform_int_distribution <int> dist(0, (vehDist::numVehicles.size() - 1));
                vehSelected = dist(mt_veh);
                string vehSelectedId = vehDist::numVehicles[vehSelected];

                if ((vehDist::vehScenario[vehSelectedId].getTimestamp() + 60) >= simTime()) { // Test if veh are less than 60 s in the scenario
                    auto it = find(vehDist::vehGenerateMessage.begin(), vehDist::vehGenerateMessage.end(), vehSelectedId);
                    if (it == vehDist::vehGenerateMessage.end()) {
                        vehDist::vehGenerateMessage.push_back(vehSelectedId);
                        cout << endl << source << " selected the " << vehSelectedId << " to generate a message at: " << simTime() << endl;
                        myfile << source << " selected the " << vehSelectedId << " to generate a message at: " << simTime() << endl;
                        i++;
                    }
                } else {
                    cout << source << " selected " << vehSelectedId << " to generate " << vehDist::beaconMessageId;
                    cout << " message, but has Timestamp: " << vehDist::vehScenario[vehSelectedId].getTimestamp() <<" at " << simTime() << endl;
                    if (trySelectVeh > (vehDist::vehScenario.size() * 4)) {
                        cout << "Error loop in select vehicle to generate message, go to vehDist.cc line 953" << endl;
                        cout << "trySelectVeh: " << trySelectVeh << " vehDist::vehScenario.size(): " << vehDist::vehScenario.size() << endl;
                        exit(3);
                    }
                }
                trySelectVeh++;
            }
            myfile.close();
        }
    }
}

WaveShortMessage* vehDist::prepareBeaconStatusWSM(std::string name, int lengthBits, t_channel channel, int priority, int serial) {
    WaveShortMessage* wsm = new WaveShortMessage(name.c_str());
    wsm->setType(1); // Beacon of Status
    wsm->addBitLength(headerLength);
    wsm->addBitLength(lengthBits);
    switch (channel) {
        case type_SCH:
            wsm->setChannelNumber(Channels::SCH1);
            break; //will be rewritten at Mac1609_4 to actual Service Channel. This is just so no controlInfo is needed
        case type_CCH:
            wsm->setChannelNumber(Channels::CCH);
            break;
    }
    wsm->setPsid(0);
    wsm->setPriority(priority);
    wsm->setWsmVersion(1);
    wsm->setSerial(serial);
    wsm->setTimestamp(simTime());
    wsm->setSource(source.c_str());

    // beaconStatus don't need
    //wsm->setRecipientAddressTemporary();  // => "BROADCAST"
    //wsm->setSenderAddressTemporary(source);
    //wsm->setTarget(); // => "BROADCAST"

    wsm->setRoadId(mobility->getRoadId().c_str());
    wsm->setSenderSpeed(mobility->getSpeed());
    wsm->setCategory(vehCategory.c_str());
    wsm->setSenderPos(curPosition);
    if (simTime() < vehDist::timeToUpdatePosition) {
        wsm->setSenderPosPrevious(mobility->getPositionAt(0.1));
    } else {
        wsm->setSenderPosPrevious(mobility->getPositionAt(simTime() - vehDist::timeToUpdatePosition));
    }
    wsm->setRateTimeToSend(rateTimeToSend);

    // heading 1 to 4 or 1 to 8
    wsm->setHeading(getVehHeading4()); //wsm->setHeading(getVehHeading8());

    DBG << "Creating BeaconStatus with Priority " << priority << " at Applayer at " << wsm->getTimestamp() << endl;
    return wsm;
}

void vehDist::generateTarget() { // Set the target node to who the message has to be delivered
    target = "rsu[0]";
    target_x = par("vehBeaconMessageTarget_x").longValue();
    target_y = par("vehBeaconMessageTarget_y").longValue();
}

void vehDist::generateBeaconMessage() {
    WaveShortMessage* wsm = new WaveShortMessage("beaconMessage");
    wsm->setType(2); // Beacon of Message
    wsm->addBitLength(headerLength);
    wsm->addBitLength(dataLengthBits);
    t_channel channel = dataOnSch ? type_SCH : type_CCH;
    switch (channel) {
        case type_SCH:
            wsm->setChannelNumber(Channels::SCH1);
            break; //will be rewritten at Mac1609_4 to actual Service Channel. This is just so no controlInfo is needed
        case type_CCH:
            wsm->setChannelNumber(Channels::CCH);
            break;
    }
    wsm->setPsid(0);
    wsm->setPriority(dataPriority);
    wsm->setWsmVersion(1);
    wsm->setSenderPos(curPosition);
    wsm->setSerial(2);
    wsm->setTimestamp(simTime());

    wsm->setSource(source.c_str());
    generateTarget(); //target = rsu[0], rsu[1] or car[*] and the respective position.
    wsm->setTarget(target.c_str());
    wsm->setTargetPos(Coord(target_x, target_y, 3));
    wsm->setSenderAddressTemporary(source.c_str());
    wsm->setRecipientAddressTemporary("Initial"); // defined in time before send

    wsm->setHopCount(beaconMessageHopLimit + 1); // Is beaconMessageHopLimit+1 because hops count is equals to routes in the path, not hops.
    string wsmDataTmp = "WSMData generated by " + source;
    wsm->setWsmData(wsmDataTmp.c_str());

    myfile.open(fileMessagesGenerated, std::ios_base::app); // Save info (Id and vehicle generate) on fileMessagesGenerated
    myfile << "                                                                     ";
    if (vehDist::beaconMessageId < 10) {
        string msgIdTmp = '0' + to_string(vehDist::beaconMessageId);
        wsm->setGlobalMessageIdentificaton(msgIdTmp.c_str()); // Id 01 to 09
    } else {
        wsm->setGlobalMessageIdentificaton(to_string(vehDist::beaconMessageId).c_str()); // Id 10 and going on
    }
    myfile << "### " << source << " generated the message ID: " << wsm->getGlobalMessageIdentificaton() << " at: " << simTime() << endl;
    cout << "### " << source << " generated the message ID: " << wsm->getGlobalMessageIdentificaton() << " at: " << simTime() << endl;
    myfile.close();

    messagesBuffer.insert(make_pair(wsm->getGlobalMessageIdentificaton(),*wsm)); // Adding the message on the buffer
    if (messagesBuffer.size() > msgBufferMaxUse) {
        msgBufferMaxUse = messagesBuffer.size();
    }
    messagesOrderReceived.push_back(wsm->getGlobalMessageIdentificaton());
    colorCarryMessage(); // Change the range-color in the vehicle (GUI)
    vehDist::beaconMessageId++;
}

WaveShortMessage* vehDist::updateBeaconMessageWSM(WaveShortMessage* wsm, string rcvId) {
    wsm->setSenderAddressTemporary(source.c_str());
    wsm->setRecipientAddressTemporary(rcvId.c_str());
    wsm->setRoadId(mobility->getRoadId().c_str());
    wsm->setCategory(vehCategory.c_str());
    wsm->setSenderSpeed(mobility->getSpeed());
    wsm->setSenderPos(curPosition);
    if (simTime() < vehDist::timeToUpdatePosition) {
        wsm->setSenderPosPrevious(mobility->getPositionAt(0.1));
    } else {
        wsm->setSenderPosPrevious(mobility->getPositionAt(simTime() - vehDist::timeToUpdatePosition));
    }
    wsm->setHeading(getVehHeading4()); //wsm->setHeading(getVehHeading8());
    wsm->setHopCount(wsm->getHopCount() -1);
    return wsm;
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
        case SendEvtUpdateRateTimeToSendVeh: {
            vehUpdateRateTimeToSend();
            scheduleAt((simTime() + rateTimeToSendUpdateTime), sendUpdateRateTimeToSendVeh);
            break;
        }
        case SendEvtGenerateBeaconMessage: {
            vehGenerateBeaconMessageAfterBegin();
            scheduleAt((simTime() + par("timeGenerateBeaconMessage").doubleValue()), sendGenerateBeaconMessageEvt);
            break;
        }
        default: {
            if (msg) {
                DBG << "APP: Error: Got Self Message of unknown kind! Name: " << msg->getName() << endl;
            }
            break;
        }
    }
}

void vehDist::vehUpdateRateTimeToSend() {
    //cout << source << " rateTimeToSend: " << rateTimeToSend;
    unsigned short int distance = traci->getDistance(mobility->getPositionAt(simTime() - rateTimeToSendUpdateTime), curPosition, false);

    if (distance >= rateTimeToSendDistanceControl){
        if(rateTimeToSend > 100) { // rateTimeToSend > 0.1f
            rateTimeToSend -= 100;
        }
    } else {
        if (rateTimeToSend < rateTimeToSendLimitTime){
            rateTimeToSend += 100;
        }
    }
    //cout << " updated to: " << rateTimeToSend << " at: " << simTime() << " by: " <<  distance << " traveled [" << mobility->getPositionAt(simTime() - rateTimeToSendUpdateTime) << " " << curPosition << "]" << endl;
}

void vehDist::printMessagesBuffer() {
    if (!messagesBuffer.empty()) {
        cout << endl << "messagesBuffer from " << source << " at: " << simTime() << " position: " << curPosition << endl;
        unordered_map<string, WaveShortMessage>::iterator it;
        for (it = messagesBuffer.begin(); it != messagesBuffer.end(); it++) {
            cout << "    Id(message): " << it->second.getGlobalMessageIdentificaton() << endl;
            cout << "    WsmData: " << it->second.getWsmData() << endl;
            cout << "    Source: " << it->second.getSource() << endl;
            cout << "    Target: " << it->second.getTarget() << endl;
            cout << "    Timestamp: " << it->second.getTimestamp() << endl;
            cout << "    HopCount: " << it->second.getHopCount() << endl << endl << endl;
        }
    } else {
        cout << endl << "messagesBuffer from " << source << " is empty now: " << simTime() << " position: " << curPosition << endl << endl;
    }
}

void vehDist::printBeaconStatusNeighbors() {
    if (!beaconStatusNeighbors.empty()) {
        cout << endl << "beaconStatusNeighbors from " << source << " at: " << simTime() << " position: " << curPosition << endl;
        unordered_map<string, WaveShortMessage>::iterator it;
        for (it = beaconStatusNeighbors.begin(); it != beaconStatusNeighbors.end(); it++) {
            cout << "    Id(vehicle): " << it->first << endl;
            cout << "    PositionPrevious: " << it->second.getSenderPosPrevious() << endl;
            cout << "    Position: " << it->second.getSenderPos() << endl;
            cout << "    Speed: " << it->second.getSenderSpeed() << endl;
            cout << "    Category: " << it->second.getCategory() << endl;
            cout << "    RoadId: " << it->second.getRoadId() << endl;
            cout << "    Heading: " << it->second.getHeading() << endl;
            cout << "    Timestamp: " << it->second.getTimestamp() << endl;
            cout << "    RateTimeToSend: " << it->second.getRateTimeToSend() << endl << endl;
        }
    } else {
        cout << endl << "beaconStatusNeighbors from " << source << " is empty now: " << simTime() << " position: " << curPosition << endl;
    }
}

//##############################################################################################################
void vehDist::onData(WaveShortMessage* wsm) {
}

void vehDist::onBeacon(WaveShortMessage* wsm) {
}

// Not used ###############################################################

unsigned short int vehDist::getVehHeading4() {
    // marcospaiva.com.br/images/rosa_dos_ventos%2002.GIF
    // marcospaiva.com.br/localizacao.htm

    double angle;
    if (mobility->getAngleRad() < 0) { // radians are negative, so degrees negative
        angle = (((mobility->getAngleRad() + 2 * M_PI ) * 180)/ M_PI);
    } else { // radians are positive, so degrees positive
        angle = ((mobility->getAngleRad() * 180) / M_PI);
    }

    if ((angle >= 315 && angle < 360) || (angle >= 0 && angle < 45)) {
        return 1; // L or E => 0º
    } else if (angle >= 45 && angle < 135) {
        return 2; // N => 90º
    } else if (angle >= 135  && angle < 225) {
        return 3; // O or W => 180º
    } else if (angle >= 225  && angle < 315) {
        return 4; // S => 270º
    } else {
        return 9; // Error
    }
}

unsigned short int vehDist::getVehHeading8() {
    // marcospaiva.com.br/images/rosa_dos_ventos%2002.GIF
    // marcospaiva.com.br/localizacao.htm

    double angle;
    if (mobility->getAngleRad() < 0) { // radians are negative, so degrees negative
        angle = (((mobility->getAngleRad() + 2 * M_PI ) * 180)/ M_PI);
    } else { // radians are positive, so degrees positive
        angle = ((mobility->getAngleRad() * 180) / M_PI);
    }

    if ((angle >= 337.5 && angle < 360) || (angle >= 0 && angle < 22.5)) {
        return 1; // L or E => 0º
    } else if (angle >= 22.5 && angle < 67.5) {
        return 2; // NE => 45º
    } else if (angle >= 67.5  && angle < 112.5) {
        return 3; // N => 90º
    } else if (angle >= 112.5  && angle < 157.5) {
        return 4; // NO => 135º
    } else if (angle >= 157.5  && angle < 202.5) {
        return 5; // O or W => 180º
    } else if (angle >= 202.5  && angle < 247.5) {
        return 6; // SO => 225º
    } else if (angle >= 247.5  && angle < 292.5) {
        return 7; // S => 270º
    } else if (angle >= 292.5  && angle < 337.5) {
        return 8; // SE => 315º
    } else {
        return 9; // Error
    }
}
