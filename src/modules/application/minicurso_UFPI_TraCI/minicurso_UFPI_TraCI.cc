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

#include "application/minicurso_UFPI_TraCI/minicurso_UFPI_TraCI.h"

using Veins::TraCIMobilityAccess;
using Veins::AnnotationManagerAccess;

const simsignalwrap_t minicurso_UFPI_TraCI::parkingStateChangedSignal = simsignalwrap_t(TRACI_SIGNAL_PARKING_CHANGE_NAME);

Define_Module(minicurso_UFPI_TraCI);

void minicurso_UFPI_TraCI::initialize(int stage) {
    BaseWaveApplLayer::initialize_minicurso_UFPI_TraCI(stage);
    if (stage == 0) {
        traci = TraCIMobilityAccess().get(getParentModule());
        annotations = AnnotationManagerAccess().getIfExists();
        ASSERT(annotations);

        //sentMessage = false;
        lastDroveAt = simTime();
        findHost()->subscribe(parkingStateChangedSignal, this);
        isParking = false;
        sendWhileParking = par("sendWhileParking").boolValue();

        // Adicionado (Minicurso_UFPI)
        // Inicialize as variáveis
        //std::cout << "void minicurso_UFPI_TraCI::initialize\n";
        lastSent = simTime();
        updateTablesEvt = new cMessage("updateTable");
        scheduleAt(simTime() + 5, updateTablesEvt);
        insertCurrentSpeedEvt = new cMessage("insertCurrentSpeed");
        scheduleAt(simTime() + 1, insertCurrentSpeedEvt);
    }
}

void minicurso_UFPI_TraCI::onBeacon(WaveShortMessage* wsm) {
    // Adicionado (Minicurso_UFPI)
    if (traci->getRoadId().compare(wsm->getRoadId()) == 0)
        speedsList.push_back(make_pair(wsm->getArrivalTime(), wsm->getSenderSpeed()));
    std::cout << "\nminicurso_UFPI_TraCI::onBeacon, wsm-speed:" << wsm->getSenderSpeed() ;

}

/*
void minicurso_UFPI_TraCI::onData(WaveShortMessage* wsm) {
    findHost()->getDisplayString().updateWith("r=16,green");
    annotations->scheduleErase(1, annotations->drawLine(wsm->getSenderPos(), traci->getPositionAt(simTime()), "blue"));

    //if (traci->getRoadId()[0] != ':') traci->commandChangeRoute(wsm->getWsmData(), 9999);
    //if (!sentMessage) sendMessage(wsm->getWsmData());
}
*/

void minicurso_UFPI_TraCI::sendMessage(std::string blockedRoadId) {
    //sentMessage = true;

    t_channel channel = dataOnSch ? type_SCH : type_CCH;
    WaveShortMessage* wsm = prepareWSM("data", dataLengthBits, channel, dataPriority, -1,2);
    wsm->setWsmData(blockedRoadId.c_str());
    sendWSM(wsm);
}
void minicurso_UFPI_TraCI::receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj) {
    Enter_Method_Silent();
    if (signalID == mobilityStateChangedSignal) {
        handlePositionUpdate(obj);
    }
    else if (signalID == parkingStateChangedSignal) {
        handleParkingUpdate(obj);
    }
}
void minicurso_UFPI_TraCI::handleParkingUpdate(cObject* obj) {
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
void minicurso_UFPI_TraCI::handlePositionUpdate(cObject* obj) {

    BaseWaveApplLayer::handlePositionUpdate(obj);

    // stopped for for at least 10s?
/*
     if (traci->getSpeed() < 1) {
        if (simTime() - lastDroveAt >= 10) {
            findHost()->getDisplayString().updateWith("r=16,red");
            //if (!sentMessage) sendMessage(traci->getRoadId());
        }
    }
    else {
        lastDroveAt = simTime();
    }
    */

}
void minicurso_UFPI_TraCI::sendWSM(WaveShortMessage* wsm) {
    if (isParking && !sendWhileParking) return;
    sendDelayedDown(wsm,individualOffset);
}

// Adicionado (Minicurso_UFPI)
void minicurso_UFPI_TraCI::handleSelfMsg(cMessage* msg){
    if(msg == updateTablesEvt){
      updateSpeedList();
      updateCongestionTable();
      verifyAndSendCongestionMessage();
      scheduleAt(simTime() + 5.0, updateTablesEvt);
    }
    else if(msg ==  insertCurrentSpeedEvt){
      speedsList.push_back(make_pair(simTime(),traci->getSpeed()));
      congestionTable[traci->getRoadId()].push_back(make_pair(simTime(),traci->getSpeed()));
      scheduleAt(simTime() + 1.0, insertCurrentSpeedEvt);
    }
    else{
      BaseWaveApplLayer::handleSelfMsg(msg);
    }
}
void minicurso_UFPI_TraCI::updateSpeedList() {
    vector<pair<simtime_t, double> > newSpeedsList;

    for(int i=0;i < speedsList.size(); ++i)
        if(simTime() - speedsList[i].first < 5.0)
            newSpeedsList.push_back(speedsList[i]);

    speedsList.clear();
    speedsList.insert(speedsList.begin(),newSpeedsList.begin(), newSpeedsList.end());
}
void minicurso_UFPI_TraCI::updateCongestionTable() {
    map<string, vector<pair<simtime_t, double> > > newMap;
    for(map<string, vector<pair<simtime_t, double> > >::iterator it=congestionTable.begin(); it!= congestionTable.end(); ++it){
        vector<pair<simtime_t, double> > newList;
        for(int i=0; i < it->second.size(); ++i)
            if(simTime() - it->second[i].first < 20.0)
                newList.push_back(it->second[i]);
        newMap[it->first] = newList;
    }
    congestionTable.clear();
    congestionTable.insert(newMap.begin(),newMap.end());
}
void minicurso_UFPI_TraCI::verifyAndSendCongestionMessage() {
    if(simTime() - lastSent < 5.0)
        return;
    if(speedsList.size() == 0)
            return;
    double sum = 0;
    for(int i = 0; i < speedsList.size(); ++i)
        sum += speedsList[i].second;
    double avg = sum/speedsList.size();
    if(avg < 5.0){
        string toSend = "";
        toSend += traci->getRoadId() + "|" + to_string(avg) + "|" + to_string(++minicurso_UFPI_TraCI::messageId);
        lastSent = simTime();
        sendMessage(toSend);
    }
    double length = traci->commandGetLaneLength(traci->commandGetLaneId());
    if(length > 0)
        traci->commandChangeRoute(traci->getRoadId(), avg/length);
}

// Novo método onData
void minicurso_UFPI_TraCI::onData(WaveShortMessage* wsm) {
    findHost()->getDisplayString().updateWith("r=16,green");
    annotations->scheduleErase(1, annotations->drawLine(wsm->getSenderPos(), traci->getPositionAt(simTime()), "blue"));
    string data(wsm->getWsmData());
    string road, speed, messageIdentifier;
    std::cout << "\nminicurso_UFPI_TraCI::onData, wsm-data:\n" << data;
    int i = 0;
    while(i < data.size() && data[i] != '|'){
        road.push_back(data[i]);
        ++i;
    }
    ++i;
    while(i < data.size()){
        speed.push_back(data[i]);
        ++i;
    }
    ++i;
    int msgId = atoi(messageIdentifier.c_str());
    if(sentMessages.count(msgId) == 0){
        sentMessages[msgId] == true;
        sendMessage(wsm->getWsmData());
    }
    while(i < data.size()){
        messageIdentifier.push_back(data[i]);
        ++i;
    }
    congestionTable[road].push_back(make_pair(simTime(), stod(speed,0)));
    if(road.compare(traci->getRoadId()) == 0)
        lastSent = simTime();
    vector<pair<simtime_t, double> > speeds = congestionTable[road];
    double sum = 0;
    for(int i = 0; i < speeds.size(); ++i)
        sum += speeds[i].second;
    double avg = sum/speeds.size();
    double length = traci->commandGetLaneLength(traci->commandGetLaneId());
    if(length>0)
        traci->commandChangeRoute(road,avg/length);
    //if (traci->getRoadId()[0] != ':') traci->commandChangeRoute(wsm->getWsmData(), 9999);
    //if (!sentMessage) sendMessage(wsm->getWsmData());
}
