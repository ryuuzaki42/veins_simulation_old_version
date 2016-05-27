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

// Adicionado (Minicurso_UFPI)
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

        //simulate asynchronous channel access
        double offSet = dblrand() * (par("beaconInterval").doubleValue()/2);
        offSet = offSet + floor(offSet/0.050)*0.050;
        individualOffset = dblrand() * maxOffset;
        //cout << findHost()->getFullName() << " Beacon offSet: " << offSet << endl; // betwen 0 and 1

        findHost()->subscribe(mobilityStateChangedSignal, this);

        if (sendBeacons) {
            scheduleAt(simTime() + offSet, sendBeaconEvt);
        }
    }
}

//######################################### vehDist - begin #####################################################################

void BaseWaveApplLayer::saveMessagesOnFile(WaveShortMessage* wsm, string fileName){
    myfile.open (fileName, std::ios_base::app); //Open file for just append

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
    myfile << "Time to generate and received: " << (simTime() - wsm->getTimestamp()) << endl;
    myfile << endl;

    myfile.close();
}

void BaseWaveApplLayer::openFileAndClose(string fileName, bool justForAppend, double ttlBeaconMessage, unsigned short int countGenerateBeaconMessage ){
    if (justForAppend) {
        myfile.open(fileName, std::ios_base::app);
    } else {
        myfile.open(fileName);
    }
    printHeaderfileExecution(ttlBeaconMessage ,countGenerateBeaconMessage);
    myfile.close();
}

void BaseWaveApplLayer::printHeaderfileExecution(double ttlBeaconMessage, unsigned short int countGenerateBeaconMessage){
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

void BaseWaveApplLayer::generalInitializeVariables_executionByExpNumber(){
    source = findHost()->getFullName();
    beaconMessageHopLimit = par("beaconMessageHopLimit").longValue();
    string seedNumber = ev.getConfig()->getConfigValue("seed-set");
    repeatNumber = atoi(seedNumber.c_str()); // number of execution (${repetition})
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
        cout << "Error: Number of experiment not configured. Go to BaseWaveApplLayer.cc line 156." << endl;
        exit(1); // Comets this line for use the values below
        ttlBeaconMessage = 60; // Just for don't left garbage value in this variable
        countGenerateBeaconMessage = 0; // Will not generate any message
    }
}

string BaseWaveApplLayer::getFolderResult(unsigned short int expSendbyDSCR){
    string expSendbyDSCRText, result_folder_part;
    switch (expSendbyDSCR){
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
        cout << "Error! expSendbyDSCR: " << expSendbyDSCR << "not defined, class in BaseWaveApplLayer.cc";
        DBG << "Error! expSendbyDSCR: " << expSendbyDSCR << "not defined, class in BaseWaveApplLayer.cc";
        exit(1);
    }

    int expPart_one_or_two = par("expPart_one_or_two");
    result_folder_part = "results/vehDist_resultsEnd_" + to_string(expPart_one_or_two) + "/" + expSendbyDSCRText + "/";
    result_folder_part += "E" + to_string(expNumber) + "_" + to_string((static_cast<int>(ttlBeaconMessage))) + "_";
    result_folder_part += to_string(countGenerateBeaconMessage) +"/";

    return result_folder_part;
}

void BaseWaveApplLayer::printCountMessagesReceived() {
    myfile.open (fileMessagesCount, std::ios_base::app);

    if (!messagesReceived.empty()) {
        myfile << "messagesReceived from " << source << endl;

        SimTime avgGeneralTimeMessageReceived;
        unsigned short int countP, countT, messageCountHopZero;
        double avgGeneralHopsMessage, avgGeneralCopyMessageReceived;

        avgGeneralTimeMessageReceived = 0;
        countP = countT = messageCountHopZero = 0;
        avgGeneralHopsMessage = avgGeneralCopyMessageReceived = 0;
        map <string, struct messages>::iterator it;
        for (it = messagesReceived.begin(); it != messagesReceived.end(); it++) {
            myfile << endl << "## Message ID: " << it->first << endl;
            myfile << "Count received: " << it->second.copyMessage << endl;
            avgGeneralCopyMessageReceived += it->second.copyMessage;

            myfile << "wsmData: " << it->second.wsmData << endl;
            myfile << "Hops: " << it->second.hops << endl;
            myfile << "Sum hops: " << it->second.sumHops << endl;
            avgGeneralHopsMessage += it->second.sumHops;
            if (it->second.sumHops == 0) {
                messageCountHopZero++;
            }

            myfile << "Average hops: " << (it->second.sumHops/it->second.copyMessage) << endl;
            myfile << "Max hop: " << it->second.maxHop << endl;
            myfile << "Min hop: " << it->second.minHop << endl;

            myfile << "Times: " << it->second.times << endl;
            myfile << "Sum times: " << it->second.sumTimeRecived << endl;
            avgGeneralTimeMessageReceived += it->second.sumTimeRecived;
            myfile << "Average time to received: " << (it->second.sumTimeRecived/it->second.copyMessage) << endl;

            myfile << "Category T count: " << it->second.countT << endl;
            countT += it->second.countT;
            myfile << "Category P count: " << it->second.countP << endl;
            countP += it->second.countP;
        }

        // TODO: XX geradas, mas só (XX - 4) recebidas
        avgGeneralHopsMessage /= messagesReceived.size();
        avgGeneralTimeMessageReceived /= avgGeneralCopyMessageReceived;

        myfile << endl << "Exp: " << expNumber << " ### Count messages received: " << messagesReceived.size() << endl;
        myfile << "Exp: " << expNumber << " ### Count messages with hop equal of zero received: " << messageCountHopZero << endl;
        myfile << "Exp: " << expNumber << " ### Count messages with hop different of zero Received: " << (messagesReceived.size() - messageCountHopZero) << endl;
        myfile << "Exp: " << expNumber << " ### Average time to receive: " << avgGeneralTimeMessageReceived << endl;
        myfile << "Exp: " << expNumber << " ### Count copy message received: " << avgGeneralCopyMessageReceived << endl;
        avgGeneralCopyMessageReceived /= messagesReceived.size();
        myfile << "Exp: " << expNumber << " ### Average copy received: " << avgGeneralCopyMessageReceived << endl;
        myfile << "Exp: " << expNumber << " ### Average hops to received: " << avgGeneralHopsMessage << endl;
        myfile << "Exp: " << expNumber << " ### Hops by category T general: " << countT << endl;
        myfile << "Exp: " << expNumber << " ### Hops by category P general: " << countP << endl;
    } else {
        myfile << "messagesReceived from " << source << " is empty" << endl;
    }
    myfile.close();
}

void BaseWaveApplLayer::messagesReceivedMeasuringRSU(WaveShortMessage* wsm) {
    string wsmData = wsm->getWsmData();
    simtime_t timeToArrived = (simTime() - wsm->getTimestamp());
    unsigned short int countHops = (beaconMessageHopLimit - wsm->getHopCount());
    map <string, struct messages>::iterator it = messagesReceived.find(wsm->getGlobalMessageIdentificaton());

    if (it != messagesReceived.end()) {
        it->second.copyMessage++;

        it->second.hops += ", " + to_string(countHops);
        it->second.sumHops += countHops;
        if (countHops > it->second.maxHop) {
            it->second.maxHop = countHops;
        }
        if (countHops < it->second.minHop) {
            it->second.minHop = countHops;
        }

        it->second.sumTimeRecived += timeToArrived;
        it->second.times += ", " + timeToArrived.str();

        if (wsmData.size() > 42){ // WSMData generated by car[x] and carry by [ T,
            it->second.wsmData += " & " + wsmData.substr(42); // To check
        }
        // Be aware, don't use the category identification as a value insert in the wsmData in the begin
        it->second.countT += count(wsmData.begin(), wsmData.end(), 'T');
        it->second.countP += count(wsmData.begin(), wsmData.end(), 'P');
    } else {
        struct messages m;
        m.copyMessage = 1;
        m.wsmData = wsmData;
        m.hops = to_string(countHops);
        m.maxHop = m.minHop = m.sumHops = countHops;

        m.sumTimeRecived = timeToArrived;
        m.times = timeToArrived.str();

        // Be aware, don't use the category identification as a value insert in the wsmData in the begin
        m.countT = count(wsmData.begin(), wsmData.end(), 'T');
        m.countP = count(wsmData.begin(), wsmData.end(), 'P');

        messagesReceived.insert(make_pair(wsm->getGlobalMessageIdentificaton(), m));
    }
}

void BaseWaveApplLayer::toFinishRSU() {
    printCountMessagesReceived();

    // Set the maxSpeed back to default: 15 m/s
    string comand = "sed -i 's/maxSpeed=.* color/maxSpeed=\"15\" color/g' vehDist.rou.xml";
    system(comand.c_str());
    cout << endl << "Setting speed back to default (15 m/s), command: " << comand << endl;
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

        //simulate asynchronous channel access
        double offSet = dblrand() * (par("beaconInterval").doubleValue()/2);
        offSet = offSet + floor(offSet/0.050)*0.050;
        individualOffset = dblrand() * maxOffset;

        findHost()->subscribe(mobilityStateChangedSignal, this);

        if (sendBeacons) {
            scheduleAt(simTime() + offSet, sendBeaconEvt); //parte modificada para o osdp e para o service_discovery
        }
    }
}

//WaveShortMessage*  BaseWaveApplLayer::prepareWSM(std::string name, int lengthBits, t_channel channel, int priority, int rcvId, int serial) {
WaveShortMessage*  BaseWaveApplLayer::prepareWSM(std::string name, int lengthBits, t_channel channel, int priority, unsigned int rcvId, int serial) {
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

    if (name == "beacon_minicurso") { // Change Minicurso_UFPI
        wsm->setRoadId(TraCIMobilityAccess().get(getParentModule()) ->getRoadId().c_str());
        wsm->setSenderSpeed(TraCIMobilityAccess(). get(getParentModule())->getSpeed());
        DBG << "Creating Beacon with Priority " << priority << " at Applayer at " << wsm->getTimestamp() << std::endl;
    } else if (name == "beacon") {
        DBG << "Creating Beacon with Priority " << priority << " at Applayer at " << wsm->getTimestamp() << std::endl;
    } else if (name == "data") {
        DBG << "Creating Data with Priority " << priority << " at Applayer at " << wsm->getTimestamp() << std::endl;
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

        //define the minimum slide window length among contacts to send new version of summary vector
        sendSummaryVectorInterval = par("sendSummaryVectorInterval").longValue();
        //define the maximum buffer size (in number of messages) that a node is willing to allocate for epidemic messages.
        maximumEpidemicBufferSize = par("maximumEpidemicBufferSize").longValue();
        //define the maximum number of hopes that a message can be forward before reach the target
        beaconMessageHopLimit = par("beaconMessageHopLimit").longValue();

        source = findHost()->getFullName();
        string seedNumber = ev.getConfig()->getConfigValue("seed-set");
        repeatNumber = atoi(seedNumber.c_str()); // number of execution (${repetition})

        //sendBeaconEvt = new cMessage("beacon evt", SEND_BEACON_EVT);
        sendBeaconEvt = new cMessage("beacon evt", SEND_BEACON_EVT_epidemic);

        //simulate asynchronous channel access
        double offSet = dblrand() * (par("beaconInterval").doubleValue()/2);
        offSet = offSet + floor(offSet/0.050)*0.050;
        individualOffset = dblrand() * maxOffset;

        findHost()->subscribe(mobilityStateChangedSignal, this);

        if (sendBeacons) {
            scheduleAt(simTime() + offSet, sendBeaconEvt);
        }
    }
}

//WaveShortMessage* BaseWaveApplLayer::prepareWSM(std::string name, int lengthBits, t_channel channel, int priority, std::string rcvId, int serial) {
WaveShortMessage* BaseWaveApplLayer::prepareWSM_epidemic(std::string name, int lengthBits, t_channel channel, int priority, unsigned int rcvId, int serial) {
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

    wsm->setSenderAddress(MACToInteger());
    wsm->setRecipientAddress(rcvId);

    wsm->setSource(source.c_str());
    wsm->setTarget(target.c_str());

    wsm->setSenderPos(curPosition);
    wsm->setSerial(serial);

    if (name == "beacon") {
        DBG << "Creating Beacon with Priority " << priority << " at Applayer at " << wsm->getTimestamp() << std::endl;
    }else if (name == "data") {
        DBG << "Creating Data with Priority " << priority << " at Applayer at " << wsm->getTimestamp() << std::endl;
    }
    return wsm;
}

unsigned int BaseWaveApplLayer::MACToInteger(){
    unsigned int macInt;
    std::stringstream ss;
    ss << std::hex << myMac;
    ss >> macInt;
    return macInt;
}

void BaseWaveApplLayer::printWaveShortMessage(WaveShortMessage* wsm) {
    cout << endl << source << " print of wsm message at "<< simTime() << endl;
    cout << "wsm->getName(): " << wsm->getName() << endl;
    cout << "wsm->getBitLength(): " << wsm->getBitLength() << endl;
    cout << "wsm->getChannelNumber(): " << wsm->getChannelNumber() << endl;
    cout << "wsm->getPsid(): " << wsm->getPsid() << endl;
    cout << "wsm->getPriority(): " << wsm->getPriority() << endl;
    cout << "wsm->getWsmVersion(): " << wsm->getWsmVersion() << endl;
    cout << "wsm->getTimestamp(): " << wsm->getTimestamp() << endl;
    cout << "wsm->getSenderAddress(): " << wsm->getSenderAddress() << endl;
    cout << "wsm->getRecipientAddress(): " << wsm->getRecipientAddress() << endl;
    cout << "wsm->getSource(): " << wsm->getSource() << endl;
    cout << "wsm->getTarget(): " << wsm->getTarget() << endl;
    cout << "wsm->getSenderPos(): " << wsm->getSenderPos() << endl;
    cout << "wsm->getSerial(): " << wsm->getSerial() << endl;
    cout << "wsm->getSummaryVector(): " << wsm->getSummaryVector() << endl;
    cout << "wsm->getRequestMessages(): " << wsm->getRequestMessages() << endl;
    cout << "wsm->getWsmData(): " << wsm->getWsmData() << endl;
    cout << "wsm.getGlobalMessageIdentificaton(): " << wsm->getGlobalMessageIdentificaton() << endl;
    cout << "wsm.getHopCount(): " << wsm->getHopCount() << endl << endl;
}

void BaseWaveApplLayer::printQueueFIFO(queue <string> qFIFO) {
    int i = 0;
    while(!qFIFO.empty()) {
        cout << source << " - queueFIFO Element " << ++i << ": " << qFIFO.front() << endl;
        qFIFO.pop();
    }
}

//Method used to initiate the anti-entropy session sending the epidemicLocalSummaryVector
void BaseWaveApplLayer::sendLocalSummaryVector(unsigned int newRecipientAddress) {
    t_channel channel = dataOnSch ? type_SCH : type_CCH;
    WaveShortMessage* wsm = prepareWSM_epidemic("data", dataLengthBits, channel, dataPriority, newRecipientAddress,2);

    wsm->setSummaryVector(true);
    wsm->setRequestMessages(false);

    wsm->setWsmData(getLocalSummaryVectorData().c_str()); //Put the summary vector here, on data wsm field

    sendWSM(wsm); //Sending the summary vector
}

//Method used to convert the unordered_map epidemicLocalSummaryVectorData in a string
string BaseWaveApplLayer::getLocalSummaryVectorData() {
    ostringstream ss;
    for (auto& x: epidemicLocalSummaryVector) {
        ss << x.first << "|" << x.second << "|";
    }

    string s = ss.str();
    s = s.substr(0, s.length() - 1);

    //cout << "EpidemicLocalSummaryVector from " << source << "(" << MACToInteger() << "): " << s << endl;
    return s.c_str();
}

string BaseWaveApplLayer::getEpidemicRequestMessageVectorData() {
    ostringstream ss;
    //adding the requester name in order to identify if the requester is also the target of the messages with hopcount == 1.
    //In this case, hopcount == 1, the messages can be sent to the target. Otherwise, the message will not be spread
    ss << source << "|";
    for (auto& x: epidemicRequestMessageVector) {
        ss << x.first << "|" << x.second << "|";
    }

    string s = ss.str();
    s = s.substr(0, s.length() - 1);

    //cout << "String format of EpidemicRequestMessageVector from " << source << ": " << s << endl;
    return s.c_str();
}

void BaseWaveApplLayer::printNodesIRecentlySentSummaryVector() {
    if (!nodesIRecentlySentSummaryVector.empty()) {
        int i = 0;
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
        int i = 0;
        cout << "Printing the epidemicLocalMessageBuffer from " << source << "(" << MACToInteger() <<"):" << endl;
        for (auto& x: epidemicLocalMessageBuffer) {
            WaveShortMessage wsmBuffered = x.second;
            cout << " Key " << ++i << ": " << x.first << endl;
            cout << " Message Content: " << wsmBuffered.getWsmData() << endl;
            cout << " source: " << wsmBuffered.getSource() << endl;
            cout << " target: " << wsmBuffered.getTarget() << endl;
            cout << " Timestamp: " << wsmBuffered.getTimestamp() << endl;
            cout << " HopCount: " << wsmBuffered.getHopCount() << endl;
            cout << " GlobalID: " << wsmBuffered.getGlobalMessageIdentificaton() << endl;
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
        s = s.substr(0, s.length() - 1);

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
        s = s.substr(0, s.length() - 1);

        cout << "EpidemicRemoteSummaryVector from " << source << ": " << s << endl;
    } else {
        cout << "EpidemicRemoteSummaryVector from " << source << " is empty now " << endl;
    }
}

void BaseWaveApplLayer::sendEpidemicRequestMessageVector(unsigned int newRecipientAddress) {
    t_channel channel = dataOnSch ? type_SCH : type_CCH;
    WaveShortMessage* wsm = prepareWSM_epidemic("data", dataLengthBits, channel, dataPriority, newRecipientAddress,2);

    wsm->setSummaryVector(false);
    wsm->setRequestMessages(true);

    wsm->setWsmData(getEpidemicRequestMessageVectorData().c_str()); //Put the summary vector here

    sendWSM(wsm); //Sending the summary vector

    //cout << "Sending a vector of request messages from " << source << "(" << MACToInteger() << ") to " << newRecipientAddress << endl;
}

void BaseWaveApplLayer::createEpidemicRequestMessageVector() {
    epidemicRequestMessageVector.clear();

    for (auto& x: epidemicRemoteSummaryVector) {
        unordered_map <string, bool>::iterator got = epidemicLocalSummaryVector.find(x.first);

        //cout << "I'm in createEpidemicRequestMessageVector().  x.first: " << x.first << " x.second: " << x.second << endl;
        if (got == epidemicLocalSummaryVector.end()) { //true value means that there is no entry in the epidemicLocalSummaryVector for a epidemicRemoteSummaryVector key
            //Putting the message in the EpidemicRequestMessageVector
            string s = x.first;
            epidemicRequestMessageVector.insert(make_pair<string, bool>(s.c_str(),true));
        } else { //An entry in the unordered_map was found
            //cout << "I'm in createEpidemicRequestMessageVector(). got->first: " << got->first << " got->second: " << got->second << endl;
            //cout << "The message " << got->first << " in the epidemicRemoteSummaryVector was found in my epidemicLocalSummaryVector." << endl;
        }
    }
}

void BaseWaveApplLayer::createEpidemicRemoteSummaryVector(string s) {
    //cout << "Creating the epidemicRemoteSummaryVector in " << source << endl;
    string delimiter = "|";
    size_t pos = 0;
    string token;
    unsigned i = 0;
    epidemicRemoteSummaryVector.clear();
    while ((pos = s.find(delimiter)) != std::string::npos) {
        //token = s.substr(0, pos);
        //cout << token << endl;
        //Catch just the key of the local summary vector
        if (i%2 == 0) {
            epidemicRemoteSummaryVector.insert(make_pair<string, bool>(s.substr(0, pos),true));
        }
        s.erase(0, pos + delimiter.length());
        i++;
    }
    //cout << s << endl;
}

void BaseWaveApplLayer::sendMessagesRequested(string s, unsigned int recipientAddress) {
    cout << "I'm " << source << "(" << MACToInteger() << "). Sending the following messages requested: " << s << " to " << recipientAddress << endl;
    t_channel channel = dataOnSch ? type_SCH : type_CCH;
    WaveShortMessage* wsm = prepareWSM_epidemic("data", dataLengthBits, channel, dataPriority, recipientAddress,2);
    string delimiter = "|";
    size_t pos = 0;
    string token;
    unsigned i = 0;
    string message="";

    printEpidemicLocalMessageBuffer();

    bool startSend = epidemicMessageSend.empty();
    //Extracting who request message
    pos = s.find(delimiter);
    string tokenRequester = s.substr(0, pos);
    s.erase(0, pos + delimiter.length());

    while ((pos = s.find(delimiter)) != std::string::npos) {
        //Catch just the key of the local summary vector
        if (i%2 == 0) {
            ostringstream ss;
            //cout << "i = " << i << " - looking for the message key(s.substr(0, pos)): " << s.substr(0, pos) << endl;
            WaveShortMessage w;
            unordered_map <string, WaveShortMessage>::const_iterator got = epidemicLocalMessageBuffer.find(s.substr(0, pos));
            if (got == epidemicLocalMessageBuffer.end()) {
                //true value means that there is no entry in the epidemicLocalSummaryVector for a epidemicRemoteSummaryVector key
            } else {
                w = got->second;
                //cout << "id " << w.getGlobalMessageIdentificaton() << endl;
                wsm->setGlobalMessageIdentificaton(w.getGlobalMessageIdentificaton());
                wsm->setHopCount(w.getHopCount() - 1);
                //exit(22);
                //WaveShortMessage w = getEpidemicLocalMessageBuffer(s.substr(0, pos));
                //Verifying if I'm still able to spread the message or not. If w.getHopCount == 1 I'm able to send the message only to its target
                if (w.getHopCount() > 1) {
                    ss << s.substr(0, pos) << "|" << w.getWsmData() << "|" << w.getSource() << "|" << w.getTarget() << "|" << w.getTimestamp() << "|" << w.getHopCount() - 1 << "|";
                } else if (w.getHopCount() == 1) {
                    if ((strcmp(tokenRequester.c_str(), w.getTarget()) == 0)) {
                        ss << s.substr(0, pos) << "|" << w.getWsmData() << "|" << w.getSource() << "|" << w.getTarget() << "|" << w.getTimestamp() << "|" << w.getHopCount() - 1 << "|";
                    }
                }
                //cout << "ss.str() inside the sendMessageRequested method: " << ss.str() << endl;
                message += ss.str();
                //cout << "message inside the sendMessageRequested method. message += ss.str(): " << message << endl;
            }
        }
        //cout << "i = " << i << " - s inside the sendMessageRequested method: " << s << endl;
        s.erase(0, pos + delimiter.length());
        //cout << "i = " << i << " - s inside the sendMessageRequested method, after s.erase(0, pos + delimiter.length()): " << s << endl;
        i++;

        message = message.substr(0, message.length() - 1);
        cout << "Message that is sending as a result of a requisition vector request. wsm->setWsmData: " << message << endl;
        //exit(1);

        wsm->setWsmData(message.c_str());
        wsm->setSummaryVector(false);
        wsm->setRequestMessages(false);

        printWaveShortMessage(wsm);
        //exit(22);

        epidemicMessageSend.insert(make_pair(wsm->getGlobalMessageIdentificaton(), *wsm));
        message = "";
    }

    if (startSend) {
        sendEpidemicMessageRequestEvt = new cMessage("beacon evt", Send_EpidemicMessageRequestEvt);
        scheduleAt(simTime(), sendEpidemicMessageRequestEvt);
    }

//    message = message.substr(0, message.length() - 1);
//    cout << "Message that is sending as a result of a requisition vector request. wsm->setWsmData: " << message << endl;
//
//    wsm->setWsmData(message.c_str());
//    wsm->setSummaryVector(false);
//    wsm->setRequestMessages(false);
//
//    printWaveShortMessage(wsm);
//
//    sendWSM(wsm);
}

void BaseWaveApplLayer::receivedOnBeacon(WaveShortMessage* wsm) {
    if (wsm->getSenderAddress() > MACToInteger()) { //Verifying if have the smaller address, with start the anti-entropy session sending out a summary vector
        unordered_map <unsigned int, simtime_t>::iterator got = nodesIRecentlySentSummaryVector.find(wsm->getSenderAddress());
        if (got == nodesIRecentlySentSummaryVector.end()) {
            //cout << source << " Contact not found in nodesIRecentlySentSummaryVector. Sending my summary vector to " << wsm->getSenderAddress() << endl;
            nodesIRecentlySentSummaryVector.insert(make_pair(wsm->getSenderAddress(), simTime()));

            sendLocalSummaryVector(wsm->getSenderAddress());

            printNodesIRecentlySentSummaryVector();
        } else {
           if ((simTime() - got->second) >= sendSummaryVectorInterval) { //Send a summary vector to this node if passed the "sendSummaryVectorInterval" interval
               //cout << "I'm " << source << " and I'm updating the entry in the nodesIRecentlySentSummaryVector." << endl;

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
    //Verifying the kind of a received message: if a summary vector (true) or a epidemic buffer data message (false).
    if (wsm->getSummaryVector()) {
        //checking if the summary vector was sent to me
        if (wsm->getRecipientAddress() == MACToInteger()) {
            cout << source << "(" << MACToInteger() << ") received the summary vector |> " << wsm->getWsmData() << " <| from " << wsm->getSenderAddress() << " at: " << simTime() << endl;
            //Creating the remote summary vector with the data received in wsm->message field
            createEpidemicRemoteSummaryVector(wsm->getWsmData());
            printEpidemicRemoteSummaryVectorData();
            printEpidemicLocalSummaryVectorData();
            //Creating a key vector in order to request messages that I still do not have in my buffer
            createEpidemicRequestMessageVector();
            printEpidemicRequestMessageVector();
            //Verifying if this is the end of second round of the anti-entropy session when the EpidemicRemoteSummaryVector and EpidemicLocalSummaryVector are equals
            if ((epidemicRequestMessageVector.empty() ||(strcmp(wsm->getWsmData(),"") == 0)) && (wsm->getSenderAddress() > MACToInteger())) {
                //cout << "EpidemicRequestMessageVector from " << source << " is empty now " << endl;
                //cout << "Or strcmp(wsm->getWsmData(),\"\") == 0) " << endl;
                //cout << "And  wsm->getSenderAddress() > MACToInteger() " << endl;
            } else if (epidemicRequestMessageVector.empty()) {
                //cout << "EpidemicRequestMessageVector from " << source << " is empty now " << endl;
                //changing the turn of the anti-entropy session. In this case, I have not found any differences between EpidemicRemoteSummaryVector and EpidemicLocalSummaryVector but I need to change the round of anti-entropy session
                sendLocalSummaryVector(wsm->getSenderAddress());
            } else {
                //Sending a request vector in order to get messages that I don't have
                sendEpidemicRequestMessageVector(wsm->getSenderAddress());
            }
        }
    } else { //is a data message requisition or a data message content
        //cout << "I'm " << source << ". This is not a summary vector" << endl;
        //Verifying if this is a request message
        if (wsm->getRequestMessages()) {
            //checking if the request vector was sent to me
            if (wsm->getRecipientAddress() == MACToInteger()) {
                //Searching for elements in the epidemicLocalMessageBuffer and sending them to requester
                cout << source << " received the epidemicRequestMessageVector |> " << wsm->getWsmData() << " <| from " << wsm->getSenderAddress() << endl;
                sendMessagesRequested(wsm->getWsmData(), wsm->getSenderAddress());
            }
        } else { //is data content
            if (wsm->getRecipientAddress() == MACToInteger()) {
                //WSMData generated by car[3]|car[3]|rsu[0]|1.2
                cout << source << "(" << MACToInteger() << ") received all the message requested |> " << wsm->getWsmData() << " <| from " << wsm->getSenderAddress() << endl;
                cout << "Before message processing" << endl;
                printEpidemicLocalMessageBuffer();
                cout << "Before message processing" << endl;
                printEpidemicLocalSummaryVectorData();
                cout << "Before message processing" << endl;
                printQueueFIFO(queueFIFO);
                string delimiter = "|";
                size_t pos = 0;
                string tokenkey, tokenData, tokenSource, tokenTarget, tokenTimestamp, tokenhopcount;
                string messageReceived = wsm->getWsmData();
                cout << "messageReceived: " << messageReceived << endl;
                simtime_t st;
                //cout << "pos = messageReceived.find(delimiter): " << messageReceived.find(delimiter) << endl;
                //cout << "std::string::npos: " << std::string::npos << endl;
                while((pos = messageReceived.find(delimiter)) != std::string::npos) {

                    // leaved to not break the code
                    tokenkey = messageReceived.substr(0, pos);
                    messageReceived.erase(0, pos + delimiter.length());

                    //tokenkey = wsm->getGlobalMessageIdentificaton();

                    pos = messageReceived.find(delimiter);
                    tokenData = messageReceived.substr(0, pos);
                    messageReceived.erase(0, pos + delimiter.length());

                    pos = messageReceived.find(delimiter);
                    tokenSource = messageReceived.substr(0, pos);
                    messageReceived.erase(0, pos + delimiter.length());

                    pos = messageReceived.find(delimiter);
                    tokenTarget = messageReceived.substr(0, pos);
                    messageReceived.erase(0, pos + delimiter.length());

                    pos = messageReceived.find(delimiter);
                    tokenTimestamp = messageReceived.substr(0, pos);
                    messageReceived.erase(0, pos + delimiter.length());

                    pos = messageReceived.find(delimiter);
                    tokenhopcount = messageReceived.substr(0, pos);
                    cout << "tokenhopcount: " << tokenhopcount << endl;
                    //exit(1);
                    messageReceived.erase(0, pos + delimiter.length());

                    //cout << "I'm " << source << " and the message received is tokenData: " << tokenData << "tokenSource: " << tokenSource << "tokenTarget: " << tokenTarget << "tokenTimestamp: " << tokenTimestamp << endl;
                    WaveShortMessage w = *wsm;
                    w.setWsmData(tokenData.c_str());
                    w.setSource(tokenSource.c_str());
                    w.setTarget(tokenTarget.c_str());
                    w.setTimestamp(st.parse(tokenTimestamp.c_str()));
                    w.setHopCount(stoi(tokenhopcount));

//                     if (source.substr(0, 3).compare("rsu") == 0) {
//                         if (source.compare(w.getTarget()) == 0) {
//                             WaveShortMessage w2 = w;
//                             w2.setGlobalMessageIdentificaton(tokenkey.c_str());
//                             messagesReceivedMeasuring(&w2);
//                         }
//                     }

                    //checking if the maximum buffer size was reached
                    if (queueFIFO.size() < maximumEpidemicBufferSize) {
                        //Verifying if there is no entry for current message received in my epidemicLocalMessageBuffer
                        unordered_map <string,WaveShortMessage>::iterator got = epidemicLocalMessageBuffer.find(tokenkey);
                        if (got == epidemicLocalMessageBuffer.end()) { //true value means that there is no entry in the epidemicLocalMessageBuffer for the current message identification
                            //Putting the message in the epidemicLocalMessageBuffer
                            epidemicLocalMessageBuffer.insert(make_pair(tokenkey,w));
                            //Putting the message in the EpidemicLocalSummaryVector
                            epidemicLocalSummaryVector.insert(make_pair(tokenkey.c_str(),true));
                            //FIFO strategy to set the maximum size that a node is willing to allocate epidemic messages in its buffer
                            queueFIFO.push(tokenkey.c_str());
                            //printQueueFIFO(queueFIFO);

                            bubble("message added in my buffer"); //making animation message more informative
                            //recording some statistics
//                             cout << "source == w.getTarget(): " << " source: " << source << " w.getTarget():" << w.getTarget() << endl;

                        } else { //An entry in the unordered_map was found
                            //do nothing because the message is already in my epidemicLocalBuffer
                        }
                    } else { //The maximum buffer size was reached, so I have to remove the first item from the queueFIFO
                        //Verifying if there is no entry for current message received in my epidemicLocalMessageBuffer
                        unordered_map <string,WaveShortMessage>::iterator got = epidemicLocalMessageBuffer.find(tokenkey.c_str());
                        if (got == epidemicLocalMessageBuffer.end()) { //true value means that there is no entry in the epidemicLocalMessageBuffer for the current message identification
                            epidemicLocalMessageBuffer.erase(queueFIFO.front());
                            epidemicLocalSummaryVector.erase(queueFIFO.front());
                            queueFIFO.pop();
                            //Putting the message in the epidemicLocalMessageBuffer
                            epidemicLocalMessageBuffer.insert(make_pair(tokenkey,w));
                            //Putting the message in the EpidemicLocalSummaryVector
                            epidemicLocalSummaryVector.insert(make_pair(tokenkey.c_str(),true));
                            //FIFO strategy to set the maximum size that a node is willing to allocate epidemic messages in its buffer
                            queueFIFO.push(tokenkey.c_str());
                            //printQueueFIFO(queueFIFO);

                            //recording some statistics
//                                cout << "source == w.getTarget(): " << " source: " << source << " w.getTarget():" << w.getTarget() << endl;
                        } else { //An entry in the unordered_map was found
                            //do nothing because the message is already in my epidemicLocalBuffer
                        }
                    }
                } // end of while
                cout << "After message processing" << endl;
                printEpidemicLocalMessageBuffer();
                cout << "After message processing" << endl;
                printEpidemicLocalSummaryVectorData();
                cout << "After message processing" << endl;
                printQueueFIFO(queueFIFO);
                //changing the turn of the anti-entropy session. If this is the first round, call sendLocalSummaryVector(wsm->getSenderAddress())
                if (wsm->getSenderAddress() < MACToInteger()) {
                    sendLocalSummaryVector(wsm->getSenderAddress());
                }
            } //end of if
        }// end of else

        //Verifying if I'm the target of a message
        if (source.compare(wsm->getTarget()) == 0) {
            if (!wsm->getSummaryVector() && !wsm->getRequestMessages()) { // TODO Change to best programation
                cout << source << " received a message for him at " << simTime() << std::endl;
                if (source.substr(0, 3).compare("rsu") == 0) {
                    messagesReceivedMeasuringRSU(wsm);
                }
            }
        } else { //this node is a relaying node because it is not the target of the message
            findHost()->getDisplayString().updateWith("r=16,green");
            //std::cout << source << " will cache the message for forwarding it later." << " at " << simTime() << std::endl;
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
            //prepareWSM(std::string name, int lengthBits, t_channel channel, int priority, int rcvId, int serial)
            //I our implementation, if rcvId = BROADCAST then we are broadcasting beacons. Otherwise, this parameter must be instantiated with the receiver address
            sendWSM(prepareWSM_epidemic("beacon", beaconLengthBits, type_CCH, beaconPriority, BROADCAST, -1));
            scheduleAt(simTime() + par("beaconInterval").doubleValue(), sendBeaconEvt);
            break;
        }
        case Send_EpidemicMessageRequestEvt: {
            unordered_map <string, WaveShortMessage>::iterator itEM;
            itEM = epidemicMessageSend.begin();

            sendWSM(itEM->second.dup());

            epidemicMessageSend.erase(epidemicMessageSend.begin());
            if (!epidemicMessageSend.empty()) {
                scheduleAt((simTime() + 0.1), sendEpidemicMessageRequestEvt);
            }
        }
        default: {
            if (msg)
                DBG << "APP: Error: Got Self Message of unknown kind! Name: " << msg->getName() << endl;
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
    }else {
        delete sendBeaconEvt;
    }
    findHost()->unsubscribe(mobilityStateChangedSignal, this);
}

BaseWaveApplLayer::~BaseWaveApplLayer() {

}
