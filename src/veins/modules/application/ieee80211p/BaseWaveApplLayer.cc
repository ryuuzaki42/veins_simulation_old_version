//
// Copyright (C) 2011 David Eckhoff <eckhoff@cs.fau.de>
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

#include "veins/modules/application/ieee80211p/BaseWaveApplLayer.h"

// Added by Minicurso_UFPI
#include "veins/modules/mobility/traci/TraCIMobility.h"
using Veins::TraCIMobilityAccess;
//

const simsignalwrap_t BaseWaveApplLayer::mobilityStateChangedSignal = simsignalwrap_t(MIXIM_SIGNAL_MOBILITY_CHANGE_NAME);

void BaseWaveApplLayer::initialize_default_veins_TraCI(int stage) {
    BaseApplLayer::initialize(stage);

    if (stage==0) {
        myMac = FindModule<WaveAppToMac1609_4Interface*>::findSubModule(getParentModule());
        assert(myMac);

        myId = getParentModule()->getIndex();

        headerLength = par("headerLength").longValue();
        double maxOffset = par("maxOffset").doubleValue();
        sendBeacons = par("sendBeacons").boolValue();
        beaconLengthBits = par("beaconLengthBits").longValue();
        beaconPriority = par("beaconPriority").longValue();

        sendData = par("sendData").boolValue();
        dataLengthBits = par("dataLengthBits").longValue();
        dataOnSch = par("dataOnSch").boolValue();
        dataPriority = par("dataPriority").longValue();

        sendBeaconEvt = new cMessage("beacon evt", SEND_BEACON_EVT);

        // Simulate asynchronous channel access
        double offSet = dblrand() * (par("beaconInterval").doubleValue()/2);
        offSet = offSet + floor(offSet/0.050)*0.050;
        individualOffset = dblrand() * maxOffset;
        //cout << findHost()->getFullName() << " Beacon offSet: " << offSet << endl; // Between 0 and 1

        findHost()->subscribe(mobilityStateChangedSignal, this);

        if (sendBeacons) {
            scheduleAt(simTime() + offSet, sendBeaconEvt);
        }
    }
}

//######################################### vehDist - begin #####################################################################
void BaseWaveApplLayer::saveMessagesOnFile(WaveShortMessage* wsm, string fileName) {
    myfile.open(fileName, std::ios_base::app); // Open file for just append

    //Send "strings" to be saved on the file
    myfile << "BeaconMessage from " << wsm->getSenderAddressTemporary() << " at " << simTime();
    myfile << " to " << wsm->getRecipientAddressTemporary() << endl;
    myfile << "wsm->getGlobalMessageIdentificaton(): " << wsm->getGlobalMessageIdentificaton() << endl;
    myfile << "wsm->getName(): " << wsm->getName() << endl;
    myfile << "wsm->getWsmVersion(): " << wsm->getWsmVersion() << endl;
    myfile << "wsm->getPriority(): " << wsm->getPriority() << endl;
    myfile << "wsm->getSerial(): " << wsm->getSerial() << endl;
    myfile << "wsm->getSecurityType(): " << wsm->getSecurityType() << endl;
    myfile << "wsm->getDataRate(): " << wsm->getDataRate() << endl;
    myfile << "wsm->getBitLength(): " << wsm->getBitLength() << endl;
    myfile << "wsm->getChannelNumber(): " << wsm->getChannelNumber() << endl;
    myfile << "wsm->getPsid(): " << wsm->getPsid() << endl;
    myfile << "wsm->getPsc(): " << wsm->getPsc() << endl;
    myfile << "wsm->getHeading(): " << wsm->getHeading() << endl;
    myfile << "wsm->getCategory(): " << wsm->getCategory() << endl;
    myfile << "wsm->getRoadId(): " << wsm->getRoadId() << endl;
    myfile << "wsm->getSenderSpeed(): " << wsm->getSenderSpeed() << endl;
    myfile << "wsm->getSenderAddressTemporary(): " << wsm->getSenderAddressTemporary() << endl;
    myfile << "wsm->getRecipientAddressTemporary(): " << wsm->getRecipientAddressTemporary() << endl;
    myfile << "wsm->getSource(): " << wsm->getSource() << endl;
    myfile << "wsm->getTarget(): " << wsm->getTarget() << endl;
    myfile << "findHost()->getFullName(): " << findHost()->getFullName() << endl;
    myfile << "wsm->getTargetPos(): " << wsm->getTargetPos() << endl;
    myfile << "wsm->getHopCount(): " << wsm->getHopCount() << endl;
    myfile << "wsm->getSenderPos(): " << wsm->getSenderPos() << endl;
    myfile << "wsm->getWsmData(): " << wsm->getWsmData() << endl;
    myfile << "wsm->getTimestamp(): " << wsm->getTimestamp() << endl;
    myfile << "Time to generate and received: " << (simTime() - wsm->getTimestamp()) << endl << endl;

    myfile.close();
}

void BaseWaveApplLayer::openFileAndClose(string fileName, bool justForAppend, double ttlBeaconMessage, unsigned short int countGenerateBeaconMessage) {
    if (justForAppend) {
        myfile.open(fileName, std::ios_base::app);
    } else {
        myfile.open(fileName);
    }

    printHeaderfileExecution(ttlBeaconMessage, countGenerateBeaconMessage);
    myfile.close();
}

void BaseWaveApplLayer::printHeaderfileExecution(double ttlBeaconMessage, unsigned short int countGenerateBeaconMessage) {
    if (repeatNumber != 0) {
        myfile << endl;
    }

    string expSendbyDSCRText;
    if (expSendbyDSCR < 10) {
        expSendbyDSCRText = "000";
    } else if (expSendbyDSCR < 100) {
        expSendbyDSCRText = "00";
    } else if (expSendbyDSCR < 1000) {
        expSendbyDSCRText = "0";
    }
    expSendbyDSCRText += to_string(expSendbyDSCR);

    myfile << "Exp: " << expNumber << " expSendbyDSCR: " << expSendbyDSCRText.c_str() << " ######################################";
    myfile << "##########################################################################" << endl;
    myfile << "Exp: " << expNumber << " expSendbyDSCR: " << expSendbyDSCRText.c_str() << " ### ExpNumber: " << expNumber << " RepeatNumber: " << repeatNumber;
    myfile << " ttlBeaconMessage: " << ttlBeaconMessage << " countGenerateBeaconMessage: " << countGenerateBeaconMessage << endl << endl;
}

void BaseWaveApplLayer::generalInitializeVariables_executionByExpNumberVehDist() {
    source = findHost()->getFullName();
    beaconMessageHopLimit = par("beaconMessageHopLimit").longValue();
    string seedNumber = ev.getConfig()->getConfigValue("seed-set");
    repeatNumber = atoi(seedNumber.c_str()); // Number of execution (${repetition})
    expSendbyDSCR = par("expSendbyDSCR").longValue();

    expNumber = par("expNumber").longValue();
    if ((expNumber == 1) || (expNumber == 5)) {
        ttlBeaconMessage = par("ttlBeaconMessage_one").doubleValue();
        countGenerateBeaconMessage = par("countGenerateBeaconMessage_one").longValue();
    } else if ((expNumber == 2) || (expNumber == 6)) {
        ttlBeaconMessage = par("ttlBeaconMessage_one").doubleValue();
        countGenerateBeaconMessage = par("countGenerateBeaconMessage_two").longValue();
    } else if ((expNumber == 3) || (expNumber == 7)) {
        ttlBeaconMessage = par("ttlBeaconMessage_two").doubleValue();
        countGenerateBeaconMessage = par("countGenerateBeaconMessage_one").longValue();
    } else if ((expNumber == 4) || (expNumber == 8)) {
        ttlBeaconMessage = par("ttlBeaconMessage_two").doubleValue();
        countGenerateBeaconMessage = par("countGenerateBeaconMessage_two").longValue();
    } else {
        cout << "Error: Number of experiment not configured. Go to BaseWaveApplLayer.cc line 155" << endl;
        exit(33);
    }
}

string BaseWaveApplLayer::getFolderResultVehDist(unsigned short int expSendbyDSCR) {
    string expSendbyDSCRText, resultFolderPart;
    switch (expSendbyDSCR) {
        case 1:
            expSendbyDSCRText = "0001_chosenByDistance";
            break;
        case 12:
            expSendbyDSCRText = "0012_chosenByDistance_Speed";
            break;
        case 13:
            expSendbyDSCRText = "0013_chosenByDistance_Category";
            break;
        case 14:
            expSendbyDSCRText = "0014_chosenByDistance_RateTimeToSend";
            break;
        case 123:
            expSendbyDSCRText = "0123_chosenByDistance_Speed_Category";
            break;
        case 1234:
            expSendbyDSCRText = "1234_chosenByDistance_Speed_Category_RateTimeToSend";
            break;
        default:
            cout << "Error, expSendbyDSCR: " << expSendbyDSCR << " not defined, class in BaseWaveApplLayer.cc line 183";
            exit(1);
    }

    unsigned short int expPartOneOrTwo = par("expPart_one_or_two");
    resultFolderPart = "results/vehDist_resultsEnd_" + to_string(expPartOneOrTwo) + "/" + expSendbyDSCRText + "/";
    resultFolderPart += "E" + to_string(expNumber) + "_" + to_string((static_cast<int>(ttlBeaconMessage))) + "_";
    resultFolderPart += to_string(countGenerateBeaconMessage) +"/";

    return resultFolderPart;
}

void BaseWaveApplLayer::restartFilesResultRSU(string folderResult) {
    fileMessagesBroadcast = fileMessagesUnicast = fileMessagesCount = folderResult + source + "_";

    fileMessagesUnicast += "Messages_Received.r";
    fileMessagesBroadcast += "Broadcast_Messages.r";
    fileMessagesCount += "Count_Messages_Received.r";

    //fileMessagesDrop and fileMessagesGenerated // Not used yet to RSU

    bool justAppend;
    if (myId == 0) {
        if (expNumber <= 4) { // Set the maxSpeed to 15 m/s in the expNumber 1 to 4
            string comand = "sed -i 's/maxSpeed=.* color/maxSpeed=\"15\" color/g' vehDist.rou.xml";
            system(comand.c_str());
            cout << endl << "Change the speed to 15 m/s, command: " << comand << endl;
        } else if (expNumber >= 5) { // Set the maxSpeed to 25 m/s in the expNumber 5 to 8
            string comand = "sed -i 's/maxSpeed=.* color/maxSpeed=\"25\" color/g' vehDist.rou.xml";
            system(comand.c_str());
            cout << endl << "Change the speed to 25 m/s, command: " << comand << endl;
        }

        string commandCreateFolder = "mkdir -p " + folderResult + " > /dev/null";
        cout << endl << "Created the folder, command: \"" << commandCreateFolder << "\"" << endl;
        cout << "repeatNumber: " << repeatNumber << endl;
        system(commandCreateFolder.c_str()); // Try create a folder to save the results

        if (repeatNumber == 0) {
            justAppend = false; // Open a new file (blank)
        } else {
            justAppend = true;
        }
    } else { // repeatNumber != 0 just append
        justAppend = true;
    }

    openFileAndClose(fileMessagesCount, justAppend, ttlBeaconMessage, countGenerateBeaconMessage);
    openFileAndClose(fileMessagesUnicast, justAppend, ttlBeaconMessage, countGenerateBeaconMessage);
    openFileAndClose(fileMessagesBroadcast, justAppend, ttlBeaconMessage, countGenerateBeaconMessage);
}

void BaseWaveApplLayer::printCountMessagesReceivedRSU() {
    myfile.open (fileMessagesCount, std::ios_base::app);

    if (!messagesReceived.empty()) {
        myfile << "messagesReceived from " << source << endl;

        SimTime avgGeneralTimeMessageReceived;
        unsigned short int countP, countT;
        double avgGeneralHopsMessage, avgGeneralCopyMessageReceived;

        avgGeneralTimeMessageReceived = 0;
        countP = countT = 0;
        avgGeneralHopsMessage = avgGeneralCopyMessageReceived = 0;
        map <string, struct messages>::iterator itMessagesReceived;
        for (itMessagesReceived = messagesReceived.begin(); itMessagesReceived != messagesReceived.end(); itMessagesReceived++) {
            myfile << endl << "## Message ID: " << itMessagesReceived->first << endl;
            myfile << "Count received: " << itMessagesReceived->second.copyMessage << endl;
            myfile << "First received source: " << itMessagesReceived->second.firstSource << endl;
            avgGeneralCopyMessageReceived += itMessagesReceived->second.copyMessage;

            myfile << "wsmData: " << itMessagesReceived->second.wsmData << endl;
            myfile << "Hops: " << itMessagesReceived->second.hops << endl;
            myfile << "Sum hops: " << itMessagesReceived->second.sumHops << endl;
            avgGeneralHopsMessage += itMessagesReceived->second.sumHops;

            myfile << "Average hops: " << (itMessagesReceived->second.sumHops/itMessagesReceived->second.copyMessage) << endl;
            myfile << "Max hop: " << itMessagesReceived->second.maxHop << endl;
            myfile << "Min hop: " << itMessagesReceived->second.minHop << endl;

            myfile << "Times: " << itMessagesReceived->second.times << endl;
            myfile << "Sum times: " << itMessagesReceived->second.sumTimeRecived << endl;
            avgGeneralTimeMessageReceived += itMessagesReceived->second.sumTimeRecived;
            myfile << "Average time to received: " << (itMessagesReceived->second.sumTimeRecived/itMessagesReceived->second.copyMessage) << endl;

            myfile << "Category T count: " << itMessagesReceived->second.countT << endl;
            countT += itMessagesReceived->second.countT;
            myfile << "Category P count: " << itMessagesReceived->second.countP << endl;
            countP += itMessagesReceived->second.countP;
        }

        unsigned short int messageCountHopZero = 0;
        string messageHopCountZero, messageHopCountDifferentZero;
        messageHopCountZero = messageHopCountDifferentZero = "";
        for (itMessagesReceived = messagesReceived.begin(); itMessagesReceived != messagesReceived.end(); itMessagesReceived++) {
            if (itMessagesReceived->second.sumHops == 0) {
                messageHopCountZero += itMessagesReceived->first + ", ";
                messageCountHopZero++;
            } else {
                messageHopCountDifferentZero += itMessagesReceived->first + ", ";
            }
        }

        myfile << endl << "Messages received with hop count equal to zero:" << endl;
        myfile << messageHopCountZero << endl;

        myfile << endl << "Messages received with hop count different of zero:" << endl;
        myfile << messageHopCountDifferentZero << endl;

        avgGeneralHopsMessage /= messagesReceived.size();
        avgGeneralTimeMessageReceived /= avgGeneralCopyMessageReceived;

        myfile << endl << endl << "Exp: " << expNumber << " ### Count messages received: " << messagesReceived.size() << endl;
        myfile << "Exp: " << expNumber << " ### Count messages with hop count equal of zero received: " << messageCountHopZero << endl;
        myfile << "Exp: " << expNumber << " ### Count messages with hop count different of zero Received: " << (messagesReceived.size() - messageCountHopZero) << endl;
        myfile << "Exp: " << expNumber << " ### Average time to receive: " << avgGeneralTimeMessageReceived << endl;
        myfile << "Exp: " << expNumber << " ### Count copy message received: " << avgGeneralCopyMessageReceived << endl;
        avgGeneralCopyMessageReceived /= messagesReceived.size();
        myfile << "Exp: " << expNumber << " ### Average copy received: " << avgGeneralCopyMessageReceived << endl;
        myfile << "Exp: " << expNumber << " ### Average hops to received: " << avgGeneralHopsMessage << endl;
        myfile << "Exp: " << expNumber << " ### Hops by category T general: " << countT << endl;
        myfile << "Exp: " << expNumber << " ### Hops by category P general: " << countP << endl;
    } else {
        myfile << "messagesReceived from " << source << " is empty" << endl;
        myfile << endl << "Exp: " << expNumber << " ### Count messages received: " << 0 << endl;
    }
    myfile.close();
}

void BaseWaveApplLayer::messagesReceivedMeasuringRSU(WaveShortMessage* wsm) {
    string wsmData = wsm->getWsmData();
    simtime_t timeToArrived = (simTime() - wsm->getTimestamp());
    unsigned short int countHops = (beaconMessageHopLimit - wsm->getHopCount());
    map <string, struct messages>::iterator itMessagesReceived = messagesReceived.find(wsm->getGlobalMessageIdentificaton());

    if (itMessagesReceived != messagesReceived.end()) {
        itMessagesReceived->second.copyMessage++;

        itMessagesReceived->second.hops += ", " + to_string(countHops);
        itMessagesReceived->second.sumHops += countHops;
        if (countHops > itMessagesReceived->second.maxHop) {
            itMessagesReceived->second.maxHop = countHops;
        }
        if (countHops < itMessagesReceived->second.minHop) {
            itMessagesReceived->second.minHop = countHops;
        }

        itMessagesReceived->second.sumTimeRecived += timeToArrived;
        itMessagesReceived->second.times += ", " + timeToArrived.str();

        if (wsmData.size() > 42) { // WSMData generated by car[x] and carry by [ T,
            itMessagesReceived->second.wsmData += " & " + wsmData.substr(42); // To check
        }

        // Be aware, don't use the category identification as a value insert in the wsmData in the begin
        itMessagesReceived->second.countT += count(wsmData.begin(), wsmData.end(), 'T');
        itMessagesReceived->second.countP += count(wsmData.begin(), wsmData.end(), 'P');
    } else {
        struct messages msg;
        msg.firstSource = wsm->getSource();
        msg.copyMessage = 1;
        msg.wsmData = wsmData;
        msg.hops = to_string(countHops);
        msg.maxHop = msg.minHop = msg.sumHops = countHops;

        msg.sumTimeRecived = timeToArrived;
        msg.times = timeToArrived.str();

        // Be aware, don't use the category identification as a value insert in the wsmData in the begin
        msg.countT = count(wsmData.begin(), wsmData.end(), 'T');
        msg.countP = count(wsmData.begin(), wsmData.end(), 'P');

        messagesReceived.insert(make_pair(wsm->getGlobalMessageIdentificaton(), msg));
    }
}

void BaseWaveApplLayer::toFinishRSU() {
    printCountMessagesReceivedRSU();

    if (myId == 0) {
        string comand = "sed -i 's/maxSpeed=.* color/maxSpeed=\"15\" color/g' vehDist.rou.xml";
        system(comand.c_str()); // Set the maxSpeed back to default: 15 m/s
        cout << endl << "Setting speed back to default (15 m/s), command: " << comand << endl;
    }
}
//######################################### vehDist - end #######################################################################
void BaseWaveApplLayer::initialize_minicurso_UFPI_TraCI(int stage) {
    BaseApplLayer::initialize(stage);

    if (stage==0) {
        myMac = FindModule<WaveAppToMac1609_4Interface*>::findSubModule(getParentModule());
        assert(myMac);

        myId = getParentModule()->getIndex();

        headerLength = par("headerLength").longValue();
        double maxOffset = par("maxOffset").doubleValue();
        sendBeacons = par("sendBeacons").boolValue();
        beaconLengthBits = par("beaconLengthBits").longValue();
        beaconPriority = par("beaconPriority").longValue();

        sendData = par("sendData").boolValue();
        dataLengthBits = par("dataLengthBits").longValue();
        dataOnSch = par("dataOnSch").boolValue();
        dataPriority = par("dataPriority").longValue();

        //sendBeaconEvt = new cMessage("beacon evt", SEND_BEACON_EVT);
        sendBeaconEvt = new cMessage("beacon evt", SEND_BEACON_EVT_minicurso);

        // Simulate asynchronous channel access
        double offSet = dblrand() * (par("beaconInterval").doubleValue()/2);
        offSet = offSet + floor(offSet/0.050)*0.050;
        individualOffset = dblrand() * maxOffset;

        findHost()->subscribe(mobilityStateChangedSignal, this);

        if (sendBeacons) {
            scheduleAt(simTime() + offSet, sendBeaconEvt); // Parte modificada para o osdp e para o service_discovery
        }
    }
}

//WaveShortMessage*  BaseWaveApplLayer::prepareWSM(string name, int lengthBits, t_channel channel, int priority, int rcvId, int serial) {
WaveShortMessage*  BaseWaveApplLayer::prepareWSM(string name, int lengthBits, t_channel channel, int priority, unsigned int rcvId, int serial) {
    WaveShortMessage* wsm = new WaveShortMessage(name.c_str());
    wsm->addBitLength(headerLength);
    wsm->addBitLength(lengthBits);

    switch (channel) {
        case type_SCH: // Will be rewritten at Mac1609_4 to actual Service Channel. This is just so no controlInfo is needed
            wsm->setChannelNumber(Channels::SCH1);
            break;
        case type_CCH:
            wsm->setChannelNumber(Channels::CCH);
            break;
    }
    wsm->setPsid(0);
    wsm->setPriority(priority);
    wsm->setWsmVersion(1);
    wsm->setTimestamp(simTime());
    wsm->setSenderAddress(myId);
    wsm->setRecipientAddress(rcvId);
    wsm->setSenderPos(curPosition);
    wsm->setSerial(serial);

    if (name == "beacon_minicurso") { // Change Minicurso_UFPI
        wsm->setRoadId(TraCIMobilityAccess().get(getParentModule())->getRoadId().c_str());
        wsm->setSenderSpeed(TraCIMobilityAccess().get(getParentModule())->getSpeed());
        DBG << "Creating Beacon with Priority " << priority << " at Applayer at " << wsm->getTimestamp() << endl;
    } else if (name == "beacon") {
        DBG << "Creating Beacon with Priority " << priority << " at Applayer at " << wsm->getTimestamp() << endl;
    } else if (name == "data") {
        DBG << "Creating Data with Priority " << priority << " at Applayer at " << wsm->getTimestamp() << endl;
    }
    return wsm;
}

//############################################ Epidemic - begin ########################################################################
void BaseWaveApplLayer::initialize_epidemic(int stage) {
    BaseApplLayer::initialize(stage);

    if (stage==0) {
        myMac = FindModule<WaveAppToMac1609_4Interface*>::findSubModule(getParentModule());
        assert(myMac);

        myId = getParentModule()->getIndex();

        headerLength = par("headerLength").longValue();
        double maxOffset = par("maxOffset").doubleValue();
        sendBeacons = par("sendBeacons").boolValue();
        beaconLengthBits = par("beaconLengthBits").longValue();
        beaconPriority = par("beaconPriority").longValue();

        sendData = par("sendData").boolValue();
        dataLengthBits = par("dataLengthBits").longValue();
        dataOnSch = par("dataOnSch").boolValue();
        dataPriority = par("dataPriority").longValue();

        // Define the minimum slide window length among contacts to send new version of summary vector
        sendSummaryVectorInterval = par("sendSummaryVectorInterval").longValue();
        // Define the maximum buffer size (in number of messages) that a node is willing to allocate for epidemic messages.
        maximumEpidemicBufferSize = par("maximumEpidemicBufferSize").longValue();
        // Define the maximum number of hopes that a message can be forward before reach the target
        beaconMessageHopLimit = par("beaconMessageHopLimit").longValue();

        source = findHost()->getFullName();
        string seedNumber = ev.getConfig()->getConfigValue("seed-set");
        repeatNumber = atoi(seedNumber.c_str()); // Number of execution (${repetition})

        //sendBeaconEvt = new cMessage("beacon evt", SEND_BEACON_EVT);
        sendBeaconEvt = new cMessage("beacon evt", SEND_BEACON_EVT_epidemic);

        // Simulate asynchronous channel access
        double offSet = dblrand() * (par("beaconInterval").doubleValue()/2);
        offSet = offSet + floor(offSet/0.050)*0.050;
        individualOffset = dblrand() * maxOffset;

        findHost()->subscribe(mobilityStateChangedSignal, this);

        if (sendBeacons) {
            scheduleAt(simTime() + offSet, sendBeaconEvt);
        }
    }
}

WaveShortMessage* BaseWaveApplLayer::prepareWSM_epidemic(std::string name, int lengthBits, t_channel channel, int priority, unsigned int rcvId, int serial) {
    WaveShortMessage* wsm = new WaveShortMessage(name.c_str());
    wsm->addBitLength(headerLength);
    wsm->addBitLength(lengthBits);
    switch (channel) {
        case type_SCH: // Will be rewritten at Mac1609_4 to actual Service Channel. This is just so no controlInfo is needed
            wsm->setChannelNumber(Channels::SCH1);
            break;
        case type_CCH:
            wsm->setChannelNumber(Channels::CCH);
            break;
    }
    wsm->setPsid(0);
    wsm->setPriority(priority);
    wsm->setWsmVersion(1);
    wsm->setTimestamp(simTime());

    wsm->setSenderAddress(MACToInteger());
    wsm->setRecipientAddress(rcvId);

    wsm->setSource(source.c_str());
    wsm->setTarget(target.c_str());

    wsm->setSenderPos(curPosition);
    wsm->setSerial(serial);

    if (name == "beacon") {
        DBG << "Creating Beacon with Priority " << priority << " at Applayer at " << wsm->getTimestamp() << endl;
    }else if (name == "data") {
        DBG << "Creating Data with Priority " << priority << " at Applayer at " << wsm->getTimestamp() << endl;
    }
    return wsm;
}

unsigned int BaseWaveApplLayer::MACToInteger(){
    unsigned int macInt;
    stringstream ss;
    ss << std::hex << myMac;
    ss >> macInt;
    return macInt;
}

void BaseWaveApplLayer::printWaveShortMessageEpidemic(WaveShortMessage* wsm) {
    cout << endl << findHost()->getFullName() << " print of wsm message at "<< simTime() << endl;
    cout << "wsm->getName(): " << wsm->getName() << endl;
    cout << "wsm->getBitLength(): " << wsm->getBitLength() << endl;
    cout << "wsm->getChannelNumber(): " << wsm->getChannelNumber() << endl;
    cout << "wsm->getPsid(): " << wsm->getPsid() << endl;
    cout << "wsm->getPriority(): " << wsm->getPriority() << endl;
    cout << "wsm->getWsmVersion(): " << wsm->getWsmVersion() << endl;
    cout << "wsm->getTimestamp(): " << wsm->getTimestamp() << endl;
    cout << "wsm->getSource(): " << wsm->getSource() << endl;
    cout << "wsm->getTarget(): " << wsm->getTarget() << endl;
    cout << "wsm->getSenderAddress(): " << wsm->getSenderAddress() << endl;
    cout << "wsm->getRecipientAddress(): " << wsm->getRecipientAddress() << endl;
    cout << "wsm->getSenderPos(): " << wsm->getSenderPos() << endl;
    cout << "wsm->getSerial(): " << wsm->getSerial() << endl;
    cout << "wsm->getSummaryVector(): " << wsm->getSummaryVector() << endl;
    cout << "wsm->getRequestMessages(): " << wsm->getRequestMessages() << endl;
    cout << "wsm->getWsmData(): " << wsm->getWsmData() << endl;
    cout << "wsm.getGlobalMessageIdentificaton(): " << wsm->getGlobalMessageIdentificaton() << endl;
    cout << "wsm.getHopCount(): " << wsm->getHopCount() << endl << endl;
}

void BaseWaveApplLayer::printQueueFIFO(queue <string> qFIFO) {
    unsigned short int i = 0;
    while (!qFIFO.empty()) {
        cout << source << " - queueFIFO Element " << ++i << ": " << qFIFO.front() << endl;
        qFIFO.pop();
    }
}

//Method used to initiate the anti-entropy session sending the epidemicLocalSummaryVector
void BaseWaveApplLayer::sendLocalSummaryVector(unsigned int newRecipientAddress) {
    string idMessage;
    unsigned short int countMessage = epidemicLocalMessageBuffer.size();
    unordered_map <string , WaveShortMessage>::iterator itMsg = epidemicLocalMessageBuffer.begin();
    //printEpidemicLocalMessageBuffer();
    while (countMessage > 0) {
        if ((itMsg->second.getTimestamp() + ttlBeaconMessage) < simTime()) {
            idMessage = itMsg->second.getGlobalMessageIdentificaton();

            if (countMessage == 1) {
                countMessage = 0;
            } else {
                countMessage--;
                itMsg++;
            }

            epidemicLocalMessageBuffer.erase(idMessage);
        } else {
            countMessage--;
            itMsg++;
        }
    }

    t_channel channel = dataOnSch ? type_SCH : type_CCH;
    WaveShortMessage* wsm = prepareWSM_epidemic("data", dataLengthBits, channel, dataPriority, newRecipientAddress, 2);

    wsm->setSummaryVector(true);
    wsm->setRequestMessages(false);

    wsm->setWsmData(getLocalSummaryVectorData().c_str()); // Put the summary vector here, on data wsm field

    sendWSM(wsm); // Sending the summary vector
}

// Method used to convert the unordered_map epidemicLocalSummaryVectorData in a string
string BaseWaveApplLayer::getLocalSummaryVectorData() {
    ostringstream ss;
    for (auto& x: epidemicLocalSummaryVector) {
        ss << x.first << "|" << x.second << "|";
    }

    string s = ss.str();
    s = s.substr(0, (s.length() - 1));

    //cout << "EpidemicLocalSummaryVector from " << source << "(" << MACToInteger() << "): " << s << endl;
    return s;
}

string BaseWaveApplLayer::getEpidemicRequestMessageVectorData() {
    ostringstream ss;
    // Adding the requester name in order to identify if the requester is also the target of the messages with hopCount == 1
    // In this case, hopCount == 1, the messages can be sent to the target. Otherwise, the message will not be spread
    ss << source << "|";
    for (auto& x: epidemicRequestMessageVector) {
        ss << x.first << "|" << x.second << "|";
    }

    string s = ss.str();
    s = s.substr(0, (s.length() - 1));

    //cout << "String format of EpidemicRequestMessageVector from " << source << ": " << s << endl;
    return s;
}

void BaseWaveApplLayer::printNodesIRecentlySentSummaryVector() {
    if (!nodesIRecentlySentSummaryVector.empty()) {
        unsigned short int i = 0;
        cout << "NodesIRecentlySentSummaryVector from " << source << " (" << MACToInteger() << "):" << endl;

        for (auto& x: nodesIRecentlySentSummaryVector) {
            cout << ++i << " Node: " << x.first << " added at " << x.second << endl;
        }
    } else {
        cout << "NodesIRecentlySentSummaryVector from " << source << " is empty now " << endl;
    }
}

void BaseWaveApplLayer::printEpidemicLocalMessageBuffer() {
    if (!epidemicLocalMessageBuffer.empty()) {
        cout << endl << "Printing the epidemicLocalMessageBuffer from " << source << " (" << MACToInteger() <<") at :" << simTime() << endl;

        for (auto& x: epidemicLocalMessageBuffer) {
            WaveShortMessage wsmBuffered = x.second;
            cout << " GlobalID " << ": " << x.first << endl;
            cout << " Message Content: " << wsmBuffered.getWsmData() << endl;
            cout << " source: " << wsmBuffered.getSource() << endl;
            cout << " target: " << wsmBuffered.getTarget() << endl;
            cout << " Timestamp: " << wsmBuffered.getTimestamp() << endl;
            cout << " HopCount: " << wsmBuffered.getHopCount() << endl << endl;
        }
    } else {
        cout << "EpidemicLocalMessageBuffer from " << source << " is empty now " << endl;
    }
}

void BaseWaveApplLayer::printEpidemicRequestMessageVector() {
    if (!epidemicRequestMessageVector.empty()) {
        ostringstream ss;
        for (auto& x: epidemicRequestMessageVector) {
            ss << x.first << "|" << x.second << "|";
        }

        string s = ss.str();
        s = s.substr(0, (s.length() - 1));

        cout << "EpidemicRequestMessageVector from " << source << ": " << s << endl;
    } else {
        cout << "EpidemicRequestMessageVector from " << source << " is empty now " << endl;
    }
}

void BaseWaveApplLayer::printEpidemicLocalSummaryVectorData() {
    if (!epidemicLocalSummaryVector.empty()) {
        ostringstream ss;
        for (auto& x: epidemicLocalSummaryVector) {
            ss << x.first << "|" << x.second << "|";
        }

        string s = ss.str();
        s = s.substr(0, s.length() - 1);

        cout << "EpidemicLocalSummaryVector from " << source << "(" << MACToInteger() << "): " << s << endl;
    } else {
        cout << "EpidemicLocalSummaryVector from " << source << " is empty now " << endl;
    }
}

void BaseWaveApplLayer::printEpidemicRemoteSummaryVectorData() {
    if (!epidemicRemoteSummaryVector.empty()) {
        ostringstream ss;
        for (auto& x: epidemicRemoteSummaryVector) {
            ss << x.first << "|" << x.second << "|";
        }

        string s = ss.str();
        s = s.substr(0, (s.length() - 1));

        cout << "EpidemicRemoteSummaryVector from " << source << ": " << s << endl;
    } else {
        cout << "EpidemicRemoteSummaryVector from " << source << " is empty now " << endl;
    }
}

void BaseWaveApplLayer::sendEpidemicRequestMessageVector(unsigned int newRecipientAddress) {
    t_channel channel = dataOnSch ? type_SCH : type_CCH;
    WaveShortMessage* wsm = prepareWSM_epidemic("data", dataLengthBits, channel, dataPriority, newRecipientAddress, 2);

    wsm->setSummaryVector(false);
    wsm->setRequestMessages(true);

    wsm->setWsmData(getEpidemicRequestMessageVectorData().c_str()); // Put the summary vector here

    //cout << "Sending a vector of request messages from " << source << "(" << MACToInteger() << ") to " << newRecipientAddress << endl;
    sendWSM(wsm); // Sending the summary vector
}

void BaseWaveApplLayer::createEpidemicRequestMessageVector() {
    epidemicRequestMessageVector.clear(); // Clean the unordered_map requestMessage before create a new one

    for (auto& x: epidemicRemoteSummaryVector) {
        unordered_map <string, bool>::iterator got = epidemicLocalSummaryVector.find(x.first);

        //cout << source << " in createEpidemicRequestMessageVector(). x.first: " << x.first << " x.second: " << x.second << endl;
        if (got == epidemicLocalSummaryVector.end()) { // True value means that there is no entry in the epidemicLocalSummaryVector for a epidemicRemoteSummaryVector key
            string s = x.first;
            epidemicRequestMessageVector.insert(make_pair(s.c_str(), true)); // Putting the message in the EpidemicRequestMessageVector
        }/* else { // An entry in the unordered_map was found
            cout << source << " The message " << got->first << " in the epidemicRemoteSummaryVector was found in my epidemicLocalSummaryVector" << endl;
        }*/
    }
}

void BaseWaveApplLayer::createEpidemicRemoteSummaryVector(string s) {
    epidemicRemoteSummaryVector.clear();

    string delimiter = "|";
    size_t pos = 0;
    unsigned short int i = 0;
    while ((pos = s.find(delimiter)) != std::string::npos) {
        if (i%2 == 0) { // Catch just the key of the local summary vector
            epidemicRemoteSummaryVector.insert(make_pair(s.substr(0, pos), true));
        }
        s.erase(0, (pos + delimiter.length()));
        i++;
    }
    //cout << source << " createEpidemicRemoteSummaryVector: " << s << endl;
}

void BaseWaveApplLayer::sendMessagesRequested(string s, unsigned int recipientAddress) {
    cout << source << "(" << MACToInteger() << ") sending the following messages requested: " << s << " to " << recipientAddress << endl;

    t_channel channel = dataOnSch ? type_SCH : type_CCH;
    WaveShortMessage* wsm = prepareWSM_epidemic("data", dataLengthBits, channel, dataPriority, recipientAddress, 2);

    string delimiter = "|";
    size_t pos = 0;

    pos = s.find(delimiter);
    string tokenRequester = s.substr(0, pos); // Extracting who request message
    s.erase(0, (pos + delimiter.length()));

    unsigned short int i = 0;
    bool startSend = epidemicMessageSend.empty();
    //printEpidemicLocalMessageBuffer();
    while ((pos = s.find(delimiter)) != std::string::npos) {
        bool insert = false;
        if (i%2 == 0) { // Catch just the key of the local summary vector
            unordered_map <string, WaveShortMessage>::iterator got = epidemicLocalMessageBuffer.find(s.substr(0, pos));

            if (got != epidemicLocalMessageBuffer.end()) {
                insert = true;

                wsm->setSource(got->second.getSource());
                wsm->setTarget(got->second.getTarget());
                wsm->setTimestamp(got->second.getTimestamp());
                wsm->setHopCount(got->second.getHopCount() - 1);
                wsm->setGlobalMessageIdentificaton(got->second.getGlobalMessageIdentificaton()); // To check
                wsm->setWsmData(got->second.getWsmData());
            }/* else {
                // True value means that there is no entry in the epidemicLocalSummaryVector for a epidemicRemoteSummaryVector key
            }*/
        }
        s.erase(0, (pos + delimiter.length()));
        i++;

        if (insert) {
            //cout << source << "One of message that is sending as a result of a requisition vector request, wsm->setWsmData: " << message << endl;

            wsm->setSummaryVector(false);
            wsm->setRequestMessages(false);

            //printWaveShortMessageEpidemic(wsm);

            epidemicMessageSend.insert(make_pair(wsm->getGlobalMessageIdentificaton(), *wsm));
        }
    }

    if (startSend && !epidemicMessageSend.empty()) {
        sendEpidemicMessageRequestEvt = new cMessage("beacon evt", Send_EpidemicMessageRequestEvt);
        scheduleAt(simTime(), sendEpidemicMessageRequestEvt);
    }
}

void BaseWaveApplLayer::receivedOnBeacon(WaveShortMessage* wsm) {
    if (wsm->getSenderAddress() > MACToInteger()) { // Verifying if have the smaller address, with start the anti-entropy session sending out a summary vector
        unordered_map <unsigned int, simtime_t>::iterator got = nodesIRecentlySentSummaryVector.find(wsm->getSenderAddress());
        if (got == nodesIRecentlySentSummaryVector.end()) {
            //cout << source << " contact not found in nodesIRecentlySentSummaryVector. Sending my summary vector to " << wsm->getSenderAddress() << endl;
            nodesIRecentlySentSummaryVector.insert(make_pair(wsm->getSenderAddress(), simTime()));

            sendLocalSummaryVector(wsm->getSenderAddress());

            printNodesIRecentlySentSummaryVector();
        } else {
           if ((simTime() - got->second) >= sendSummaryVectorInterval) { // Send a summary vector to this node if passed the "sendSummaryVectorInterval" interval
               //cout << source << " updating the entry in the nodesIRecentlySentSummaryVector" << endl;

               got->second = simTime();
               sendLocalSummaryVector(wsm->getSenderAddress());

               printNodesIRecentlySentSummaryVector();
           }
        }
    } /*else { // My address is bigger than the Beacon sender -> Do nothing
        cout << source << "(" << MACToInteger() << ")" << " SenderAddress: " << wsm->getSenderAddress() << " My ID is bigger than the Beacon sender" << endl;
    }*/
}

void BaseWaveApplLayer::receivedOnData(WaveShortMessage* wsm) {
    if (wsm->getRecipientAddress() == MACToInteger()) { // Checking if is the recipient of this message
        if (wsm->getSummaryVector()) {
            cout << source << "(" << MACToInteger() << ") received the summary vector |> " << wsm->getWsmData() << " <| from " << wsm->getSenderAddress() << " at " << simTime() << endl;

            createEpidemicRemoteSummaryVector(wsm->getWsmData()); // Creating the remote summary vector

            printEpidemicRemoteSummaryVectorData();
            printEpidemicLocalSummaryVectorData();

            createEpidemicRequestMessageVector(); // Creating a key vector in order to request messages that I still do not have in my buffer

            printEpidemicRequestMessageVector();

            // Verifying if this is the end of second round of the anti-entropy session when the EpidemicRemoteSummaryVector and EpidemicLocalSummaryVector are equals
            if ((epidemicRequestMessageVector.empty() || (strcmp(wsm->getWsmData(), "") == 0)) && (wsm->getSenderAddress() > MACToInteger())) {
                //cout << "EpidemicRequestMessageVector from " << source << " is empty now " << endl;
                //cout << "Or strcmp(wsm->getWsmData(),\"\") == 0) " << endl;
                //cout << "And  wsm->getSenderAddress() > MACToInteger() " << endl;
            } else if (epidemicRequestMessageVector.empty()) {
                // Changing the turn of the anti-entropy session.
                // In this case not have any differences between EpidemicRemoteSummaryVector and EpidemicLocalSummaryVector, but I need to change the round of anti-entropy session
                sendLocalSummaryVector(wsm->getSenderAddress());
            } else {
                sendEpidemicRequestMessageVector(wsm->getSenderAddress()); // Sending a request vector in order to get messages that I don't have
            }
        } else {
            if (wsm->getRequestMessages()) { // Searching for elements in the epidemicLocalMessageBuffer and sending them to requester
                cout << source << " received the epidemicRequestMessageVector |> " << wsm->getWsmData() << " <| from " << wsm->getSenderAddress() << endl;
                sendMessagesRequested(wsm->getWsmData(), wsm->getSenderAddress());
            } else { // It's data content
                cout << source << "(" << MACToInteger() << ") received a message requested " << wsm->getGlobalMessageIdentificaton() << " |> " << wsm->getWsmData() << " <| from " << wsm->getSenderAddress() << endl;

                if (source.compare(wsm->getTarget()) == 0) { // Verifying if is the target of this message
                    cout << source << " received a message for him at " << simTime() << endl;
                    if (source.substr(0, 3).compare("rsu") == 0) {
                        findHost()->bubble("Received one message");
                        messagesReceivedMeasuringRSU(wsm);
                    } /* else {
                        // This message has target destination one vehicle
                    } */
                } else {
                    cout << "Before message processing" << endl;
                    printEpidemicLocalMessageBuffer();
                    cout << "Before message processing" << endl;
                    printEpidemicLocalSummaryVectorData();
                    cout << "Before message processing" << endl;
                    printQueueFIFO(queueFIFO);

                    // Verifying if there is no entry for current message received in my epidemicLocalMessageBuffer
                    unordered_map <string, WaveShortMessage>::iterator got = epidemicLocalMessageBuffer.find(wsm->getGlobalMessageIdentificaton());

                    if (got == epidemicLocalMessageBuffer.end()) { // True value means that there is no entry in the epidemicLocalMessageBuffer for the current message identification
                        if (queueFIFO.size() > maximumEpidemicBufferSize) { // The maximum buffer size was reached, so remove the first item from the queueFIFO
                            epidemicLocalMessageBuffer.erase(queueFIFO.front());
                            epidemicLocalSummaryVector.erase(queueFIFO.front());
                            queueFIFO.pop();
                        }

                        epidemicLocalMessageBuffer.insert(make_pair(wsm->getGlobalMessageIdentificaton(), *wsm));
                        epidemicLocalSummaryVector.insert(make_pair(wsm->getGlobalMessageIdentificaton(), true));

                        // FIFO strategy to set the maximum size that a node is willing to allocate epidemic messages in its buffer
                        queueFIFO.push(wsm->getGlobalMessageIdentificaton());
                    }

                    cout << "After message processing" << endl;
                    printEpidemicLocalMessageBuffer();
                    cout << "After message processing" << endl;
                    printEpidemicLocalSummaryVectorData();
                    cout << "After message processing" << endl;
                    printQueueFIFO(queueFIFO);
                }

                // Changing the turn of the anti-entropy session
                if (wsm->getSenderAddress() < MACToInteger()) { // To check, send a by time
                    bool SendOrNotLocalSummaryVector = false;
                    if (wsm->getSenderAddress() == nodesRecentlySendLocalSummaryVector) {
                        if ((simTime() - sendSummaryVectorInterval) >= lastTimeSendLocalSummaryVector) {
                            SendOrNotLocalSummaryVector = true;
                        }
                    } else {
                        SendOrNotLocalSummaryVector = true;
                    }

                    if(SendOrNotLocalSummaryVector) {
                        sendLocalSummaryVector(wsm->getSenderAddress());
                        nodesRecentlySendLocalSummaryVector = wsm->getSenderAddress();
                        lastTimeSendLocalSummaryVector = simTime();
                    }
                }
            }
        }
    }
}
//############################################ Epidemic - end ########################################################################

void BaseWaveApplLayer::receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj, cObject* details) {
    Enter_Method_Silent();
    if (signalID == mobilityStateChangedSignal) {
        handlePositionUpdate(obj);
    }
}

void BaseWaveApplLayer::handlePositionUpdate(cObject* obj) {
    ChannelMobilityPtrType const mobility = check_and_cast<ChannelMobilityPtrType>(obj);
    curPosition = mobility->getCurrentPosition();
}

void BaseWaveApplLayer::handleLowerMsg(cMessage* msg) {
    WaveShortMessage* wsm = dynamic_cast<WaveShortMessage*>(msg);
    ASSERT(wsm);

    if (std::string(wsm->getName()) == "beacon") {
        onBeacon(wsm);
    }
    else if (std::string(wsm->getName()) == "data") {
        onData(wsm);
    }
    else if (std::string(wsm->getName()) == "beacon_minicurso") {
        onBeacon(wsm);
    }
    else {
        DBG << "unknown message (" << wsm->getName() << ")  received\n";
    }
    delete(msg);
}

void BaseWaveApplLayer::handleSelfMsg(cMessage* msg) {
    switch (msg->getKind()) {
        case SEND_BEACON_EVT: {
            sendWSM(prepareWSM("beacon", beaconLengthBits, type_CCH, beaconPriority, 0, -1));
            scheduleAt(simTime() + par("beaconInterval").doubleValue(), sendBeaconEvt);
            break;
        }
        case SEND_BEACON_EVT_minicurso: {
            sendWSM(prepareWSM("beacon_minicurso", beaconLengthBits, type_CCH, beaconPriority, 0, -1));
            scheduleAt(simTime() + par("beaconInterval").doubleValue(), sendBeaconEvt);
            break;
        }
        case SEND_BEACON_EVT_epidemic: {
            sendWSM(prepareWSM_epidemic("beacon", beaconLengthBits, type_CCH, beaconPriority, -1, -1));
            scheduleAt(simTime() + par("beaconInterval").doubleValue(), sendBeaconEvt);
            break;
        }
        case Send_EpidemicMessageRequestEvt: {
            unordered_map <string, WaveShortMessage>::iterator itEpidemicMessageSend;
            itEpidemicMessageSend = epidemicMessageSend.begin();

            sendWSM(itEpidemicMessageSend->second.dup());

            epidemicMessageSend.erase(epidemicMessageSend.begin());
            if (!epidemicMessageSend.empty()) {
                scheduleAt((simTime() + 0.1), sendEpidemicMessageRequestEvt);
            }
        }
        default: {
            if (msg) {
                DBG << "APP: Error: Got Self Message of unknown kind! Name: " << msg->getName() << endl;
            }
            break;
        }
    }
}

void BaseWaveApplLayer::sendWSM(WaveShortMessage* wsm) {
    sendDelayedDown(wsm,individualOffset);
}

void BaseWaveApplLayer::finish() {
    if (sendBeaconEvt->isScheduled()) {
        cancelAndDelete(sendBeaconEvt);
    } else {
        delete sendBeaconEvt;
    }
    findHost()->unsubscribe(mobilityStateChangedSignal, this);
}

BaseWaveApplLayer::~BaseWaveApplLayer() {
}
