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

        restartFilesResult();
        //cout << " " << findHost()->getFullName() << " entered in the scenario" << endl;
    }
}

void vehDist_rsu::onBeacon(WaveShortMessage* wsm) {
    // to do?

}

void vehDist_rsu::restartFilesResult(){

    fileMessagesNameBroadcast = "results/rsuBroadcastMessages.txt";

    fileMessagesNameUnicast = "results/";
    fileMessagesNameUnicast += findHost()->getFullName();
    fileMessagesNameUnicast += "MessagesReceived.txt";

    fileMessagesCount = "results/";
    fileMessagesCount += findHost()->getFullName();
    fileMessagesCount += "CountMessagesReceived.txt";

     //fileMessagesDrop = ...

    repeatNumber = par("repeatNumber");
    if ((strcmp(findHost()->getFullName(), "rsu[0]") == 0) && (repeatNumber == 0)) {
        //create a folder results
        system("mkdir results");
        //Open a new file for the current simulation
        myfile.open (fileMessagesNameBroadcast);
        printHeaderfileExecution();
        myfile.close();

        myfile.open (fileMessagesNameUnicast);
        printHeaderfileExecution();
        myfile.close();

        myfile.open (fileMessagesCount);
        printHeaderfileExecution();
        myfile.close();
    }else if ((strcmp(findHost()->getFullName(), "rsu[0]") == 0) && (repeatNumber != 0)) {
        //Open a new file for the current simulation
        myfile.open (fileMessagesNameBroadcast, std::ios_base::app);
        printHeaderfileExecution();
        myfile.close();

        myfile.open (fileMessagesNameUnicast, std::ios_base::app);
        printHeaderfileExecution();
        myfile.close();

        myfile.open (fileMessagesCount, std::ios_base::app);
        printHeaderfileExecution();
        myfile.close();
    }
}

void vehDist_rsu::onData(WaveShortMessage* wsm){
    if (strcmp(wsm->getRecipientAddressString(), findHost()->getFullName()) == 0){
        findHost()->bubble("Received data");
        saveMessagesOnFile(wsm, fileMessagesNameUnicast);

        messagesReceivedMeasuring(wsm);
    }
    else{
        saveMessagesOnFile(wsm, fileMessagesNameBroadcast);
    }
}

void vehDist_rsu::messagesReceivedMeasuring(WaveShortMessage* wsm){
    int limitHopCount = par("hopCount").doubleValue();
    auto it = messagesReceived.find(wsm->getGlobalMessageIdentificaton());
    if (it != messagesReceived.end()){

        it->second.hops += ", ";
        it->second.hops += to_string(limitHopCount - wsm->getHopCount());
        it->second.HopsCount = it->second.HopsCount + 1;
        it->second.averageHops += (limitHopCount - wsm->getHopCount());
        if ((limitHopCount - wsm->getHopCount()) > it->second. maxHop){
            it->second.maxHop = (limitHopCount - wsm->getHopCount());
        }
        if ((limitHopCount - wsm->getHopCount()) < it->second. minHop){
             it->second.minHop = (limitHopCount - wsm->getHopCount());
         }

        it->second.timeAverage += (simTime() - wsm->getTimestamp());
        it->second.times += ", ";
        simtime_t tmpTime = (simTime() - wsm->getTimestamp());
        it->second.times += tmpTime.str();

    } else {
        struct messages m;
        m.HopsCount = 1;
        m.wsmData = wsm->getWsmData();
        m.hops = to_string(limitHopCount - wsm->getHopCount());
        m.maxHop = m.minHop = m.averageHops = limitHopCount - wsm->getHopCount();
        m.averageHops = (limitHopCount - wsm->getHopCount());
        m.timeAverage = (simTime() - wsm->getTimestamp());
        simtime_t tmpTime = (simTime() - wsm->getTimestamp());
        m.times = tmpTime.str();
        messagesReceived.insert(make_pair(wsm->getGlobalMessageIdentificaton(), m));
    }
}

void vehDist_rsu::saveMessagesOnFile(WaveShortMessage* wsm, string fileName){
        //Open file for just apeend
        myfile.open (fileName, std::ios_base::app);

        //Send "strings" to be saved on the file
        myfile << "Data from " << wsm->getSenderAddressString() << " at " << simTime();
        myfile << " to " << wsm->getRecipientAddressString() << endl;
        myfile << "wsm->getGlobalMessageIdentificaton(): " << wsm->getGlobalMessageIdentificaton() << endl;
        myfile << "wsm->getName(): " << wsm->getName() << endl;
        myfile << "wsm->getBitLength(): " << wsm->getBitLength() << endl;
        myfile << "wsm->getChannelNumber(): " << wsm->getChannelNumber() << endl;
        myfile << "wsm->getPsid(): " << wsm->getPsid() << endl;
        myfile << "wsm->getPriority(): " << wsm->getPriority() << endl;
        myfile << "wsm->getWsmVersion(): " << wsm->getWsmVersion() << endl;
        myfile << "wsm->getHeading(): " << wsm->getHeading() << endl;
        myfile << "wsm->getSenderAddressString(): " << wsm->getSenderAddressString() << endl;
        myfile << "wsm->getRecipientAddressString(): " << wsm->getRecipientAddressString() << endl;
        myfile << "wsm->getSource(): " << wsm->getSource() << endl;
        myfile << "wsm->getTarget(): " << wsm->getTarget() << endl;
        myfile << "findHost()->getFullName(): " << findHost()->getFullName() << endl;
        myfile << "wsm->getHopCount(): " << wsm->getHopCount() << endl;
        myfile << "wsm->getSenderPos(): " << wsm->getSenderPos() << endl;
        myfile << "wsm->getSerial(): " << wsm->getSerial() << endl;
        myfile << "wsm->getWsmData(): " << wsm->getWsmData() << endl;
        myfile << "wsm->getTimestamp(): " << wsm->getTimestamp() << endl;
        myfile << "Time to generate and recived: " << (simTime() - wsm->getTimestamp()) << endl;
        myfile << endl;

        myfile.close();
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
    wsm->setSerial(serial);
    wsm->setTimestamp(simTime());
    wsm->setSenderPos(curPosition);

    wsm->setSenderAddressString(findHost()->getFullName());

    //beacon don't need
    //wsm->setRecipientAddressString(); => "BROADCAST"
    //wsm->setSource(source);
    //wsm->setTarget(target);

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

void vehDist_rsu::printHeaderfileExecution(){
    myfile << "#############################################################################################";
    myfile << "#############################################################################################" << endl;
    myfile << "Execution number: " << repeatNumber << endl << endl;
}

void vehDist_rsu::printCountMessagesReceived(){
    myfile.open (fileMessagesCount, std::ios_base::app);

    if (messagesReceived.empty()) {
        myfile << "messagesReceived from " << findHost()->getFullName() << " is empty now" << endl;
    } else {
        myfile << "messagesReceived from " << findHost()->getFullName() << endl;

        map<string, struct messages>::iterator it;
        for (it = messagesReceived.begin(); it != messagesReceived.end(); it++) {
            myfile << endl;
            myfile << "Message Id: " << it->first << endl;
            myfile << "Count received: " << it->second.HopsCount << endl;
            myfile << it->second.wsmData << endl;
            myfile << "Hops: " << it->second.hops << endl;
            myfile << "Sum hops: " << it->second.averageHops << endl;
            it->second.averageHops = (it->second.averageHops/it->second.HopsCount);
            myfile << "Average Hops: " << it->second.averageHops << endl;
            myfile << "Max Hop: " << it->second.maxHop << endl;
            myfile << "Min Hop: " << it->second.minHop << endl;
            myfile << "Times: " << it->second.times << endl;
            myfile << "Sum times: " << it->second.timeAverage << endl;
            myfile << "Avegare time to received: " << (it->second.timeAverage/it->second.HopsCount) << endl;

        }
        myfile << "Count Messages Received: " << messagesReceived.size() << endl;
        myfile << endl;
    }
    myfile.close();
}

void vehDist_rsu:: finish(){
    printCountMessagesReceived();
}

void vehDist_rsu::sendWSM(WaveShortMessage* wsm) {
    sendDelayedDown(wsm,individualOffset);
}

void vehDist_rsu::sendMessage(std::string blockedRoadId) {
    sentMessage = true;
    t_channel channel = dataOnSch ? type_SCH : type_CCH;
    WaveShortMessage* wsm = prepareWSM("data2rsu", dataLengthBits, channel, dataPriority, -1,2);
    wsm->setWsmData(blockedRoadId.c_str());
    sendWSM(wsm);
}
