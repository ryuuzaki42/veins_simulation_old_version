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

#include "application/epidemic/epidemic.h"

using Veins::TraCIMobilityAccess;
using Veins::AnnotationManagerAccess;
using namespace std;

const simsignalwrap_t epidemic::parkingStateChangedSignal = simsignalwrap_t(TRACI_SIGNAL_PARKING_CHANGE_NAME);

Define_Module(epidemic);

void epidemic::initialize(int stage) {
    BaseWaveApplLayer::initialize_epidemic(stage);
    if (stage == 0) {
        traci = TraCIMobilityAccess().get(getParentModule());
        annotations = AnnotationManagerAccess().getIfExists();
        ASSERT(annotations);

        lastDroveAt = simTime();
        findHost()->subscribe(parkingStateChangedSignal, this);
        isParking = false;
        sendWhileParking = par("sendWhileParking").boolValue();

        //cout << "I'm " << findHost()->getFullName() <<  " myMac: " << myMac << " MACToInteger: " << MACToInteger() << endl;

        //Set the source node which generate a message that will be sent to another random node
        //source = car[0] or car[1] etc.
        source = findHost()->getFullName();

        WATCH(source);
        WATCH(target);
        //WATCH_VECTOR(epidemicLocalMessageBuffer);
        //WATCH(epidemicLocalSummaryVector);
        //WATCH(epidemicRemoteSummaryVector);
        //WATCH(epidemicRequestMessageVector);
        //WATCH(nodesIRecentlySentSummaryVector);
        //WATCH(queueFIFO);
        WATCH(hopCount);
        WATCH(maximumEpidemicBufferSize);
        WATCH(sendSummaryVectorInterval);

        //Generate a message with a probability x=80% to be sent to another node
        //initialize random seed
        //srand (time(NULL));
        // generate secret number between 1 and 100:
        //int probability = rand() % 100 + 1;
        //cout << "Probability of message generation from " << findHost()->getFullName() << " : " << probability << endl;

        //if(probability <= 100) //100 means that all nodes will generate messages to forward in the network
        generateMessage();
    }
}

//This method is called when beacons arrive in the device's wireless interface
void epidemic::onBeacon(WaveShortMessage* wsm) {

    //cout << "I'm " << findHost()->getFullName() << "(" << MACToInteger() << ") and I received a Beacon from " << wsm->getSenderAddress() << " with destination " << wsm->getRecipientAddress() << endl;

    //Verifying if I have the smaller address in order to start the anti-entropy session sending out my summary vector
    if(wsm->getSenderAddress() > MACToInteger()){ //true means that I have the smaller address
        //cout << "I'm " << findHost()->getFullName() << " and my ID is smaller than the Beacon sender." << endl;
        //Verifying if (current contact simutime - last contact simutime) is bigger than previous slide window defined
        //A new summary vector needs to be re-sent to the same contact node only "sendSummaryVectorInterval" seconds after the last one
        unordered_map<unsigned int,simtime_t>::const_iterator got = nodesIRecentlySentSummaryVector.find(wsm->getSenderAddress());
        if(got == nodesIRecentlySentSummaryVector.end()){ //true value means that there is no entry in the nodesIRecentlySentSummaryVector for the current contact node
            //cout << "I'm " << findHost()->getFullName() << ". Contact not found in nodesIRecentlySentSummaryVector. Sending my summary vector to " << wsm->getSenderAddress() << endl;
            //Sending my summary vector to another mobile node
            sendLocalSummaryVector(wsm->getSenderAddress());
            nodesIRecentlySentSummaryVector.insert(make_pair<unsigned int, simtime_t>(wsm->getSenderAddress(),simTime()));
            printNodesIRecentlySentSummaryVector();
        }
        //An entry in the unordered_map was found
        else{
           //cout << "I'm " << findHost()->getFullName() << ". Contact found in nodesIRecentlySentSummaryVector. Node: " << got->first << " added at " << (simTime() - got->second) << " seconds ago." << endl;
           if((simTime() - got->second) > sendSummaryVectorInterval){ //checking if I should update the nodesIRecentlySentSummaryVector entry
               //cout << "I'm " << findHost()->getFullName() << " and I'm updating the entry in the nodesIRecentlySentSummaryVector." << endl;
               sendLocalSummaryVector(wsm->getSenderAddress());
               nodesIRecentlySentSummaryVector.erase(wsm->getSenderAddress());
               nodesIRecentlySentSummaryVector.insert(make_pair<unsigned int, simtime_t>(wsm->getSenderAddress(),simTime()));
               printNodesIRecentlySentSummaryVector();
           }
        }
    }
    //My address is bigger than the Beacon sender
    else{
        //do nothing
        //std::cout << "My ID is bigger than the Beacon sender." << std::endl;
    }
}

void epidemic::onData(WaveShortMessage* wsm) {

    //Verifying the kind of a received message: if a summary vector (true) or a epidemic buffer data message (false).
    if(wsm->getSummaryVector()){
       //checking if the summary vector was sent to me
       if(wsm->getRecipientAddress() == MACToInteger()){
          cout << "I'm " << findHost()->getFullName() << "(" << MACToInteger() << ") and I just recieved the summary vector |> " << wsm->getWsmData() << " <| from " << wsm->getSenderAddress() << endl;
          //Creating the remote summary vector with the data received in wsm->message field
          createEpidemicRemoteSummaryVector(wsm->getWsmData());
          printEpidemicRemoteSummaryVectorData();
          printEpidemicLocalSummaryVectorData();
          //Creating a key vector in order to request messages that I still do not have in my buffer
          createEpidemicRequestMessageVector();
          printEpidemicRequestMessageVector();
          //Verifying if this is the end of second round of the anti-entropy session when the EpidemicRemoteSummaryVector and EpidemicLocalSummaryVector are equals
          if((epidemicRequestMessageVector.empty() ||(strcmp(wsm->getWsmData(),"") == 0)) && (wsm->getSenderAddress() > MACToInteger())){
             //cout << "EpidemicRequestMessageVector from " << findHost()->getFullName() << " is empty now " << endl;
             //cout << "Or strcmp(wsm->getWsmData(),\"\") == 0) " << endl;
             //cout << "And  wsm->getSenderAddress() > MACToInteger() " << endl;
          }
          else if(epidemicRequestMessageVector.empty()){
              //cout << "EpidemicRequestMessageVector from " << findHost()->getFullName() << " is empty now " << endl;
              //changing the turn of the anti-entropy session. In this case, I have not found any differences between EpidemicRemoteSummaryVector and EpidemicLocalSummaryVector but I need to change the round of anti-entropy session
              sendLocalSummaryVector(wsm->getSenderAddress());
          }
          else{
              //Sending a request vector in order to get messages that I don't have
              sendEpidemicRequestMessageVector(wsm->getSenderAddress());
          }
       }
    }
    //is a data message requisition or a data message content
    else{
        //cout << "I'm " << findHost()->getFullName() << ". This is not a summary vector" << endl;
        //Verifying if this is a request message
        if(wsm->getRequestMessages()){
            //checking if the request vector was sent to me
            if(wsm->getRecipientAddress() == MACToInteger()){
                //Searching for elements in the epidemicLocalMessageBuffer and sending them to requester
                cout << "I'm " << findHost()->getFullName() << ". I received the epidemicRequestMessageVector |> " << wsm->getWsmData() << " <| from " << wsm->getSenderAddress() << endl;
                sendMessagesRequested(wsm->getWsmData(), wsm->getSenderAddress());
            }
        }
        else{
             if(wsm->getRecipientAddress() == MACToInteger()){
                 //WSMData generated by car[3]|car[3]|rsu[0]|1.2
                 cout << "I'm " << findHost()->getFullName() << "(" << MACToInteger() << "). I received all the message requested |> " << wsm->getWsmData() << " <| from " << wsm->getSenderAddress() << endl;
                 cout << "Before message processing " << endl;
                 printEpidemicLocalMessageBuffer();
                 cout << "Before message processing " << endl;
                 printEpidemicLocalSummaryVectorData();
                 cout << "Before message processing " << endl;
                 printQueueFIFO(queueFIFO);
                 string delimiter = "|";
                 size_t pos = 0;
                 string tokenkey, tokenData, tokenSource, tokenTarget, tokenTimestamp, tokenhopcount;
                 string messageReceived = wsm->getWsmData();
                 simtime_t st;
                 //cout << "pos = messageReceived.find(delimiter): " << messageReceived.find(delimiter) << endl;
                 //cout << "std::string::npos: " << std::string::npos << endl;
                 while((pos = messageReceived.find(delimiter)) != std::string::npos) {
                        tokenkey = messageReceived.substr(0, pos);
                        messageReceived.erase(0, pos + delimiter.length());
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
                        messageReceived.erase(0, pos + delimiter.length());
                        //cout << "I'm " << findHost()->getFullName() << " and the message received is tokenData: " << tokenData << "tokenSource: " << tokenSource << "tokenTarget: " << tokenTarget << "tokenTimestamp: " << tokenTimestamp << endl;
                        WaveShortMessage w = *wsm;
                        w.setWsmData(tokenData.c_str());
                        w.setSource(tokenSource.c_str());
                        w.setTarget(tokenTarget.c_str());
                        w.setTimestamp(st.parse(tokenTimestamp.c_str()));
                        w.setHopCount(stoi(tokenhopcount));
                        //checking if the maximum buffer size was reached
                        if(queueFIFO.size() < maximumEpidemicBufferSize){
                            //Verifying if there is no entry for current message received in my epidemicLocalMessageBuffer
                            unordered_map<string,WaveShortMessage>::const_iterator got = epidemicLocalMessageBuffer.find(tokenkey);
                            if(got == epidemicLocalMessageBuffer.end()){ //true value means that there is no entry in the epidemicLocalMessageBuffer for the current message identification
                                //Putting the message in the epidemicLocalMessageBuffer
                                epidemicLocalMessageBuffer.insert(MyPairEpidemicMessageBuffer(tokenkey,w));
                                //Putting the message in the EpidemicLocalSummaryVector
                                epidemicLocalSummaryVector.insert(make_pair<string, bool>(tokenkey.c_str(),true));
                                //FIFO strategy to set the maximum size that a node is willing to allocate epidemic messages in its buffer
                                queueFIFO.push(tokenkey.c_str());
                                //printQueueFIFO(queueFIFO);
                            }
                            //An entry in the unordered_map was found
                            else{
                                //do nothing
                            }
                        }
                        else{
                            //Verifying if there is no entry for current message received in my epidemicLocalMessageBuffer
                            unordered_map<string,WaveShortMessage>::const_iterator got = epidemicLocalMessageBuffer.find(tokenkey.c_str());
                            if(got == epidemicLocalMessageBuffer.end()){ //true value means that there is no entry in the epidemicLocalMessageBuffer for the current message identification
                                epidemicLocalMessageBuffer.erase(queueFIFO.front());
                                epidemicLocalSummaryVector.erase(queueFIFO.front());
                                queueFIFO.pop();
                                //Putting the message in the epidemicLocalMessageBuffer
                                epidemicLocalMessageBuffer.insert(MyPairEpidemicMessageBuffer(tokenkey,w));
                                //Putting the message in the EpidemicLocalSummaryVector
                                epidemicLocalSummaryVector.insert(make_pair<string, bool>(tokenkey.c_str(),true));
                                //FIFO strategy to set the maximum size that a node is willing to allocate epidemic messages in its buffer
                                queueFIFO.push(tokenkey.c_str());
                                //printQueueFIFO(queueFIFO);
                            }
                            //An entry in the unordered_map was found
                            else{
                                //do nothing
                            }
                        }
                 } // end of while
                 cout << "After message processing " << endl;
                 printEpidemicLocalMessageBuffer();
                 cout << "After message processing " << endl;
                 printEpidemicLocalSummaryVectorData();
                 cout << "After message processing " << endl;
                 printQueueFIFO(queueFIFO);
                 //changing the turn of the anti-entropy session. If this is the first round, call sendLocalSummaryVector(wsm->getSenderAddress())
                 if(wsm->getSenderAddress() < MACToInteger())
                   sendLocalSummaryVector(wsm->getSenderAddress());
             } //end of if
        }// end of else

        //Verifying if I'm the target of a message
        if(strcmp(wsm->getTarget(),findHost()->getFullName()) == 0){
          std::cout << "I'm " << findHost()->getFullName() << ", the recipient of the message." << " at " << simTime() << std::endl;
          //sendWSM(prepareWSM_epidemic("beacon", beaconLengthBits, type_CCH, beaconPriority, wsm->getSenderAddress(), -1));
        }
        //this node is a relaying node because it is not the target of the message
        else{
            findHost()->getDisplayString().updateWith("r=16,green");
            annotations->scheduleErase(1, annotations->drawLine(wsm->getSenderPos(), traci->getPositionAt(simTime()), "blue"));
            //std::cout << findHost()->getFullName() << " will cache the message for forwarding it later." << " at " << simTime() << std::endl;
        }
    }
}

void epidemic::sendBeacon() {
    t_channel channel = dataOnSch ? type_SCH : type_CCH;
    std::cout << "Channel: " << channel << std::endl;
    WaveShortMessage* wsm = prepareWSM_epidemic("beacon", dataLengthBits, channel, dataPriority, BROADCAST,2);
    sendWSM(wsm);
}

void epidemic::sendWSM(WaveShortMessage* wsm) {
    if (isParking && !sendWhileParking)
        return;
    sendDelayedDown(wsm,individualOffset);
}

//Method used to initiate the anti-entropy session sending the epidemicLocalSummaryVector
void epidemic::sendLocalSummaryVector(unsigned int newRecipientAddress) {
    t_channel channel = dataOnSch ? type_SCH : type_CCH;
    WaveShortMessage* wsm = prepareWSM_epidemic("data", dataLengthBits, channel, dataPriority, newRecipientAddress,2);
    wsm->setSummaryVector(true);
    wsm->setRequestMessages(false);
    //Put the summary vector here, on data wsm field
    wsm->setWsmData(getLocalSummaryVectorData().c_str());
    //Sending the summary vector
    sendWSM(wsm);
}

void epidemic::sendMessagesRequested(string s, unsigned int recipientAddress) {

    cout << "I'm " << findHost()->getFullName() << "(" << MACToInteger() << "). Sending the following messages resquested: " << s << " to " << recipientAddress << endl;
    t_channel channel = dataOnSch ? type_SCH : type_CCH;
    WaveShortMessage* wsm = prepareWSM_epidemic("data", dataLengthBits, channel, dataPriority, recipientAddress,2);
    string delimiter = "|";
    size_t pos = 0;
    string token;
    unsigned i = 0;
    string message="";

    printEpidemicLocalMessageBuffer();

    //Extracting who request message
    pos = s.find(delimiter);
    string tokenRequester = s.substr(0, pos);
    s.erase(0, pos + delimiter.length());

    while ((pos = s.find(delimiter)) != std::string::npos) {
      //Catch jus the key of the local summary vector
      if(i%2 == 0){
          ostringstream ss;
          //cout << "i = " << i << " - looking for the message key(s.substr(0, pos)): " << s.substr(0, pos) << endl;
          WaveShortMessage w;
          unordered_map<string,WaveShortMessage>::const_iterator got = epidemicLocalMessageBuffer.find(s.substr(0, pos));
          if(got == epidemicLocalMessageBuffer.end()){ //true value means that there is no entry in the epidemicLocalSummaryVector for a epidemicRemoteSummaryVector key
          }
          else{
               w = got->second;
               //WaveShortMessage w = getEpidemicLocalMessageBuffer(s.substr(0, pos));
               //Verifying if I'm still able to spread the message or not. If w.getHopCount == 1 I'm able to send the message only to its target
               if(w.getHopCount() > 1){
                 ss << s.substr(0, pos) << "|" << w.getWsmData() << "|" << w.getSource() << "|" << w.getTarget() << "|" << w.getTimestamp() << "|" << w.getHopCount() - 1 << "|";
               }
               else if(w.getHopCount() == 1){
                 if((strcmp(tokenRequester.c_str(),w.getTarget()) == 0)){
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
    }
    message = message.substr(0, message.length() - 1);
    cout << "Message that is sending as a result of a requisition vector request. wsm->setWsmData: " <<  message << endl;
    wsm->setWsmData(message.c_str());
    wsm->setSummaryVector(false);
    wsm->setRequestMessages(false);
    sendWSM(wsm);
}

void epidemic::sendEpidemicRequestMessageVector(unsigned int newRecipientAddress) {
    t_channel channel = dataOnSch ? type_SCH : type_CCH;
    WaveShortMessage* wsm = prepareWSM_epidemic("data", dataLengthBits, channel, dataPriority, newRecipientAddress,2);
    wsm->setSummaryVector(false);
    wsm->setRequestMessages(true);
    //Put the summary vector here
    wsm->setWsmData(getEpidemicRequestMessageVectorData().c_str());
    //Sending the summary vector
    sendWSM(wsm);
    //cout << "Sending a vector of request messages from " << findHost()->getFullName() <<"(" << MACToInteger() << ") to " << newRecipientAddress << endl;
    //epidemicRequestMessageVector.clear();
}

void epidemic::receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj) {
    Enter_Method_Silent();
    if (signalID == mobilityStateChangedSignal) {
        handlePositionUpdate(obj);
    }
    else if (signalID == parkingStateChangedSignal) {
        handleParkingUpdate(obj);
    }
}
void epidemic::handleParkingUpdate(cObject* obj) {
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
void epidemic::handlePositionUpdate(cObject* obj) {
    BaseWaveApplLayer::handlePositionUpdate(obj);
}

void epidemic::createEpidemicRemoteSummaryVector(string s){
    //cout << "Creating the epidemicRemoteSummaryVector in " << findHost()->getFullName() << endl;
    string delimiter = "|";
    size_t pos = 0;
    string token;
    unsigned i = 0;
    epidemicRemoteSummaryVector.clear();
    while ((pos = s.find(delimiter)) != std::string::npos) {
      //token = s.substr(0, pos);
      //std::cout << token << std::endl;
      //Catch jus the key of the local summary vector
      if(i%2 == 0)
        epidemicRemoteSummaryVector.insert(make_pair<string, bool>(s.substr(0, pos),true));
      s.erase(0, pos + delimiter.length());
      i++;
    }
    //std::cout << s << std::endl;
}

void epidemic::createEpidemicRequestMessageVector(){
    epidemicRequestMessageVector.clear();
    for(auto& x: epidemicRemoteSummaryVector){
        unordered_map<string,bool>::const_iterator got = epidemicLocalSummaryVector.find(x.first);
        //cout << "I'm in createEpidemicRequestMessageVector().  x.first: " << x.first << " x.second: " << x.second << endl;
        if(got == epidemicLocalSummaryVector.end()){ //true value means that there is no entry in the epidemicLocalSummaryVector for a epidemicRemoteSummaryVector key
            //Putting the message in the EpidemicRequestMessageVector
            string s = x.first;
            epidemicRequestMessageVector.insert(make_pair<string, bool>(s.c_str(),true));
        }
        //An entry in the unordered_map was found
        else{
            //cout << "I'm in createEpidemicRequestMessageVector().  got->first: " << got->first << " got->second: " << got->second << endl;
            //cout << "The message " << got->first << " in the epidemicRemoteSummaryVector was found in my epidemicLocalSummaryVector." << endl;
        }
    }
}

string epidemic::getEpidemicRequestMessageVectorData(){
    ostringstream ss;
    //adding the requester name in order to identify if the requester is also the target of the messages with hopcount == 1.
    //In this case, hopcount == 1, the messages can be sent to the target. Otherwise, the message will not be spread
    ss << findHost()->getFullName() << "|";
    for(auto& x: epidemicRequestMessageVector)
        ss << x.first << "|" << x.second << "|";
    string s = ss.str();
    s = s.substr(0, s.length() - 1);
    cout << "String format of EpidemicRequestMessageVector from " << findHost()->getFullName() << ": " <<  s  << endl;
    return s.c_str();
}

//WaveShortMessage epidemic::getEpidemicLocalMessageBuffer(string s){
//    //cout << "Getting the epidemicLocalMessageBuffer[" << s << "] from " << findHost()->getFullName() << endl;
//    unordered_map<string,WaveShortMessage>::const_iterator got = epidemicLocalMessageBuffer.find(s.c_str());
//    return got->second;
//}

//Method used to convert the unordered_map epidemicLocalSummaryVectorData in a string
string epidemic::getLocalSummaryVectorData(){
    ostringstream ss;
    for(auto& x: epidemicLocalSummaryVector)
        ss << x.first << "|" << x.second << "|";
    string s = ss.str();
    s = s.substr(0, s.length() - 1);
    //cout << "EpidemicLocalSummaryVector from " << findHost()->getFullName() << "(" << MACToInteger() << "): " <<  s  << endl;
    return s.c_str();
}

//Generate a target in order to send a message
void epidemic::generateTarget(){
    //Set the target node to whom my message has to be delivered
    //target = rsu[0] rsu[1] or car[*].
    //if ((strcmp(findHost()->getFullName(),"car[4]") == 0) || (strcmp(findHost()->getFullName(),"car[13]") == 0))
    //    target = "car[7]";
    //else
        target = "rsu[0]";
    //cout << findHost()->getFullName() << "generating a RSU target to its message (" << source << " -> " << target << ")"<< endl;
}

//Generate a message in order to be sent to a target
void epidemic::generateMessage(){
    //Set the target node to whom my message has to be delivered
    //target = rsu[0] rsu[1] or car[*].
    generateTarget();

    //cout << findHost()->getFullName() << " generating a message to be sent (" << source << " -> " << target << ")"<< endl;

    WaveShortMessage wsm;
    wsm.setName("data");
    t_channel channel = dataOnSch ? type_SCH : type_CCH;
    wsm.addBitLength(headerLength);
    wsm.addBitLength(dataLengthBits);
    switch (channel) {
        case type_SCH: wsm.setChannelNumber(Channels::SCH1); break; //will be rewritten at Mac1609_4 to actual Service Channel. This is just so no controlInfo is needed
        case type_CCH: wsm.setChannelNumber(Channels::CCH); break;
    }
    wsm.setPsid(0);
    wsm.setPriority(dataPriority);
    wsm.setWsmVersion(1);
    wsm.setTimestamp(simTime());
    wsm.setSenderAddress(MACToInteger());
    wsm.setRecipientAddress(BROADCAST);
    wsm.setSource(source.c_str());
    wsm.setTarget(target.c_str());
    wsm.setSenderPos(curPosition);
    wsm.setSerial(2);
    wsm.setSummaryVector(false);
    string data = "WSMData generated by ";
    data += findHost()->getFullName();
    wsm.setWsmData(data.c_str());
    wsm.setLocalMessageIdentificaton(to_string(epidemic::messageId).c_str());
    wsm.setGlobalMessageIdentificaton((to_string(epidemic::messageId) + to_string(MACToInteger())).c_str());
    wsm.setHopCount(hopCount);
    epidemic::messageId++;

    //Putting the message in the epidemicLocalMessageBuffer
    //epidemicLocalMessageBuffer.insert(MyPairEpidemicMessageBuffer(to_string(MACToInteger()),wsm));
    epidemicLocalMessageBuffer.insert(MyPairEpidemicMessageBuffer(wsm.getGlobalMessageIdentificaton(),wsm));
    //Putting the message key in the epidemicLocalSummaryVector
    //epidemicLocalSummaryVector.insert(make_pair<string, bool>(to_string(MACToInteger()),true));
    epidemicLocalSummaryVector.insert(make_pair<string, bool>(wsm.getGlobalMessageIdentificaton(),true));
    //FIFO strategy to set the maximum size that a node is willing to allocate epidemic messages in its buffer
    queueFIFO.push(wsm.getGlobalMessageIdentificaton());

    //printQueueFIFO(queueFIFO);
    //printEpidemicLocalMessageBuffer();
    //printEpidemicLocalSummaryVectorData();

}

void epidemic::printQueueFIFO(queue<string> qFIFO){
    int i = 0;
    while(!qFIFO.empty()){
        cout << "I'm " << findHost()->getFullName() << " - queueFIFO Element " << ++i << ": " << qFIFO.front() << endl;
        qFIFO.pop();
    }
}

void epidemic::printEpidemicLocalMessageBuffer(){
    if(epidemicLocalMessageBuffer.empty()){
           cout << "EpidemicLocalMessageBuffer from " << findHost()->getFullName() << " is empty now " << endl;
    }
    else{
        int i = 0;
        cout << "Printing the epidemicLocalMessageBuffer from " << findHost()->getFullName() << "(" << MACToInteger() <<"):" << endl;
        for(auto& x: epidemicLocalMessageBuffer){
            WaveShortMessage wsmBuffered = x.second;
            cout << " Key " << ++i << ": " << x.first << " - Message Content: " << wsmBuffered.getWsmData() << " source: " << wsmBuffered.getSource() << " target: " << wsmBuffered.getTarget() << " Timestamp: " << wsmBuffered.getTimestamp() << " HopCount: " << wsmBuffered.getHopCount() << endl;
        }
    }
}

void epidemic::printEpidemicLocalSummaryVectorData(){
    if(epidemicLocalSummaryVector.empty()){
           cout << "EpidemicLocalSummaryVector from " << findHost()->getFullName() << " is empty now " << endl;
    }
    else{
        ostringstream ss;
        for(auto& x: epidemicLocalSummaryVector)
           ss << x.first << "|" << x.second << "|";
        string s = ss.str();
        s = s.substr(0, s.length() - 1);
        cout << "EpidemicLocalSummaryVector from " << findHost()->getFullName() << "(" << MACToInteger() << "): " <<  s  << endl;
    }
}

void epidemic::printEpidemicRemoteSummaryVectorData(){
    if(epidemicRemoteSummaryVector.empty()){
           cout << "EpidemicRemoteSummaryVector from " << findHost()->getFullName() << " is empty now " << endl;
    }
    else{
        ostringstream ss;
        for(auto& x: epidemicRemoteSummaryVector)
            ss << x.first << "|" << x.second << "|";
        string s = ss.str();
        s = s.substr(0, s.length() - 1);
        cout << "EpidemicRemoteSummaryVector from " << findHost()->getFullName() << ": " <<  s  << endl;
    }
}

void epidemic::printEpidemicRequestMessageVector(){

    if(epidemicRequestMessageVector.empty()){
        cout << "EpidemicRequestMessageVector from " << findHost()->getFullName() << " is empty now " << endl;
    }
    else{
        ostringstream ss;
        for(auto& x: epidemicRequestMessageVector)
            ss << x.first << "|" << x.second << "|";
        string s = ss.str();
        s = s.substr(0, s.length() - 1);
        cout << "EpidemicRequestMessageVector from " << findHost()->getFullName() << ": " <<  s  << endl;
    }
}

void epidemic::printNodesIRecentlySentSummaryVector(){
    if(nodesIRecentlySentSummaryVector.empty()){
           cout << "NodesIRecentlySentSummaryVector from " << findHost()->getFullName() << " is empty now " << endl;
    }
    else{
        int i = 0;
        cout << "NodesIRecentlySentSummaryVector from " << findHost()->getFullName() << " (" << MACToInteger() << "):" << endl;
        for(auto& x: nodesIRecentlySentSummaryVector)
            cout << ++i << " Node: " << x.first << " added at " << x.second << endl;
    }
}

void epidemic::printWaveShortMessage(WaveShortMessage wsm){
    cout << "wsm.getName():" << wsm.getName() << endl;
    cout << "wsm.getBitLength():" << wsm.getBitLength() << endl;
    cout << "wsm.getChannelNumber():" << wsm.getChannelNumber() << endl;
    cout << "wsm.getPsid():" << wsm.getPsid() << endl;
    cout << "wsm.getPriority():" << wsm.getPriority() << endl;
    cout << "wsm.getWsmVersion():" << wsm.getWsmVersion() << endl;
    cout << "wsm.getTimestamp():" << wsm.getTimestamp() << endl;
    cout << "wsm.getSenderAddress():" << wsm.getSenderAddress() << endl;
    cout << "wsm.getRecipientAddress():" << wsm.getRecipientAddress() << endl;
    cout << "wsm.getSource():" << wsm.getSource() << endl;
    cout << "wsm.getTarget():" << wsm.getTarget() << endl;
    cout << "wsm.getSenderPos():" << wsm.getSenderPos() << endl;
    cout << "wsm.getSerial():" << wsm.getSerial() << endl;
    cout << "wsm.getSummaryVector():" << wsm.getSummaryVector() << endl;
    cout << "wsm.getRequestMessages():" << wsm.getRequestMessages() << endl;
    cout << "wsm.getWsmData():" << wsm.getWsmData() << endl;
    cout << "wsm.getLocalMessageIdentificaton(): " << wsm.getLocalMessageIdentificaton() << endl;
    cout << "wsm.getGlobalMessageIdentificaton(): " << wsm.getGlobalMessageIdentificaton() << endl;
    cout << "wsm.getHopCount(): " << wsm.getHopCount() << endl;
}

void epidemic::printWaveShortMessage(WaveShortMessage* wsm){
    cout << "wsm->getName()" << wsm->getName() << endl;
    cout << "wsm->getBitLength()" << wsm->getBitLength() << endl;
    cout << "wsm->getChannelNumber()" << wsm->getChannelNumber() << endl;
    cout << "wsm->getPsid()" << wsm->getPsid() << endl;
    cout << "wsm->getPriority()" << wsm->getPriority() << endl;
    cout << "wsm->getWsmVersion()" << wsm->getWsmVersion() << endl;
    cout << "wsm->getTimestamp()" << wsm->getTimestamp() << endl;
    cout << "wsm->getSenderAddress()" << wsm->getSenderAddress() << endl;
    cout << "wsm->getRecipientAddress()" << wsm->getRecipientAddress() << endl;
    cout << "wsm->getSource()" << wsm->getSource() << endl;
    cout << "wsm->getTarget()" << wsm->getTarget() << endl;
    cout << "wsm->getSenderPos()" << wsm->getSenderPos() << endl;
    cout << "wsm->getSerial()" << wsm->getSerial() << endl;
    cout << "wsm->getSummaryVector()" << wsm->getSummaryVector() << endl;
    cout << "wsm->getRequestMessages()" << wsm->getRequestMessages() << endl;
    cout << "wsm->getWsmData()" << wsm->getWsmData() << endl;
}

void epidemic::finish(){
    cout << "Number of Messages Generated: " << epidemic::messageId << endl;
    recordScalar("#numMessageGenerated", epidemic::messageId);

}
