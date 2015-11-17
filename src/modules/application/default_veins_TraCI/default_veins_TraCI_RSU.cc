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

#include "application/default_veins_TraCI/default_veins_TraCI_RSU.h"

using Veins::AnnotationManagerAccess;

Define_Module(default_veins_TraCI_RSU);

void default_veins_TraCI_RSU::initialize(int stage) {
    BaseWaveApplLayer::initialize_default_veins_TraCI(stage);
    if (stage == 0) {
        mobi = dynamic_cast<BaseMobility*> (getParentModule()->getSubmodule("mobility"));
        ASSERT(mobi);
        annotations = AnnotationManagerAccess().getIfExists();
        ASSERT(annotations);
    }
}

void default_veins_TraCI_RSU::onBeacon(WaveShortMessage* wsm) {
    /*DBG << "\n wsm:" << wsm;
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
*/

}

void default_veins_TraCI_RSU::onData(WaveShortMessage* wsm) {
    findHost()->getDisplayString().updateWith("r=16,green");

    annotations->scheduleErase(1, annotations->drawLine(wsm->getSenderPos(), mobi->getCurrentPosition(), "blue"));

    if (!sentMessage) sendMessage(wsm->getWsmData());
}

void default_veins_TraCI_RSU::sendMessage(std::string blockedRoadId) {
    sentMessage = true;
    t_channel channel = dataOnSch ? type_SCH : type_CCH;
    WaveShortMessage* wsm = prepareWSM("data", dataLengthBits, channel, dataPriority, -1,2);
    wsm->setWsmData(blockedRoadId.c_str());
    sendWSM(wsm);
}
void default_veins_TraCI_RSU::sendWSM(WaveShortMessage* wsm) {
    sendDelayedDown(wsm,individualOffset);
}
