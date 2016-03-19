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

#include "application/mfcv/mfcv_rsu.h"

using Veins::AnnotationManagerAccess;

Define_Module(mfcv_rsu);

void mfcv_rsu::initialize(int stage) {
    BaseWaveApplLayer::initialize_default_veins_TraCI(stage);
    if (stage == 0) {
        mobi = dynamic_cast<BaseMobility*> (getParentModule()->getSubmodule("mobility"));
        ASSERT(mobi);
        annotations = AnnotationManagerAccess().getIfExists();
        ASSERT(annotations);
    }
}

void mfcv_rsu::onBeacon(WaveShortMessage* wsm) {

    std::cout << "\n wsm:" << wsm;
    std::cout << "\n wsm->getArrivalTime():" << wsm->getArrivalTime();
    std::cout << "\n wsm->getRoadId():" << wsm->getRoadId();
    std::cout << "\n wsm->getFullName():" << wsm->getFullName();
    std::cout << "\n wsm->getWsmVersion():" << wsm->getWsmVersion();
    std::cout << "\n wsm->getSecurityType():" << wsm->getSecurityType();
    std::cout << "\n wsm->getChannelNumber():" << wsm->getChannelNumber();
    std::cout << "\n wsm->getDataRate():" << wsm->getDataRate();
    std::cout << "\n wsm->getPriority():" << wsm->getPriority();
    std::cout << "\n wsm->getPsid():" << wsm->getPsid();
    std::cout << "\n wsm->getPsc():" << wsm->getPsc();
    std::cout << "\n wsm->getTimestamp():" << wsm->getTimestamp();
    std::cout << "\n wsm->getSenderPos():" << wsm->getSenderPos();
    std::cout << "\n wsm->:getWsmLength():" << wsm->getWsmLength();
    std::cout << "\n wsm->:getWsmData():" << wsm->getWsmData();
    std::cout << "\n wsm->:getSenderSpeed():" << wsm->getSenderSpeed();
    std::cout << "\n wsm->:getSenderAddress():" << wsm->getSenderAddress();
    std::cout << "\n wsm->:getRecipientAddress():" << wsm->getRecipientAddress();
    std::cout << "\n wsm->getSerial():" << wsm->getSerial();
    std::cout << "\n wsm->getArrivalTime():" << wsm->getArrivalTime();
    std::cout << "\n wsm->getDisplayString():" << wsm->getDisplayString();
    std::cout << "\n wsm->getBitLength():" << wsm->getBitLength() << "\n\n";

}

void mfcv_rsu::onData(WaveShortMessage* wsm) {
    findHost()->getDisplayString().updateWith("r=16,green");

    annotations->scheduleErase(1, annotations->drawLine(wsm->getSenderPos(), mobi->getCurrentPosition(), "blue"));

    if (!sentMessage) sendMessage(wsm->getWsmData());
}

void mfcv_rsu::sendMessage(std::string blockedRoadId) {
    sentMessage = true;
    t_channel channel = dataOnSch ? type_SCH : type_CCH;
    WaveShortMessage* wsm = prepareWSM("data", dataLengthBits, channel, dataPriority, -1,2);
    wsm->setWsmData(blockedRoadId.c_str());
    sendWSM(wsm);
}
void mfcv_rsu::sendWSM(WaveShortMessage* wsm) {
    sendDelayedDown(wsm,individualOffset);
}

//
void mfcv_rsu::handleSelfMsg(cMessage* msg) {
    switch (msg->getKind()) {
        case SEND_BEACON_EVT: {
            sendWSM(prepareWSM_rsu("beacon_rsu", beaconLengthBits, type_CCH, beaconPriority, 0, -1));
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


void mfcv_rsu::handleLowerMsg(cMessage* msg) {

    WaveShortMessage* wsm = dynamic_cast<WaveShortMessage*>(msg);
    ASSERT(wsm);

    if (std::string(wsm->getName()) == "beacon_rsu") {
        DBG << "Beacon (rsu) recebido!";
        onBeacon(wsm);
    }
    else if (std::string(wsm->getName()) == "beacon_node") {
        DBG << "Beacon (node) recebido!";
        onBeacon(wsm);
    }
    else if (std::string(wsm->getName()) == "data") {
        onData(wsm);
    }
    else {
        DBG << "unknown message (" << wsm->getName() << ")  received\n";
    }
    delete(msg);
}

WaveShortMessage* mfcv_rsu::prepareWSM_rsu(std::string name, int lengthBits, t_channel channel, int priority, int rcvId, int serial) {
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
    wsm->setSenderAddress(myId);
    wsm->setRecipientAddress(rcvId);
    wsm->setSenderPos(curPosition);
    wsm->setSerial(serial);

    if (name == "beacon") {

        // Adicionado (Minicurso_UFPI)
        //wsm->setRoadId(TraCIMobilityAccess().get(getParentModule()) ->getRoadId().c_str());
        //wsm->setSenderSpeed(TraCIMobilityAccess(). get(getParentModule())->getSpeed());

       //DBG << "\n ttt" << wsm->getSenderSpeed() << " ttt\n";

        DBG << "Creating Beacon with Priority " << priority << " at Applayer at " << wsm->getTimestamp() << std::endl;
    }
    else if (name == "beacon_rsu") {
           DBG << "Creating Beacon_rsu with Priority " << priority << " at Applayer at " << wsm->getTimestamp() << std::endl;

    }
    else if (name == "data") {
        DBG << "Creating Data with Priority " << priority << " at Applayer at " << wsm->getTimestamp() << std::endl;
    }
    return wsm;
}

