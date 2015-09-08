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

#include "application/mfcv/mfcv.h"

using Veins::TraCIMobilityAccess;
using Veins::AnnotationManagerAccess;

const simsignalwrap_t mfcv::parkingStateChangedSignal = simsignalwrap_t(TRACI_SIGNAL_PARKING_CHANGE_NAME);

Define_Module(mfcv);

void mfcv::initialize(int stage) {
	BaseWaveApplLayer::initialize_mfcv(stage);
	if (stage == 0) {
		traci = TraCIMobilityAccess().get(getParentModule());
		annotations = AnnotationManagerAccess().getIfExists();
		ASSERT(annotations);

		sentMessage = false;
		lastDroveAt = simTime();
		findHost()->subscribe(parkingStateChangedSignal, this);
		isParking = false;
		sendWhileParking = par("sendWhileParking").boolValue();
	}
}

void mfcv::onBeacon(WaveShortMessage* wsm) {

    DBG << "\n wsm:" << wsm;
    DBG << "\n wsm->getArrivalTime():" << wsm->getArrivalTime();
    DBG << "\n wsm->getRoadId():" << wsm->getRoadId();
    DBG << "\n wsm->getFullName():" << wsm->getFullName();
    DBG << "\n wsm->getWsmVersion():" << wsm->getWsmVersion();
    DBG << "\n wsm->getSecurityType():" << wsm->getSecurityType();
    DBG << "\n wsm->getChannelNumber():" << wsm->getChannelNumber();
    DBG << "\n wsm->getDataRate():" << wsm->getDataRate();
    DBG << "\n wsm->getPriority():" << wsm->getPriority();
    DBG << "\n wsm->getPsid():" << wsm->getPsid();
    DBG << "\n wsm->getPsc():" << wsm->getPsc();
    DBG << "\n wsm->getTimestamp():" << wsm->getTimestamp();
    DBG << "\n wsm->getSenderPos():" << wsm->getSenderPos();
    DBG << "\n wsm->:getWsmLength():" << wsm->getWsmLength();
    DBG << "\n wsm->:getWsmData():" << wsm->getWsmData();
    DBG << "\n wsm->:getSenderSpeed():" << wsm->getSenderSpeed();
    DBG << "\n wsm->:getSenderAddress():" << wsm->getSenderAddress();
    DBG << "\n wsm->:getRecipientAddress():" << wsm->getRecipientAddress();
    DBG << "\n wsm->getSerial():" << wsm->getSerial();
    DBG << "\n wsm->getArrivalTime():" << wsm->getArrivalTime();
    DBG << "\n wsm->getDisplayString():" << wsm->getDisplayString();
    DBG << "\n wsm->getBitLength():" << wsm->getBitLength() << "\n\n";

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

void mfcv::onData(WaveShortMessage* wsm) {
	findHost()->getDisplayString().updateWith("r=16,green");
	annotations->scheduleErase(1, annotations->drawLine(wsm->getSenderPos(), traci->getPositionAt(simTime()), "blue"));

	if (traci->getRoadId()[0] != ':') traci->commandChangeRoute(wsm->getWsmData(), 9999);
	if (!sentMessage) sendMessage(wsm->getWsmData());
}

void mfcv::sendMessage(std::string blockedRoadId) {
	/*
    sentMessage = true;

	t_channel channel = dataOnSch ? type_SCH : type_CCH;
	WaveShortMessage* wsm = prepareWSM("data", dataLengthBits, channel, dataPriority, -1,2);
	wsm->setWsmData(blockedRoadId.c_str());
	sendWSM(wsm);
	*/
}
void mfcv::receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj) {
	Enter_Method_Silent();
	if (signalID == mobilityStateChangedSignal) {
		handlePositionUpdate(obj);
	}
	else if (signalID == parkingStateChangedSignal) {
		handleParkingUpdate(obj);
	}
}
void mfcv::handleParkingUpdate(cObject* obj) {
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
void mfcv::handlePositionUpdate(cObject* obj) {
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
void mfcv::sendWSM(WaveShortMessage* wsm) {
	if (isParking && !sendWhileParking) return;
	sendDelayedDown(wsm,individualOffset);
}

//
void mfcv::handleSelfMsg(cMessage* msg) {
    switch (msg->getKind()) {
        case SEND_BEACON_EVT: {
            sendWSM(prepareWSM_node("beacon_node", beaconLengthBits, type_CCH, beaconPriority, 0, -1));
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

void mfcv::handleLowerMsg(cMessage* msg) {

    WaveShortMessage* wsm = dynamic_cast<WaveShortMessage*>(msg);
    ASSERT(wsm);

    if (std::string(wsm->getName()) == "beacon_node") {
        DBG << "Beacon (node) recebido!";
        onBeacon(wsm);
    }
    else if (std::string(wsm->getName()) == "beacon_rsu") {
        DBG << "Beacon (rsu) recebido!";
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

WaveShortMessage* mfcv::prepareWSM_node(std::string name, int lengthBits, t_channel channel, int priority, int rcvId, int serial) {
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
    else if (name == "beacon_node") {
        DBG << "Creating Beacon_node with Priority " << priority << " at Applayer at " << wsm->getTimestamp() << std::endl;

    }
    else if (name == "data") {
        DBG << "Creating Data with Priority " << priority << " at Applayer at " << wsm->getTimestamp() << std::endl;
    }
    return wsm;
}
