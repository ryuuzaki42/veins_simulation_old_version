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

#include "application/mfcv_epidemic/mfcv_epidemic_rsu.h"

using Veins::AnnotationManagerAccess;

Define_Module(mfcv_epidemic_rsu);

void mfcv_epidemic_rsu::initialize(int stage) {
    BaseWaveApplLayer::initialize_mfcv_epidemic(stage);
    if (stage == 0) {
        mobi = dynamic_cast<BaseMobility*> (getParentModule()->getSubmodule("mobility"));
        ASSERT(mobi);
        annotations = AnnotationManagerAccess().getIfExists();
        ASSERT(annotations);

        //To record some statistics about the simulation
        //hopCountStats.setName("hopCountStats"); //Histogram
        //hopCountStats.setRangeAutoUpper(0, 10, 1.5); //Histogram
        //hopCountVector.setName("HopCount");
        //messageArrivalTimeStats.setName("messageArrivalStats"); //Histogram
        //messageArrivalTimeStats.setRangeAutoUpper(0, 10, 1.5); //Histogram
        //messageArrivalTimeVector.setName("messageArrivalVector");
        //numMessageReceived = 0;

        std::cout << "I'm " << findHost()->getFullName() <<  " myMac: " << myMac << " MACToInteger: " << MACToInteger() << endl;
    }
}

void mfcv_epidemic_rsu::onBeacon(WaveShortMessage* wsm) {

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

void mfcv_epidemic_rsu::onData(WaveShortMessage* wsm) {
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
          }else if(mfcv_epidemicRequestMessageVector.empty()){
              //cout << "Mfcv_EpidemicRequestMessageVector from " << findHost()->getFullName() << " is empty now " << endl;
              //changing the turn of the anti-entropy session. In this case, I have not found any differences between Mfcv_EpidemicRemoteSummaryVector and Mfcv_EpidemicLocalSummaryVector but I need to change the round of anti-entropy session
              sendLocalSummaryVector(wsm->getSenderAddress());
          }else{
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
        //is data content
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
                                bubble("message added in my buffer"); //making animation message more informative
                                //recording some statistics
//                                cout << "findHost()->getFullName() == w.getTarget(): " << " findHost()->getFullName(): " << findHost()->getFullName() << " w.getTarget():" << w.getTarget() << endl;
                                if(strcmp(findHost()->getFullName(), w.getTarget()) == 0){
                                    //hopCountVector.record(hopCount - w.getHopCount());
                                    //hopCountStats.collect(hopCount - w.getHopCount());
                                    //messageArrivalTimeVector.record((simTime() - w.getTimestamp()));
                                    //messageArrivalTimeStats.collect((simTime() - w.getTimestamp()));
                                    //numMessageReceived++;

                                   // emit(delayToDeliverSignal, (simTime() - w.getTimestamp()));
                                   // emit(hopsToDeliverSignal, (hopCount - w.getHopCount()));
                                   // emit(messageArrivalSignal, 1);

                                    if(maiortempo < (simTime() - w.getTimestamp()).dbl())
                                        maiortempo = (simTime() - w.getTimestamp()).dbl();
//                                    cout << "numMessageReceived: " << numMessageReceived << endl;
                                }
                            }
                            //An entry in the unordered_map was found
                            else{
                                //do nothing because the message is already in my mfcv_epidemicLocalBuffer
                            }
                        }
                        //The maximum buffer size was reached, so I have to remove the first item from the queueFIFO
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
                                //recording some statistics
//                                cout << "findHost()->getFullName() == w.getTarget(): " << " findHost()->getFullName(): " << findHost()->getFullName() << " w.getTarget():" << w.getTarget() << endl;
                                if(strcmp(findHost()->getFullName(), w.getTarget()) == 0){
                                    //hopCountVector.record(hopCount - w.getHopCount());
                                    //hopCountStats.collect(hopCount - w.getHopCount());
                                    //messageArrivalTimeVector.record((simTime() - w.getTimestamp()));
                                    //messageArrivalTimeStats.collect((simTime() - w.getTimestamp()));
                                    cout << "Removing an item in my buffer because the size of it was reached" << endl;
                                    //cout << "numMessageReceived++: " << numMessageReceived++ << endl;
                                    //numMessageReceived++;
                                    //emit(arrivalSignal, (hopCount - w.getHopCount()));
                                    //emit(arrivalSignal, (simTime() - w.getTimestamp()));
                                    if(maiortempo < (simTime() - w.getTimestamp()).dbl())
                                        maiortempo = (simTime() - w.getTimestamp()).dbl();
//                                    cout << "numMessageReceived: " << numMessageReceived << endl;
                                }
                            }
                            //An entry in the unordered_map was found
                            else{
                                //do nothing because the message is already in my mfcv_epidemicLocalBuffer
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
             //std::cout << findHost()->getFullName() << " will cache the message for forwarding it later." << " at " << simTime() << std::endl;
        }
    }
}

void mfcv_epidemic_rsu::sendBeacon() {
    t_channel channel = dataOnSch ? type_SCH : type_CCH;
    std::cout << "Channel: " << channel << std::endl;
    WaveShortMessage* wsm = prepareWSM_mfcv_epidemic("beacon", dataLengthBits, channel, dataPriority, BROADCAST,2);
    sendWSM(wsm);
}

void mfcv_epidemic_rsu::sendWSM(WaveShortMessage* wsm) {
    sendDelayedDown(wsm,individualOffset);
}

//Method used to initiate the anti-entropy session sending the mfcv_epidemicLocalSummaryVector
void mfcv_epidemic_rsu::sendLocalSummaryVector(unsigned int newRecipientAddress) {
    t_channel channel = dataOnSch ? type_SCH : type_CCH;
    WaveShortMessage* wsm = prepareWSM_mfcv_epidemic("data", dataLengthBits, channel, dataPriority, newRecipientAddress,2);
    wsm->setSummaryVector(true);
    wsm->setRequestMessages(false);
    //Put the summary vector here, on data wsm field
    wsm->setWsmData(getLocalSummaryVectorData().c_str());
    //Sending the summary vector
    sendWSM(wsm);
}

void mfcv_epidemic_rsu::sendMessagesRequested(string s, unsigned int recipientAddress) {

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

          }else{
               w = got->second;
               //WaveShortMessage w = getMfcv_EpidemicLocalMessageBuffer(s.substr(0, pos));
               //Verifying if I'm still able to spread the message or not. If w.getHopCount == 1 I'm able to send the message only to its target
               if(w.getHopCount() > 1){
                 ss << s.substr(0, pos) << "|" << w.getWsmData() << "|" << w.getSource() << "|" << w.getTarget() << "|" << w.getTimestamp() << "|" << w.getHopCount() - 1 << "|";
               }else if(w.getHopCount() == 1){
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

//void mfcv_epidemic_rsu::sendMessagesRequested(string s, unsigned int recipientAddress) {
//
//    cout << "I'm " << findHost()->getFullName() << "(" << MACToInteger() << "). Sending the following messages resquested: " << s << " to " << recipientAddress << endl;
//    t_channel channel = dataOnSch ? type_SCH : type_CCH;
//    WaveShortMessage* wsm = prepareWSM_mfcv_epidemic("data", dataLengthBits, channel, dataPriority, recipientAddress,2);
//    string delimiter = "|";
//    size_t pos = 0;
//    string token;
//    unsigned i = 0;
//    string message="";
//
//    printMfcv_EpidemicLocalMessageBuffer();
//
//    //Extracting who request message
//    pos = s.find(delimiter);
//    string tokenRequester = s.substr(0, pos);
//    s.erase(0, pos + delimiter.length());
//
//    while ((pos = s.find(delimiter)) != std::string::npos) {
//      //Catch jus the key of the local summary vector
//      if(i%2 == 0){
//          ostringstream ss;
//          //cout << "i = " << i << " - looking for the message key(s.substr(0, pos)): " << s.substr(0, pos) << endl;
//          WaveShortMessage w = getMfcv_EpidemicLocalMessageBuffer(s.substr(0, pos));
//          //Verifying if I'm still able to spread the message or not. If w.getHopCount == 1 I'm able to send the message only to its target
//          if(w.getHopCount() > 1){
//            ss << s.substr(0, pos) << "|" << w.getWsmData() << "|" << w.getSource() << "|" << w.getTarget() << "|" << w.getTimestamp() << "|" << w.getHopCount() - 1 << "|";
//          }
//          else if(w.getHopCount() == 1){
//            if((strcmp(tokenRequester.c_str(),w.getTarget()) == 0)){
//              ss << s.substr(0, pos) << "|" << w.getWsmData() << "|" << w.getSource() << "|" << w.getTarget() << "|" << w.getTimestamp() << "|" << w.getHopCount() - 1 << "|";
//            }
//          }
//          else{
//              //when w.getHopCount == 0. I'm not able to send the message to other nodes
//              //do nothing
//          }
//
//          //cout << "ss.str() inside the sendMessageRequested method: " << ss.str() << endl;
//          message += ss.str();
//          //cout << "message inside the sendMessageRequested method. message += ss.str(): " << message << endl;
//      }
//      //cout << "i = " << i << " - s inside the sendMessageRequested method: " << s << endl;
//      s.erase(0, pos + delimiter.length());
//      //cout << "i = " << i << " - s inside the sendMessageRequested method, after s.erase(0, pos + delimiter.length()): " << s << endl;
//      i++;
//    }
//    message = message.substr(0, message.length() - 1);
//    cout << "Message that is sending as a result of a requisition vector request. wsm->setWsmData: " <<  message << endl;
//    wsm->setWsmData(message.c_str());
//    wsm->setSummaryVector(false);
//    wsm->setRequestMessages(false);
//    sendWSM(wsm);
//}

void mfcv_epidemic_rsu::sendMfcv_EpidemicRequestMessageVector(unsigned int newRecipientAddress) {
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

void mfcv_epidemic_rsu::createMfcv_EpidemicRemoteSummaryVector(string s){
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

void mfcv_epidemic_rsu::createMfcv_EpidemicRequestMessageVector(){
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

string mfcv_epidemic_rsu::getMfcv_EpidemicRequestMessageVectorData(){
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

//WaveShortMessage mfcv_epidemic_rsu::getMfcv_EpidemicLocalMessageBuffer(string s){
//    //cout << "Getting the mfcv_epidemicLocalMessageBuffer[" << s << "] from " << findHost()->getFullName() << endl;
//    unordered_map<string,WaveShortMessage>::const_iterator got = mfcv_epidemicLocalMessageBuffer.find(s.c_str());
//    return got->second;
//}

//Method used to convert the unordered_map mfcv_epidemicLocalSummaryVectorData in a string
string mfcv_epidemic_rsu::getLocalSummaryVectorData(){
    ostringstream ss;
    for(auto& x: mfcv_epidemicLocalSummaryVector)
        ss << x.first << "|" << x.second << "|";
    string s = ss.str();
    s = s.substr(0, s.length() - 1);
    //cout << "Mfcv_EpidemicLocalSummaryVector from " << findHost()->getFullName() << "(" << MACToInteger() << "): " <<  s  << endl;
    return s.c_str();
}

void mfcv_epidemic_rsu::printQueueFIFO(queue<string> qFIFO){
    int i = 0;
    while(!qFIFO.empty()){
        cout << "I'm " << findHost()->getFullName() << " - queueFIFO Element " << ++i << ": " << qFIFO.front() << endl;
        qFIFO.pop();
    }
}

void mfcv_epidemic_rsu::printMfcv_EpidemicLocalMessageBuffer(){
    if(mfcv_epidemicLocalMessageBuffer.empty()){
           cout << "Mfcv_EpidemicLocalMessageBuffer from " << findHost()->getFullName() << " is empty now " << endl;
    }else{
        int i = 0;
        cout << "Printing the mfcv_epidemicLocalMessageBuffer from " << findHost()->getFullName() << "(" << MACToInteger() <<"):" << endl;
        for(auto& x: mfcv_epidemicLocalMessageBuffer){
            WaveShortMessage wsmBuffered = x.second;
            cout << " Key " << ++i << ": " << x.first << " - Message Content: " << wsmBuffered.getWsmData() << " source: " << wsmBuffered.getSource() << " target: " << wsmBuffered.getTarget() << " Timestamp: " << wsmBuffered.getTimestamp() << " HopCount: " << wsmBuffered.getHopCount() << endl;
        }
    }
}

void mfcv_epidemic_rsu::printMfcv_EpidemicLocalSummaryVectorData(){
    if(mfcv_epidemicLocalSummaryVector.empty()){
           cout << "Mfcv_EpidemicLocalSummaryVector from " << findHost()->getFullName() << " is empty now " << endl;
    }else{
        ostringstream ss;
        for(auto& x: mfcv_epidemicLocalSummaryVector)
           ss << x.first << "|" << x.second << "|";
        string s = ss.str();
        s = s.substr(0, s.length() - 1);
        cout << "Mfcv_EpidemicLocalSummaryVector from " << findHost()->getFullName() << "(" << MACToInteger() << "): " <<  s  << endl;
    }
}

void mfcv_epidemic_rsu::printMfcv_EpidemicRemoteSummaryVectorData(){
    if(mfcv_epidemicRemoteSummaryVector.empty()){
           cout << "Mfcv_EpidemicRemoteSummaryVector from " << findHost()->getFullName() << " is empty now " << endl;
    }else{
        ostringstream ss;
        for(auto& x: mfcv_epidemicRemoteSummaryVector)
            ss << x.first << "|" << x.second << "|";
        string s = ss.str();
        s = s.substr(0, s.length() - 1);
        cout << "Mfcv_EpidemicRemoteSummaryVector from " << findHost()->getFullName() << ": " <<  s  << endl;
    }
}

void mfcv_epidemic_rsu::printMfcv_EpidemicRequestMessageVector(){

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

void mfcv_epidemic_rsu::printNodesIRecentlySentSummaryVector(){
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

void mfcv_epidemic_rsu::printWaveShortMessage(WaveShortMessage wsm){
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

void mfcv_epidemic_rsu::printWaveShortMessage(WaveShortMessage* wsm){
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

void mfcv_epidemic_rsu::finish(){
//    // This function is called by OMNeT++ at the end of the simulation.
//    cout << "Number of Messages Received: " << numMessageReceived << endl;
//    cout << "Hop count, min:    " << hopCountStats.getMin() << endl;
//    cout << "Hop count, max:    " << hopCountStats.getMax() << endl;
//    cout << "Hop count, mean:   " << hopCountStats.getMean() << endl;
//    cout << "Hop count, stddev: " << hopCountStats.getStddev() << endl;
//
//    // This function is called by OMNeT++ at the end of the simulation.
//    cout << "Maior Delay: " << maiortempo << endl;
//    cout << "messageArrivalTimeStats, min:    " << messageArrivalTimeStats.getMin() << endl;
//    cout << "messageArrivalTimeStats, max:    " << messageArrivalTimeStats.getMax() << endl;
//    cout << "messageArrivalTimeStats, mean:   " << messageArrivalTimeStats.getMean() << endl;
//    cout << "messageArrivalTimeStats, stddev: " << messageArrivalTimeStats.getStddev() << endl;
//
//
//    // This function is called by OMNeT++ at the end of the simulation.
//    EV << "Number of Messages Received: " << numMessageReceived << endl;
//    EV << "Hop count, min:    " << hopCountStats.getMin() << endl;
//    EV << "Hop count, max:    " << hopCountStats.getMax() << endl;
//    EV << "Hop count, mean:   " << hopCountStats.getMean() << endl;
//    EV << "Hop count, stddev: " << hopCountStats.getStddev() << endl;
//
//    // This function is called by OMNeT++ at the end of the simulation.
//    EV << "Maior Delay: " << maiortempo << endl;
//    EV << "messageArrivalTimeStats, min:    " << messageArrivalTimeStats.getMin() << endl;
//    EV << "messageArrivalTimeStats, max:    " << messageArrivalTimeStats.getMax() << endl;
//    EV << "messageArrivalTimeStats, mean:   " << messageArrivalTimeStats.getMean() << endl;
//    EV << "messageArrivalTimeStats, stddev: " << messageArrivalTimeStats.getStddev() << endl;
//
//    recordScalar("#numMessageReceived", numMessageReceived);
//
//    hopCountStats.recordAs("hop CCount");
//    messageArrivalTimeStats.recordAs("Latency to Delivery Message");
}
