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

#include "application/vehDist/vehDist_rsu.h"

using Veins::AnnotationManagerAccess;

Define_Module(vehDist_rsu);

void vehDist_rsu::initialize(int stage) {
    BaseWaveApplLayer::initialize_default_veins_TraCI(stage);
    if (stage == 0) {
        mobi = dynamic_cast<BaseMobility*> (getParentModule()->getSubmodule("mobility"));
        ASSERT(mobi);
        annotations = AnnotationManagerAccess().getIfExists();
        ASSERT(annotations);

        if (strcmp(findHost()->getFullName(), "rsu[0]") == 0){
            //create a folder results
            system("mkdir results");
            //Open a new file for the current simulation
            myfile.open ("results/RSUBroadcastMessages.txt");
            myfile.close();

            fileMessagesName = "results/";
            fileMessagesName += findHost()->getFullName();
            fileMessagesName += "MessagesReceived.txt";
            //cout  << "fileMessagesName " << fileMessagesName << endl;
            myfile.open (fileMessagesName);
            myfile.close();
        }
        //cout << " " << findHost()->getFullName() << " entered in the scenario" << endl;
        //cout << endl << findHost()->getFullName() << " myId " <<  myId << endl;
    }
}

void vehDist_rsu::onBeacon(WaveShortMessage* wsm) {

}

void vehDist_rsu::onData(WaveShortMessage* wsm){

    if (strcmp(wsm->getRecipientAddressString(), findHost()->getFullName()) == 0){
        findHost()->bubble("Received data");
        wsm->setTimestamp(simTime());
        recordOnFileMessages(wsm);
    }
    else{
   // else if (strcmp(wsm->getRecipientAddressString(), "BROADCAST") == 0){
      //  findHost()->bubble("Received BroadcastMessage");
        recordOnFileMessagesBroadcast(wsm);
    //}
    }

//    findHost()->getDisplayString().updateWith("r=16,green");
//
//    annotations->scheduleErase(1, annotations->drawLine(wsm->getSenderPos(), mobi->getCurrentPosition(), "blue"));
//
//    if (!sentMessage) sendMessage(wsm->getWsmData());
}

void vehDist_rsu::sendMessage(std::string blockedRoadId) {
    sentMessage = true;
    t_channel channel = dataOnSch ? type_SCH : type_CCH;
    WaveShortMessage* wsm = prepareWSM("data", dataLengthBits, channel, dataPriority, -1,2);
    wsm->setWsmData(blockedRoadId.c_str());
    sendWSM(wsm);
}
void vehDist_rsu::sendWSM(WaveShortMessage* wsm) {
    sendDelayedDown(wsm,individualOffset);
}

void vehDist_rsu::recordOnFileMessages(WaveShortMessage* wsm){
        //Open file for just apeend
        myfile.open (fileMessagesName, std::ios_base::app);

        fieldsToSave(wsm);
        myfile.close();
}

void vehDist_rsu::recordOnFileMessagesBroadcast(WaveShortMessage* wsm){
        //Open file for just apeend
        myfile.open ("results/RSUBroadcastMessages.txt", std::ios_base::app);

        fieldsToSave(wsm);
        myfile.close();
}

void vehDist_rsu::fieldsToSave(WaveShortMessage* wsm){
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

WaveShortMessage* vehDist_rsu::prepareBeaconWSM(std::string name, int lengthBits, t_channel channel, int priority, unsigned int rcvId, int serial) {
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

    cout << "rsu create a beacon: " << findHost()->getFullName() << endl;
    wsm->setSenderAddressString(findHost()->getFullName());
    wsm->setRecipientAddressString("BROADCAST");

    //beacon don't need
    //wsm->setSource(source);
    //wsm->setTarget(target);

    wsm->setSenderPos(curPosition);
    wsm->setSerial(serial);

        if (name == "beacon") {
        DBG << "Creating Beacon with Priority " << priority << " at Applayer at " << wsm->getTimestamp() << std::endl;
    } else if (name == "data") {
        DBG << "Creating Data with Priority " << priority << " at Applayer at " << wsm->getTimestamp() << std::endl;
    }
    return wsm;
}

void vehDist_rsu::handleSelfMsg(cMessage* msg) {
    switch (msg->getKind()) {
        case SEND_BEACON_EVT: {
            //sendWSM(prepareWSM("beacon", beaconLengthBits, type_CCH, beaconPriority, 0, -1));
            sendWSM(prepareBeaconWSM("beacon", beaconLengthBits, type_CCH, beaconPriority, 0, -1));
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
