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

#include "application/mfcv_epidemic/mfcv_epidemic.h"

using Veins::TraCIMobilityAccess;
using Veins::AnnotationManagerAccess;
using namespace std;

const simsignalwrap_t mfcv_epidemic::parkingStateChangedSignal = simsignalwrap_t(TRACI_SIGNAL_PARKING_CHANGE_NAME);

Define_Module(mfcv_epidemic);

void mfcv_epidemic::initialize(int stage) {
    BaseWaveApplLayer::initialize_mfcv_epidemic(stage);
    if (stage == 0) {


        //test Jonh
        BaseWaveApplLayer::vehCount++;


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

        cout << "target before generateMessage()" << target << endl << endl;

        WATCH(source);
        WATCH(target);
        //WATCH_VECTOR(mfcv_epidemicLocalMessageBuffer);
        //WATCH(mfcv_epidemicLocalSummaryVector);
        //WATCH(mfcv_epidemicRemoteSummaryVector);
        //WATCH(mfcv_epidemicRequestMessageVector);
        //WATCH(nodesIRecentlySentSummaryVector);
        //WATCH(queueFIFO);
        WATCH(hopCount);
        WATCH(maximumMfcvEpidemicBufferSize);
        WATCH(sendSummaryVectorInterval);

        //Generate a message with a probability x=80% to be sent to another node
        //initialize random seed
        //srand (time(NULL));
        // generate secret number between 1 and 100:
        //int probability = rand() % 100 + 1;
        //cout << "Probability of message generation from " << findHost()->getFullName() << " : " << probability << endl;

        //if(probability <= 100) //100 means that all nodes will generate messages to forward in the network
        //generateMessage();

        // test Jonh

        // source ex: car[0], car[1], car[2]
        if (source.compare(0,3,"car") == 0) {
            cout << "It is a veh, and my fullName is: " << source << endl;
        } else {
            cout << "It not a veh, and my fullName is: " << source << endl;
        }

        // only car[0] generate message
        //if (source.compare("car[0]") == 0) {
          generateMessage();
        //}

        cout << "target after generateMessage(): " << target << endl << endl;

        if (source.compare(0,3,"car") == 0) {
            //Open a new file for the current simulation
            myfile.open ("results/onBeacon_veh.txt");
            myfile.close();

            //Open a new file for the current simulation
            myfile.open ("results/LocalMessageBuffer_veh.txt");
            myfile.close();
        }

        cout << "veh ID " << traci->getId() << endl;
        cout << "simTime() " << SimTime() << endl;
        cout << "ExternalId " << traci->getExternalId() << endl;
        cout << "Index " << traci->getIndex() << endl;
        cout << "veh BaseWaveApplLayer::vehCount " << BaseWaveApplLayer::vehCount << endl;
        cout << "veh BaseWaveApplLayer::rsuCount " << BaseWaveApplLayer::rsuCount << endl;
        cout << endl;
    }
}

//This method is called when beacons arrive in the device's wireless interface
void mfcv_epidemic::onBeacon(WaveShortMessage* wsm) {

    //cout << "I'm " << findHost()->getFullName() << "(" << MACToInteger() << ") and I received a Beacon from " << wsm->getSenderAddress() << " with destination " << wsm->getRecipientAddress() << endl;


    //test Jonh
    cout << "wsm->getSenderPos()" << wsm->getSenderPos() << endl;
    cout << "traci->getCurrentPosition()" << traci->getCurrentPosition() << endl;
    cout << "Air distance (100.0,100.0,3.895) to (200.0,200.0,1.895): " << traci->commandDistanceRequest(Coord(100.0,100.0,3.895), Coord(200.0,200.0,1.895), false) << endl ;
    cout << "Driving distance (100.0,100.0,1.895) to (200.0,200.0,1.895): " << traci->commandDistanceRequest(Coord(100.0,100.0,1.895), Coord(200.0,200.0,1.895), true) << endl ;
    cout << "1- Air distance wsm to local: " << traci->commandDistanceRequest(wsm->getSenderPos(), traci->getCurrentPosition(), false) << endl;
    cout << "1- Driving distance wsm to local: " << traci->commandDistanceRequest(wsm->getSenderPos(), traci->getCurrentPosition(), true) << endl ;
    cout << "2- Air distance local to wsm: " << traci->commandDistanceRequest(traci->getCurrentPosition(), wsm->getSenderPos(),false) << endl;
    cout << "2- Driving distance local to wsm: " << traci->commandDistanceRequest(traci->getCurrentPosition(), wsm->getSenderPos(), true) << endl;

    Coord sendersPos  = wsm->getSenderPos(/*sStart*/);
    Coord receiverPos = traci->getCurrentPosition(/*sStart*/);
    cout << "receiverPos.distance(sendersPos) " << receiverPos.distance(sendersPos) << endl;
    cout << "traci->getCurrentPosition().distance(wsm->getSenderPos()) " << traci->getCurrentPosition().distance(wsm->getSenderPos()) << endl;
    cout << "time-point used " << receiverPos.distance(sendersPos) / BaseWorldUtility::speedOfLight << endl;
    cout << "wsm->getSenderPos()" << wsm->getSenderPos() << endl;
    cout << "traci->getCurrentPosition()" << traci->getCurrentPosition() << endl << endl;

    //bubble a short message in the device (in this case a vehicle)
    findHost()->bubble("Received Beacon");


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

void mfcv_epidemic::onData(WaveShortMessage* wsm) {


    //test Jonh
    recordOnFile(wsm);

    //bubble a short message in the device (in this case a vehicle)
    findHost()->bubble("Received Data");

    //Verifying the kind of a received message: if a summary vector (true) or a mfcv_epidemic buffer data message (false).
    if(wsm->getSummaryVector()){
       //checking if the summary vector was sent to me
       if(wsm->getRecipientAddress() == MACToInteger()){
          cout << "I'm " << findHost()->getFullName() << "(" << MACToInteger() << ") and I just recieved the summary vector |> " << wsm->getWsmData() << " <| from " << wsm->getSenderAddress() << endl;
          //Creating the remote summary vector with the data received in wsm->message field
          createMfcv_EpidemicRemoteSummaryVector(wsm->getWsmData());
          printMfcv_EpidemicRemoteSummaryVectorData();
          printMfcv_EpidemicLocalSummaryVectorData();
          //Creating a key vector in order to request messages that I still do not have in my buffer
          createMfcv_EpidemicRequestMessageVector();
          printMfcv_EpidemicRequestMessageVector();
          //Verifying if this is the end of second round of the anti-entropy session when the Mfcv_EpidemicRemoteSummaryVector and Mfcv_EpidemicLocalSummaryVector are equals
          if((mfcv_epidemicRequestMessageVector.empty() ||(strcmp(wsm->getWsmData(),"") == 0)) && (wsm->getSenderAddress() > MACToInteger())){
             //cout << "Mfcv_EpidemicRequestMessageVector from " << findHost()->getFullName() << " is empty now " << endl;
             //cout << "Or strcmp(wsm->getWsmData(),\"\") == 0) " << endl;
             //cout << "And  wsm->getSenderAddress() > MACToInteger() " << endl;
          }
          else if(mfcv_epidemicRequestMessageVector.empty()){
              //cout << "Mfcv_EpidemicRequestMessageVector from " << findHost()->getFullName() << " is empty now " << endl;
              //changing the turn of the anti-entropy session. In this case, I have not found any differences between Mfcv_EpidemicRemoteSummaryVector and Mfcv_EpidemicLocalSummaryVector but I need to change the round of anti-entropy session
              sendLocalSummaryVector(wsm->getSenderAddress());
          }
          else{
              //Sending a request vector in order to get messages that I don't have
              sendMfcv_EpidemicRequestMessageVector(wsm->getSenderAddress());
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
                //Searching for elements in the mfcv_epidemicLocalMessageBuffer and sending them to requester
                cout << "I'm " << findHost()->getFullName() << ". I received the mfcv_epidemicRequestMessageVector |> " << wsm->getWsmData() << " <| from " << wsm->getSenderAddress() << endl;
                sendMessagesRequested(wsm->getWsmData(), wsm->getSenderAddress());
            }
        }
        else{
             if(wsm->getRecipientAddress() == MACToInteger()){
                 //WSMData generated by car[3]|car[3]|rsu[0]|1.2
                 cout << "I'm " << findHost()->getFullName() << "(" << MACToInteger() << "). I received all the message requested |> " << wsm->getWsmData() << " <| from " << wsm->getSenderAddress() << endl;
                 cout << "Before message processing " << endl;
                 printMfcv_EpidemicLocalMessageBuffer();
                 cout << "Before message processing " << endl;
                 printMfcv_EpidemicLocalSummaryVectorData();
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
                        if(queueFIFO.size() < maximumMfcvEpidemicBufferSize){
                            //Verifying if there is no entry for current message received in my mfcv_epidemicLocalMessageBuffer
                            unordered_map<string,WaveShortMessage>::const_iterator got = mfcv_epidemicLocalMessageBuffer.find(tokenkey);
                            if(got == mfcv_epidemicLocalMessageBuffer.end()){ //true value means that there is no entry in the mfcv_epidemicLocalMessageBuffer for the current message identification
                                //Putting the message in the mfcv_epidemicLocalMessageBuffer
                                mfcv_epidemicLocalMessageBuffer.insert(MyPairMfcv_EpidemicMessageBuffer(tokenkey,w));
                                //Putting the message in the Mfcv_EpidemicLocalSummaryVector
                                mfcv_epidemicLocalSummaryVector.insert(make_pair<string, bool>(tokenkey.c_str(),true));
                                //FIFO strategy to set the maximum size that a node is willing to allocate mfcv_epidemic messages in its buffer
                                queueFIFO.push(tokenkey.c_str());
                                //printQueueFIFO(queueFIFO);
                            }
                            //An entry in the unordered_map was found
                            else{
                                //do nothing
                            }
                        }
                        else{
                            //Verifying if there is no entry for current message received in my mfcv_epidemicLocalMessageBuffer
                            unordered_map<string,WaveShortMessage>::const_iterator got = mfcv_epidemicLocalMessageBuffer.find(tokenkey.c_str());
                            if(got == mfcv_epidemicLocalMessageBuffer.end()){ //true value means that there is no entry in the mfcv_epidemicLocalMessageBuffer for the current message identification
                                mfcv_epidemicLocalMessageBuffer.erase(queueFIFO.front());
                                mfcv_epidemicLocalSummaryVector.erase(queueFIFO.front());
                                queueFIFO.pop();
                                //Putting the message in the mfcv_epidemicLocalMessageBuffer
                                mfcv_epidemicLocalMessageBuffer.insert(MyPairMfcv_EpidemicMessageBuffer(tokenkey,w));
                                //Putting the message in the Mfcv_EpidemicLocalSummaryVector
                                mfcv_epidemicLocalSummaryVector.insert(make_pair<string, bool>(tokenkey.c_str(),true));
                                //FIFO strategy to set the maximum size that a node is willing to allocate mfcv_epidemic messages in its buffer
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
                 printMfcv_EpidemicLocalMessageBuffer();
                 cout << "After message processing " << endl;
                 printMfcv_EpidemicLocalSummaryVectorData();
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
          //sendWSM(prepareWSM_mfcv_epidemic("beacon", beaconLengthBits, type_CCH, beaconPriority, wsm->getSenderAddress(), -1));
        }
        //this node is a relaying node because it is not the target of the message
        else{
            findHost()->getDisplayString().updateWith("r=16,green");
            annotations->scheduleErase(1, annotations->drawLine(wsm->getSenderPos(), traci->getPositionAt(simTime()), "blue"));
            //std::cout << findHost()->getFullName() << " will cache the message for forwarding it later." << " at " << simTime() << std::endl;
        }
    }
}

void mfcv_epidemic::sendBeacon() {
    t_channel channel = dataOnSch ? type_SCH : type_CCH;
    std::cout << "Channel: " << channel << std::endl;
    WaveShortMessage* wsm = prepareWSM_mfcv_epidemic("beacon", dataLengthBits, channel, dataPriority, BROADCAST,2);
    sendWSM(wsm);
}

void mfcv_epidemic::sendWSM(WaveShortMessage* wsm) {
    if (isParking && !sendWhileParking)
        return;
    sendDelayedDown(wsm,individualOffset);
}

//Method used to initiate the anti-entropy session sending the mfcv_epidemicLocalSummaryVector
void mfcv_epidemic::sendLocalSummaryVector(unsigned int newRecipientAddress) {
    t_channel channel = dataOnSch ? type_SCH : type_CCH;
    WaveShortMessage* wsm = prepareWSM_mfcv_epidemic("data", dataLengthBits, channel, dataPriority, newRecipientAddress,2);
    wsm->setSummaryVector(true);
    wsm->setRequestMessages(false);
    //Put the summary vector here, on data wsm field
    wsm->setWsmData(getLocalSummaryVectorData().c_str());
    //Sending the summary vector
    sendWSM(wsm);
}

void mfcv_epidemic::sendMessagesRequested(string s, unsigned int recipientAddress) {

    cout << "I'm " << findHost()->getFullName() << "(" << MACToInteger() << "). Sending the following messages resquested: " << s << " to " << recipientAddress << endl;
    t_channel channel = dataOnSch ? type_SCH : type_CCH;
    WaveShortMessage* wsm = prepareWSM_mfcv_epidemic("data", dataLengthBits, channel, dataPriority, recipientAddress,2);
    string delimiter = "|";
    size_t pos = 0;
    string token;
    unsigned i = 0;
    string message="";

    printMfcv_EpidemicLocalMessageBuffer();

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
          unordered_map<string,WaveShortMessage>::const_iterator got = mfcv_epidemicLocalMessageBuffer.find(s.substr(0, pos));
          if(got == mfcv_epidemicLocalMessageBuffer.end()){ //true value means that there is no entry in the mfcv_epidemicLocalSummaryVector for a mfcv_epidemicRemoteSummaryVector key
          }
          else{
               w = got->second;
               //WaveShortMessage w = getMfcv_EpidemicLocalMessageBuffer(s.substr(0, pos));
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

void mfcv_epidemic::sendMfcv_EpidemicRequestMessageVector(unsigned int newRecipientAddress) {
    t_channel channel = dataOnSch ? type_SCH : type_CCH;
    WaveShortMessage* wsm = prepareWSM_mfcv_epidemic("data", dataLengthBits, channel, dataPriority, newRecipientAddress,2);
    wsm->setSummaryVector(false);
    wsm->setRequestMessages(true);
    //Put the summary vector here
    wsm->setWsmData(getMfcv_EpidemicRequestMessageVectorData().c_str());
    //Sending the summary vector
    sendWSM(wsm);
    //cout << "Sending a vector of request messages from " << findHost()->getFullName() <<"(" << MACToInteger() << ") to " << newRecipientAddress << endl;
    //mfcv_epidemicRequestMessageVector.clear();
}

void mfcv_epidemic::receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj) {
    Enter_Method_Silent();
    if (signalID == mobilityStateChangedSignal) {
        handlePositionUpdate(obj);
    }
    else if (signalID == parkingStateChangedSignal) {
        handleParkingUpdate(obj);
    }
}
void mfcv_epidemic::handleParkingUpdate(cObject* obj) {
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
void mfcv_epidemic::handlePositionUpdate(cObject* obj) {
    BaseWaveApplLayer::handlePositionUpdate(obj);
}

void mfcv_epidemic::createMfcv_EpidemicRemoteSummaryVector(string s){
    //cout << "Creating the mfcv_epidemicRemoteSummaryVector in " << findHost()->getFullName() << endl;
    string delimiter = "|";
    size_t pos = 0;
    string token;
    unsigned i = 0;
    mfcv_epidemicRemoteSummaryVector.clear();
    while ((pos = s.find(delimiter)) != std::string::npos) {
      //token = s.substr(0, pos);
      //std::cout << token << std::endl;
      //Catch jus the key of the local summary vector
      if(i%2 == 0)
        mfcv_epidemicRemoteSummaryVector.insert(make_pair<string, bool>(s.substr(0, pos),true));
      s.erase(0, pos + delimiter.length());
      i++;
    }
    //std::cout << s << std::endl;
}

void mfcv_epidemic::createMfcv_EpidemicRequestMessageVector(){
    mfcv_epidemicRequestMessageVector.clear();
    for(auto& x: mfcv_epidemicRemoteSummaryVector){
        unordered_map<string,bool>::const_iterator got = mfcv_epidemicLocalSummaryVector.find(x.first);
        //cout << "I'm in createMfcv_EpidemicRequestMessageVector().  x.first: " << x.first << " x.second: " << x.second << endl;
        if(got == mfcv_epidemicLocalSummaryVector.end()){ //true value means that there is no entry in the mfcv_epidemicLocalSummaryVector for a mfcv_epidemicRemoteSummaryVector key
            //Putting the message in the Mfcv_EpidemicRequestMessageVector
            string s = x.first;
            mfcv_epidemicRequestMessageVector.insert(make_pair<string, bool>(s.c_str(),true));
        }
        //An entry in the unordered_map was found
        else{
            //cout << "I'm in createMfcv_EpidemicRequestMessageVector().  got->first: " << got->first << " got->second: " << got->second << endl;
            //cout << "The message " << got->first << " in the mfcv_epidemicRemoteSummaryVector was found in my mfcv_epidemicLocalSummaryVector." << endl;
        }
    }
}

string mfcv_epidemic::getMfcv_EpidemicRequestMessageVectorData(){
    ostringstream ss;
    //adding the requester name in order to identify if the requester is also the target of the messages with hopcount == 1.
    //In this case, hopcount == 1, the messages can be sent to the target. Otherwise, the message will not be spread
    ss << findHost()->getFullName() << "|";
    for(auto& x: mfcv_epidemicRequestMessageVector)
        ss << x.first << "|" << x.second << "|";
    string s = ss.str();
    s = s.substr(0, s.length() - 1);
    cout << "String format of Mfcv_EpidemicRequestMessageVector from " << findHost()->getFullName() << ": " <<  s  << endl;
    return s.c_str();
}

//WaveShortMessage mfcv_epidemic::getMfcv_EpidemicLocalMessageBuffer(string s){
//    //cout << "Getting the mfcv_epidemicLocalMessageBuffer[" << s << "] from " << findHost()->getFullName() << endl;
//    unordered_map<string,WaveShortMessage>::const_iterator got = mfcv_epidemicLocalMessageBuffer.find(s.c_str());
//    return got->second;
//}

//Method used to convert the unordered_map mfcv_epidemicLocalSummaryVectorData in a string
string mfcv_epidemic::getLocalSummaryVectorData(){
    ostringstream ss;
    for(auto& x: mfcv_epidemicLocalSummaryVector)
        ss << x.first << "|" << x.second << "|";
    string s = ss.str();
    s = s.substr(0, s.length() - 1);
    //cout << "Mfcv_EpidemicLocalSummaryVector from " << findHost()->getFullName() << "(" << MACToInteger() << "): " <<  s  << endl;
    return s.c_str();
}

//Generate a target in order to send a message
void mfcv_epidemic::generateTarget(){
    //Set the target node to whom my message has to be delivered
    //target = rsu[0] rsu[1] or car[*].
    //if ((strcmp(findHost()->getFullName(),"car[4]") == 0) || (strcmp(findHost()->getFullName(),"car[13]") == 0))
    //    target = "car[7]";
    //else
        target = "rsu[0]";
    //cout << findHost()->getFullName() << "generating a RSU target to its message (" << source << " -> " << target << ")"<< endl;


    // text Jonh
    srand (time(NULL));
    cout << endl;
    cout << "Host count " << (BaseWaveApplLayer::vehCount + BaseWaveApplLayer::rsuCount) << endl;
    int targetRandom = rand() % (BaseWaveApplLayer::vehCount + BaseWaveApplLayer::rsuCount);
    cout << "targetRandom "<< targetRandom << endl;

    cout << "BaseWaveApplLayer::vehCount "<< BaseWaveApplLayer::vehCount << endl;
    cout << "BaseWaveApplLayer::rsuCount "<< BaseWaveApplLayer::rsuCount << endl;
    //cout << "rand "<< rand() % (BaseWaveApplLayer::vehCount + BaseWaveApplLayer::rsuCount) << endl;
    cout << endl;

    if ( targetRandom <= BaseWaveApplLayer::vehCount){
        cout << "targetRandom: " << targetRandom << " set a veh as a target, with: car[" << targetRandom << "]" << endl;
    }else{ //targetRandom > BaseWaveApplLayer::vehCount
        cout << "targetRandom: " << targetRandom << " set a rsu as a target, with: rsu[" << targetRandom - BaseWaveApplLayer::vehCount <<"]" << endl;
    }

}

//Generate a message in order to be sent to a target
void mfcv_epidemic::generateMessage(){


    // test Jonh
    cout << endl;
    cout << "In generateMessage() veh fullName" << findHost()->getFullName() <<endl;
    cout << endl;


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


    // test Jonh
    cout << endl;
    cout << "source.c_str() " << source.c_str() << endl;
    cout << "target.c_str() " << target.c_str() << endl;
    cout << endl;

    wsm.setSource(source.c_str());
    wsm.setTarget(target.c_str());
    wsm.setSenderPos(curPosition);
    wsm.setSerial(2);
    wsm.setSummaryVector(false);
    string data = "WSMData generated by ";
    data += findHost()->getFullName();
    wsm.setWsmData(data.c_str());
    wsm.setLocalMessageIdentificaton(to_string(mfcv_epidemic::messageId).c_str());
    wsm.setGlobalMessageIdentificaton((to_string(mfcv_epidemic::messageId) + to_string(MACToInteger())).c_str());
    wsm.setHopCount(hopCount);
    mfcv_epidemic::messageId++;

    //Putting the message in the mfcv_epidemicLocalMessageBuffer
    //mfcv_epidemicLocalMessageBuffer.insert(MyPairMfcv_EpidemicMessageBuffer(to_string(MACToInteger()),wsm));
    mfcv_epidemicLocalMessageBuffer.insert(MyPairMfcv_EpidemicMessageBuffer(wsm.getGlobalMessageIdentificaton(),wsm));
    //Putting the message key in the mfcv_epidemicLocalSummaryVector
    //mfcv_epidemicLocalSummaryVector.insert(make_pair<string, bool>(to_string(MACToInteger()),true));
    mfcv_epidemicLocalSummaryVector.insert(make_pair<string, bool>(wsm.getGlobalMessageIdentificaton(),true));
    //FIFO strategy to set the maximum size that a node is willing to allocate mfcv_epidemic messages in its buffer
    queueFIFO.push(wsm.getGlobalMessageIdentificaton());

    //printQueueFIFO(queueFIFO);
    //printMfcv_EpidemicLocalMessageBuffer();
    //printMfcv_EpidemicLocalSummaryVectorData();

}

void mfcv_epidemic::printQueueFIFO(queue<string> qFIFO){
    int i = 0;
    while(!qFIFO.empty()){
        cout << "I'm " << findHost()->getFullName() << " - queueFIFO Element " << ++i << ": " << qFIFO.front() << endl;
        qFIFO.pop();
    }
}

void mfcv_epidemic::printMfcv_EpidemicLocalMessageBuffer(){
    if(mfcv_epidemicLocalMessageBuffer.empty()){
           cout << "Mfcv_EpidemicLocalMessageBuffer from " << findHost()->getFullName() << " is empty now " << endl;
    }
    else{
        int i = 0;
        cout << "Printing the mfcv_epidemicLocalMessageBuffer from " << findHost()->getFullName() << "(" << MACToInteger() <<"):" << endl;
        for(auto& x: mfcv_epidemicLocalMessageBuffer){
            WaveShortMessage wsmBuffered = x.second;
            cout << " Key " << ++i << ": " << x.first << " - Message Content: " << wsmBuffered.getWsmData() << " source: " << wsmBuffered.getSource() << " target: " << wsmBuffered.getTarget() << " Timestamp: " << wsmBuffered.getTimestamp() << " HopCount: " << wsmBuffered.getHopCount() << endl;
        }
    }
}

void mfcv_epidemic::printMfcv_EpidemicLocalSummaryVectorData(){
    if(mfcv_epidemicLocalSummaryVector.empty()){
           cout << "Mfcv_EpidemicLocalSummaryVector from " << findHost()->getFullName() << " is empty now " << endl;
    }
    else{
        ostringstream ss;
        for(auto& x: mfcv_epidemicLocalSummaryVector)
           ss << x.first << "|" << x.second << "|";
        string s = ss.str();
        s = s.substr(0, s.length() - 1);
        cout << "Mfcv_EpidemicLocalSummaryVector from " << findHost()->getFullName() << "(" << MACToInteger() << "): " <<  s  << endl;
    }
}

void mfcv_epidemic::printMfcv_EpidemicRemoteSummaryVectorData(){
    if(mfcv_epidemicRemoteSummaryVector.empty()){
           cout << "Mfcv_EpidemicRemoteSummaryVector from " << findHost()->getFullName() << " is empty now " << endl;
    }
    else{
        ostringstream ss;
        for(auto& x: mfcv_epidemicRemoteSummaryVector)
            ss << x.first << "|" << x.second << "|";
        string s = ss.str();
        s = s.substr(0, s.length() - 1);
        cout << "Mfcv_EpidemicRemoteSummaryVector from " << findHost()->getFullName() << ": " <<  s  << endl;
    }
}

void mfcv_epidemic::printMfcv_EpidemicRequestMessageVector(){

    if(mfcv_epidemicRequestMessageVector.empty()){
        cout << "Mfcv_EpidemicRequestMessageVector from " << findHost()->getFullName() << " is empty now " << endl;
    }
    else{
        ostringstream ss;
        for(auto& x: mfcv_epidemicRequestMessageVector)
            ss << x.first << "|" << x.second << "|";
        string s = ss.str();
        s = s.substr(0, s.length() - 1);
        cout << "Mfcv_EpidemicRequestMessageVector from " << findHost()->getFullName() << ": " <<  s  << endl;
    }
}

void mfcv_epidemic::printNodesIRecentlySentSummaryVector(){
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

void mfcv_epidemic::printWaveShortMessage(WaveShortMessage wsm){
    cout << "wsm.getName():" << wsm.getName() << endl;
    cout << "wsm.getBitLength():" << wsm.getBitLength() << endl;
    cout << "wsm.getChannelNumber():" << wsm.getChannelNumber() << endl;
    cout << "wsm.getPsid():" << wsm.getPsid() << endl;
    cout << "wsm.getPriority():" << wsm.getPriority() << endl;
    cout << "wsm.getWsmVersion():" << wsm.getWsmVersion() << endl;
    cout << "wsm.getTimestamp():" << wsm.getTimestamp() << endl;
    cout << "wsm.getSenderAddress():" << wsm.getSenderAddress() << endl;
    cout << "wsm.getHeading() " << wsm.getHeading() << endl;
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

void mfcv_epidemic::printWaveShortMessage(WaveShortMessage* wsm){
    cout << "wsm->getName()" << wsm->getName() << endl;
    cout << "wsm->getBitLength()" << wsm->getBitLength() << endl;
    cout << "wsm->getChannelNumber()" << wsm->getChannelNumber() << endl;
    cout << "wsm->getPsid()" << wsm->getPsid() << endl;
    cout << "wsm->getPriority()" << wsm->getPriority() << endl;
    cout << "wsm->getWsmVersion()" << wsm->getWsmVersion() << endl;
    cout << "wsm->getTimestamp()" << wsm->getTimestamp() << endl;
    cout << "wsm->getSenderAddress()" << wsm->getSenderAddress() << endl;
    cout << "wsm->getHeading() " << wsm->getHeading() << endl;
    cout << "wsm->getRecipientAddress()" << wsm->getRecipientAddress() << endl;
    cout << "wsm->getSource()" << wsm->getSource() << endl;
    cout << "wsm->getTarget()" << wsm->getTarget() << endl;
    cout << "wsm->getSenderPos()" << wsm->getSenderPos() << endl;
    cout << "wsm->getSerial()" << wsm->getSerial() << endl;
    cout << "wsm->getSummaryVector()" << wsm->getSummaryVector() << endl;
    cout << "wsm->getRequestMessages()" << wsm->getRequestMessages() << endl;
    cout << "wsm->getWsmData()" << wsm->getWsmData() << endl;
}

void mfcv_epidemic::finish(){
    cout << "Number of Messages Generated: " << mfcv_epidemic::messageId << endl;
    recordScalar("#numMessageGenerated", mfcv_epidemic::messageId);

    // test Jonh
    printMfcv_EpidemicLocalMessageBufferOnFile();

    cout << endl << endl;
    cout << "findHost()->getName() " << findHost()->getName() << endl;
    cout << "findHost()->getFullName() " << findHost()->getFullName() << endl;
    cout << "Angle in radians " <<  traci->getAngleRad() << endl;
    cout << "Angle in degrees: " << ((traci->getAngleRad() * 180) / M_PI) << endl;
    cout << "findHost()->getDisplayString() " << findHost()->getDisplayString() << endl;
    cout << "traci->getCurrentPosition() " << traci->getCurrentPosition() << endl;
    cout << "traci->getDescriptor() " << traci->getDescriptor() << endl;
    cout << "traci->getExternalId() " << traci->getExternalId() << endl;
    cout << "traci->getId() " << traci->getId() << endl;

    //Converting from degrees to radians
    // radians = ( degrees * pi ) / 180 ;
    //Converting from radians to degrees
    // degrees=( radians * 180 ) / pi ;

    //    ← = -3.14159
    //    → = 0
    //    ↓ = -1.5708
    //    ↑ = 1.5708
    //math.la.asu.edu/~nbrewer/Spring2006/Unit%20Circle%20Diagram_files/unit_circle.gif
    //math10.com/en/geometry/angles/measure/angles-and-measurement.html

}


// test Jonh

//This function is for record beacon on file
void mfcv_epidemic::recordOnFile(WaveShortMessage* wsm){

    //Open file for just apeend
    myfile.open ("results/onBeacon_veh.txt", std::ios_base::app);

    //Send "strings" to be saved on the file onBeacon_veh.txt
    myfile << "Beacon from " << wsm->getSenderAddress() << " at " << simTime();
    myfile << " to " << wsm->getRecipientAddress() << endl;
    myfile << "wsm->getName() " << wsm->getName() << endl;
    myfile << "wsm->getBitLength() " << wsm->getBitLength() << endl;
    myfile << "wsm->getChannelNumber() " << wsm->getChannelNumber() << endl;
    myfile << "wsm->getPsid() " << wsm->getPsid() << endl;
    myfile << "wsm->getPriority() " << wsm->getPriority() << endl;
    myfile << "wsm->getWsmVersion() " << wsm->getWsmVersion() << endl;
    myfile << "wsm->getTimestamp() " << wsm->getTimestamp() << endl;
    myfile << "wsm->getSenderAddress() " << wsm->getSenderAddress() << endl;
    myfile << "wsm->getHeading() " << wsm->getHeading() << endl;
    myfile << "wsm->getRecipientAddress() " << wsm->getRecipientAddress() << endl;
    myfile << "wsm->getSource() " << wsm->getSource() << endl;
    myfile << "wsm->getTarget() " << wsm->getTarget() << endl;
    myfile << "wsm->getSenderPos() " << wsm->getSenderPos() << endl;
    myfile << "wsm->getSerial() " << wsm->getSerial() << endl;
    myfile << "wsm->getSummaryVector() " << wsm->getSummaryVector() << endl;
    myfile << "wsm->getRequestMessages() " << wsm->getRequestMessages() << endl;
    myfile << "wsm->getWsmData() " << wsm->getWsmData() << endl;
    myfile << endl;
    myfile.close();
}

void mfcv_epidemic::printMfcv_EpidemicLocalMessageBufferOnFile(){

    //Open file for just apeend
    myfile.open ("results/LocalMessageBuffer_veh.txt", std::ios_base::app);

    //Send "strings" to be saved on the file onBeacon_veh.txt
    if(mfcv_epidemicLocalMessageBuffer.empty()){
        //cout << "Mfcv_EpidemicLocalMessageBuffer from " << findHost()->getFullName() << " is empty now " << endl;
        myfile << "Mfcv_EpidemicLocalMessageBuffer from " << findHost()->getFullName() << " is empty now " << endl;
    }
    else{
        int i = 0;
        //cout << "Printing the mfcv_epidemicLocalMessageBuffer from " << findHost()->getFullName() << "(" << MACToInteger() <<"):" << endl;
        myfile << "Printing the mfcv_epidemicLocalMessageBuffer from " << findHost()->getFullName() << "(" << MACToInteger() <<"):" << endl;
        for(auto& x: mfcv_epidemicLocalMessageBuffer){
            WaveShortMessage wsmBuffered = x.second;
            //cout << " Key " << ++i << ": " << x.first << " - Message Content: " << wsmBuffered.getWsmData() << " source: " << wsmBuffered.getSource() << " target: " << wsmBuffered.getTarget() << " Timestamp: " << wsmBuffered.getTimestamp() << " HopCount: " << wsmBuffered.getHopCount() << endl;
            myfile << " Key " << ++i << ": " << x.first << " - Message Content: " << wsmBuffered.getWsmData() << " source: " << wsmBuffered.getSource() << " target: " << wsmBuffered.getTarget() << " Timestamp: " << wsmBuffered.getTimestamp() << " HopCount: " << wsmBuffered.getHopCount() << endl;
        }
    }
    myfile << endl;
    myfile.close();
}

WaveShortMessage* mfcv_epidemic::prepareWSM_mfcv_epidemic(std::string name, int lengthBits, t_channel channel, int priority, unsigned int rcvId, int serial) {
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
    //wsm->setSource(source);
    //wsm->setTarget(target);
    wsm->setSenderPos(curPosition);
    wsm->setSerial(serial);
    wsm->setRoadId(traci->getRoadId().c_str());
    wsm->setSenderSpeed(traci->getSpeed());
    wsm->setVehicleId(traci->getId());

    // ver como definir o id, traci->getId(), e a categoria
    if (traci->getId() < 5) {
        wsm->setCategory(1);
    }
    else if (traci->getId() > 5) {
         wsm->setCategory(2);
    }
    else if (traci->getId() < 10) {
             wsm->setCategory(4);
    }
    else {
        wsm->setCategory(4);
    }

    wsm->setHeading(getHeading());
    //wsm->setHeading(((traci->getAngleRad() * 180) / M_PI));
    cout << endl << endl;
    cout << "In prepareWSM_mfcv_epidemic3" << endl;
    cout << "findHost()->getDisplayString() " << findHost()->getDisplayString() << endl;
    cout << "Angle radians " << ((traci->getAngleRad() * 180) / M_PI) << endl;
    cout << "Heading " << traci->getAngleRad() << endl;
    cout << "Angle degree (getAngle) " << getHeading() << endl;
    cout << "Angle in degrees (M_PI): " << ((traci->getAngleRad() * 180) / M_PI) << endl;
    cout << "Angle in degrees (2*M_PI): " << (((traci->getAngleRad() + 2*M_PI ) * 180)/ M_PI) << endl;
    cout << endl << endl;

    if (name == "beacon") {
        DBG << "Creating Beacon with Priority " << priority << " at Applayer at " << wsm->getTimestamp() << std::endl;
    }else if (name == "data") {
        DBG << "Creating Data with Priority " << priority << " at Applayer at " << wsm->getTimestamp() << std::endl;
    }
    return wsm;
}

unsigned int mfcv_epidemic::getHeading(){
    // return angle <0º - 359º>

    // marcospaiva.com.br/images/rosa_dos_ventos%2002.GIF
    // marcospaiva.com.br/localizacao.htm

    // (angle >= 337.5 && angle < 360) || (angle >= 0 && angle < 22.5) return 1; // L or E => 0º
    // angle >= 22.5 && angle < 67.5                                   return 2; // NE => 45º
    // angle >= 67.5  && angle < 112,5                                 return 3; // N => 90º
    // angle >= 112.5  && angle < 157.5                                return 4; // NO => 135º
    // angle >= 157,5  && angle < 202,5                                return 5; // O or W => 180º
    // angle >= 202.5  && angle < 247.5                                return 6; // SO => 225º
    // angle >= 292.5  && angle < 337.5                                return 8; // SE => 315º
    // angle >= 360 return 9; // Error

    double angle;
    if (traci->getAngleRad() < 0) // radians are negtive, so degrees negative
        angle = (((traci->getAngleRad() + 2*M_PI ) * 180)/ M_PI);
    else //radians are positive, so degrees positive
        angle = ((traci->getAngleRad() * 180) / M_PI);

    if ((angle >= 337.5 && angle < 360) || (angle >= 0 && angle < 22.5)){
        cout << "Angle: " << angle << " Heading " << 1 << endl;
        return 1; // L or E => 0º
    }
    else if (angle >= 22.5 && angle < 67.5) {
        cout << "Angle (getHeading): " << angle << " Heading " << 2 << endl;
        return 2; // NE => 45º
    }
    else if (angle >= 67.5  && angle < 112.5) {
        cout << "Angle (getHeading): " << angle << " Heading " << 3 << endl;
        return 3; // N => 90º
    }
    else if (angle >= 112.5  && angle < 157.5) {
        cout << "Angle (getHeading): " << angle << " Heading " << 4 << endl;
        return 4; // NO => 135º
    }
    else if (angle >= 157.5  && angle < 202.5) {
        cout << "Angle (getHeading): " << angle << " Heading " << 5 << endl;
        return 5; // O or W => 180º
    }
    else if (angle >= 202.5  && angle < 247.5) {
        cout << "Angle (getHeading): " << angle << " Heading " << 6 << endl;
        return 6; // SO => 225º
    }
    else if (angle >= 247.5  && angle < 292.5) {
        cout << "Angle (getHeading): " << angle << " Heading " << 7 << endl;
        return 7; // S => 270º
    }
    else if (angle >= 292.5  && angle < 337.5) {
        cout << "Angle (getHeading): " << angle << " Heading " << 8 << endl;
        return 8; // SE => 315º
    }
    else {
        cout << "Error angle (getHeading): " << angle << " Heading of error " << 9 << endl;
        return 9; // Error
    }
}

void mfcv_epidemic::handleSelfMsg(cMessage* msg) {
    switch (msg->getKind()) {
//        case SEND_BEACON_EVT: {
//            sendWSM(prepareWSM("beacon", beaconLengthBits, type_CCH, beaconPriority, 0, -1));
//            scheduleAt(simTime() + par("beaconInterval").doubleValue(), sendBeaconEvt);
//            break;
//        }
//        case SEND_BEACON_EVT_minicurso: {
//            sendWSM(prepareWSM("beacon_minicurso", beaconLengthBits, type_CCH, beaconPriority, 0, -1));
//            scheduleAt(simTime() + par("beaconInterval").doubleValue(), sendBeaconEvt);
//            break;
//        }
//        case SEND_BEACON_EVT_epidemic: {
//            //prepareWSM(std::string name, int lengthBits, t_channel channel, int priority, int rcvId, int serial)
//            //I our implementation, if rcvId = BROADCAST then we are broadcasting beacons. Otherwise, this parameter must be instantiated with the receiver address
//            sendWSM(prepareWSM_epidemic("beacon", beaconLengthBits, type_CCH, beaconPriority, BROADCAST, -1));
//            scheduleAt(simTime() + par("beaconInterval").doubleValue(), sendBeaconEvt);
//            break;
//        }
        case SEND_BEACON_EVT_mfcv_epidemic: {
            //prepareWSM(std::string name, int lengthBits, t_channel channel, int priority, int rcvId, int serial)
            //I our implementation, if rcvId = BROADCAST then we are broadcasting beacons. Otherwise, this parameter must be instantiated with the receiver address
            sendWSM(prepareWSM_mfcv_epidemic("beacon", beaconLengthBits, type_CCH, beaconPriority, BROADCAST, -1));
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
