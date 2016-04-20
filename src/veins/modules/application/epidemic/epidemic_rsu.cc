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

#include "veins/modules/application/epidemic/epidemic_rsu.h"

using Veins::AnnotationManagerAccess;

Define_Module(epidemic_rsu);

void epidemic_rsu::initialize(int stage) {
    BaseWaveApplLayer::initialize_epidemic(stage);
    if (stage == 0) {
        mobi = dynamic_cast<BaseMobility*> (getParentModule()->getSubmodule("mobility"));
        ASSERT(mobi);
        annotations = AnnotationManagerAccess().getIfExists();
        ASSERT(annotations);
        sentMessage = false;

        //To record some statistics about the simulation
        //hopCountStats.setName("hopCountStats"); //Histogram
        //hopCountStats.setRangeAutoUpper(0, 10, 1.5); //Histogram
        //hopCountVector.setName("HopCount");
        //messageArrivalTimeStats.setName("messageArrivalStats"); //Histogram
        //messageArrivalTimeStats.setRangeAutoUpper(0, 10, 1.5); //Histogram
        //messageArrivalTimeVector.setName("messageArrivalVector");
        //numMessageReceived = 0;

        std::cout << "I'm " << findHost()->getFullName() <<  " myMac: " << myMac << " MACToInteger: " << MACToInteger() << endl;
        epidemic_InitializeVariables();
    }
}

void epidemic_rsu::epidemic_InitializeVariables(){
    source = findHost()->getFullName();
    stringTmp = ev.getConfig()->getConfigValue("seed-set");
    repeatNumber = atoi(stringTmp.c_str()); // number of execution (${repetition})

    experimentNumber = par("experimentNumber");

    stringTmp = "results/epidemic_resultsEnd/E" + to_string(experimentNumber);
    stringTmp += "_" + to_string((static_cast<int>(ttlBeaconMessage))) + "_" + to_string(countGenerateBeaconMessage) +"/";

    fileMessagesBroadcast = fileMessagesUnicast = fileMessagesCount = stringTmp + findHost()->getFullName();

    fileMessagesBroadcast += "_Broadcast_Messages.r";
    fileMessagesUnicast += "_Messages_Received.r";
    fileMessagesCount += "_Count_Messages_Received.r";

    //fileMessagesDrop and fileMessagesGenerated not used yet to RSU

    if ((myId == 0) && (repeatNumber == 0)) { //Open a new file (blank)
        if (experimentNumber == 1) { // maxSpeed 15 m/s
            string comand = "sed -i 's/maxSpeed=.* color/maxSpeed=\"15\" color/g' vehDist.rou.xml";
            system(comand.c_str());
            cout << endl << "Change the spped to 15 m/s, command: " << comand << endl;
        } else if (experimentNumber == 5){  // maxSpeed 25 m/s
            string comand = "sed -i 's/maxSpeed=.* color/maxSpeed=\"25\" color/g' vehDist.rou.xml";
            system(comand.c_str());
            cout << endl << "Change the spped to 25 m/s, command: " << comand << endl;
        }

        stringTmp = "mkdir -p " + stringTmp + " > /dev/null";
        cout << endl << "Created the folder, command: \"" << stringTmp << "\"" << endl;
        cout << "repeatNumber " << repeatNumber << endl;
        system(stringTmp.c_str()); //create a folder results

        openFileAndClose(fileMessagesBroadcast, false, ttlBeaconMessage, countGenerateBeaconMessage);
        openFileAndClose(fileMessagesUnicast, false, ttlBeaconMessage, countGenerateBeaconMessage);
        openFileAndClose(fileMessagesCount, false, ttlBeaconMessage, countGenerateBeaconMessage);
    } else { // (repeatNumber != 0)) // for just append
        openFileAndClose(fileMessagesBroadcast, true, ttlBeaconMessage, countGenerateBeaconMessage);
        openFileAndClose(fileMessagesUnicast, true, ttlBeaconMessage, countGenerateBeaconMessage);
        openFileAndClose(fileMessagesCount, true, ttlBeaconMessage, countGenerateBeaconMessage);
    }
}

void epidemic_rsu::messagesReceivedMeasuring(WaveShortMessage* wsm) {
    string wsmData = wsm->getWsmData();
    simtime_t timeToArrived = (simTime() - wsm->getTimestamp());
    unsigned short int countHops = (beaconMessageHopLimit - wsm->getHopCount());
    map<string, struct messages>::iterator it = messagesReceived.find(wsm->getGlobalMessageIdentificaton());

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
            it->second.wsmData += " & " + wsmData.substr(42);
        }
        // Be aware, don't use the category identification as a value insert in the wsmData in the begin
        it->second.countT += count(wsmData.begin(), wsmData.end(), 'T');
        it->second.countP += count(wsmData.begin(), wsmData.end(), 'P');
    } else{
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

void epidemic_rsu::printCountMessagesReceived() {
    myfile.open (fileMessagesCount, std::ios_base::app);

    if (!messagesReceived.empty()) {
        myfile << "messagesReceived from " << source << endl;

        float avgGeneralHopsMessage = 0;
        SimTime avgGeneralCopyMessageReceived = 0;
        SimTime avgGeneralTimeMessageReceived = 0;

        unsigned short int countP = 0;
        unsigned short int countT = 0;
        map<string, struct messages>::iterator it;
        for (it = messagesReceived.begin(); it != messagesReceived.end(); it++) {
            myfile << endl << "## Message ID: " << it->first << endl;
            myfile << "Count received: " << it->second.copyMessage << endl;
            avgGeneralCopyMessageReceived += it->second.copyMessage;

            myfile << it->second.wsmData << endl;
            myfile << "Hops: " << it->second.hops << endl;
            myfile << "Sum hops: " << it->second.sumHops << endl;
            avgGeneralHopsMessage += it->second.sumHops;
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
        avgGeneralTimeMessageReceived /= messagesReceived.size();
        avgGeneralCopyMessageReceived /= messagesReceived.size();

        myfile << endl << "Exp: " << experimentNumber << " ### Count Messages Received: " << messagesReceived.size() << endl;
        myfile << "Exp: " << experimentNumber << " ### avg time to receive: " << avgGeneralTimeMessageReceived << endl;
        myfile << "Exp: " << experimentNumber << " ### avg copy received: " << avgGeneralCopyMessageReceived << endl;
        myfile << "Exp: " << experimentNumber << " ### avg hops to received: " << avgGeneralHopsMessage << endl;

        myfile << "Exp: " << experimentNumber << " ### Category T general: " << countT << endl;
        myfile << "Exp: " << experimentNumber << " ### Category P general: " << countP << endl << endl;;
    } else {
        myfile << "messagesReceived from " << source << " is empty" << endl;
    }
    myfile.close();
}

void epidemic_rsu::onBeacon(WaveShortMessage* wsm) {
    //cout << "I'm " << findHost()->getFullName() << "(" << MACToInteger() << ") and I received a Beacon from " << wsm->getSenderAddress() << " with destination " << wsm->getRecipientAddress() << endl;

    //Verifying if I have the smaller address in order to start the anti-entropy session sending out my summary vector
    if (wsm->getSenderAddress() > MACToInteger()) { //true means that I have the smaller address
        //cout << "I'm " << findHost()->getFullName() << " and my ID is smaller than the Beacon sender." << endl;
        //Verifying if (current contact simutime - last contact simutime) is bigger than previous slide window defined
        //A new summary vector needs to be re-sent to the same contact node only "sendSummaryVectorInterval" seconds after the last one
        unordered_map<unsigned int,simtime_t>::const_iterator got = nodesIRecentlySentSummaryVector.find(wsm->getSenderAddress());
        if (got == nodesIRecentlySentSummaryVector.end()) { //true value means that there is no entry in the nodesIRecentlySentSummaryVector for the current contact node
            //cout << "I'm " << findHost()->getFullName() << ". Contact not found in nodesIRecentlySentSummaryVector. Sending my summary vector to " << wsm->getSenderAddress() << endl;
            //Sending my summary vector to another mobile node
            sendLocalSummaryVector(wsm->getSenderAddress());
            nodesIRecentlySentSummaryVector.insert(make_pair<unsigned int, simtime_t>(wsm->getSenderAddress(),simTime()));
            printNodesIRecentlySentSummaryVector();
        } else { //An entry in the unordered_map was found
           //cout << "I'm " << findHost()->getFullName() << ". Contact found in nodesIRecentlySentSummaryVector. Node: " << got->first << " added at " << (simTime() - got->second) << " seconds ago." << endl;
           if((simTime() - got->second) > sendSummaryVectorInterval) { //checking if I should update the nodesIRecentlySentSummaryVector entry
               //cout << "I'm " << findHost()->getFullName() << " and I'm updating the entry in the nodesIRecentlySentSummaryVector." << endl;
               sendLocalSummaryVector(wsm->getSenderAddress());
               nodesIRecentlySentSummaryVector.erase(wsm->getSenderAddress());
               nodesIRecentlySentSummaryVector.insert(make_pair<unsigned int, simtime_t>(wsm->getSenderAddress(),simTime()));
               printNodesIRecentlySentSummaryVector();
           }
        }
    } else { //My address is bigger than the Beacon sender
        //do nothing
        //std::cout << "My ID is bigger than the Beacon sender." << std::endl;
    }
}

void epidemic_rsu::onData(WaveShortMessage* wsm) {
    //Verifying the kind of a received message: if a summary vector (true) or a epidemic buffer data message (false).
    if (wsm->getSummaryVector()) {
       //checking if the summary vector was sent to me
       if (wsm->getRecipientAddress() == MACToInteger()) {
          cout << "I'm " << findHost()->getFullName() << "(" << MACToInteger() << ") and I just recieved the summary vector |> " << wsm->getWsmData() << " <| from " << wsm->getSenderAddress() << endl;
          //Creating the remote summary vector with the data received in wsm->message field
          createEpidemicRemoteSummaryVector(wsm->getWsmData());
          printEpidemicRemoteSummaryVectorData();
          printEpidemicLocalSummaryVectorData();
          //Creating a key vector in order to request messages that I still do not have in my buffer
          createEpidemicRequestMessageVector();
          printEpidemicRequestMessageVector();
          //Verifying if this is the end of second round of the anti-entropy session when the EpidemicRemoteSummaryVector and EpidemicLocalSummaryVector are equals
          if ((epidemicRequestMessageVector.empty() ||(strcmp(wsm->getWsmData(),"") == 0)) && (wsm->getSenderAddress() > MACToInteger())) {
             //cout << "EpidemicRequestMessageVector from " << findHost()->getFullName() << " is empty now " << endl;
             //cout << "Or strcmp(wsm->getWsmData(),\"\") == 0) " << endl;
             //cout << "And  wsm->getSenderAddress() > MACToInteger() " << endl;
          } else if(epidemicRequestMessageVector.empty()) {
              //cout << "EpidemicRequestMessageVector from " << findHost()->getFullName() << " is empty now " << endl;
              //changing the turn of the anti-entropy session. In this case, I have not found any differences between EpidemicRemoteSummaryVector and EpidemicLocalSummaryVector but I need to change the round of anti-entropy session
              sendLocalSummaryVector(wsm->getSenderAddress());
          } else {
              //Sending a request vector in order to get messages that I don't have
              sendEpidemicRequestMessageVector(wsm->getSenderAddress());
          }
       }
    } else { //is a data message requisition or a data message content
        //cout << "I'm " << findHost()->getFullName() << ". This is not a summary vector" << endl;
        //Verifying if this is a request message
        if(wsm->getRequestMessages()) {
            //checking if the request vector was sent to me
            if(wsm->getRecipientAddress() == MACToInteger()) {
                //Searching for elements in the epidemicLocalMessageBuffer and sending them to requester
                cout << "I'm " << findHost()->getFullName() << ". I received the epidemicRequestMessageVector |> " << wsm->getWsmData() << " <| from " << wsm->getSenderAddress() << endl;
                sendMessagesRequested(wsm->getWsmData(), wsm->getSenderAddress());
            }
        } else { //is data content
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

                     // leaved to not break the code
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

                        // test Jonh
                        w.setGlobalMessageIdentificaton(wsm->getGlobalMessageIdentificaton());
                        cout << "IDw:" << w.getGlobalMessageIdentificaton() << endl;
                        cout << "IDwsm:" << wsm->getGlobalMessageIdentificaton() << endl;
                        //exit(1);

                        //checking if the maximum buffer size was reached
                        if(queueFIFO.size() < maximumEpidemicBufferSize) {
                            //Verifying if there is no entry for current message received in my epidemicLocalMessageBuffer
                            unordered_map<string,WaveShortMessage>::const_iterator got = epidemicLocalMessageBuffer.find(tokenkey);
                            if(got == epidemicLocalMessageBuffer.end()) { //true value means that there is no entry in the epidemicLocalMessageBuffer for the current message identification
                                //Putting the message in the epidemicLocalMessageBuffer
                                epidemicLocalMessageBuffer.insert(MyPairEpidemicMessageBuffer(tokenkey,w));
                                //Putting the message in the EpidemicLocalSummaryVector

                                messagesReceivedMeasuring(&w);

                                epidemicLocalSummaryVector.insert(make_pair<string, bool>(tokenkey.c_str(),true));
                                //FIFO strategy to set the maximum size that a node is willing to allocate epidemic messages in its buffer
                                queueFIFO.push(tokenkey.c_str());
                                //printQueueFIFO(queueFIFO);
                                bubble("message added in my buffer"); //making animation message more informative
                                //recording some statistics
//                                cout << "findHost()->getFullName() == w.getTarget(): " << " findHost()->getFullName(): " << findHost()->getFullName() << " w.getTarget():" << w.getTarget() << endl;
                                if(strcmp(findHost()->getFullName(), w.getTarget()) == 0) {
                                    //hopCountVector.record(hopCount - w.getHopCount());
                                    //hopCountStats.collect(hopCount - w.getHopCount());
                                    //messageArrivalTimeVector.record((simTime() - w.getTimestamp()));
                                    //messageArrivalTimeStats.collect((simTime() - w.getTimestamp()));
                                    //numMessageReceived++;

                                   // emit(delayToDeliverSignal, (simTime() - w.getTimestamp()));
                                   // emit(hopsToDeliverSignal, (hopCount - w.getHopCount()));
                                   // emit(messageArrivalSignal, 1);

                                    if(maiortempo < (simTime() - w.getTimestamp()).dbl()) {
                                        maiortempo = (simTime() - w.getTimestamp()).dbl();
                                     }
//                                  cout << "numMessageReceived: " << numMessageReceived << endl;
                                }
                            } else { //An entry in the unordered_map was found
                                //do nothing because the message is already in my epidemicLocalBuffer
                            }
                        } else { //The maximum buffer size was reached, so I have to remove the first item from the queueFIFO
                            //Verifying if there is no entry for current message received in my epidemicLocalMessageBuffer
                            unordered_map<string,WaveShortMessage>::const_iterator got = epidemicLocalMessageBuffer.find(tokenkey.c_str());
                            if(got == epidemicLocalMessageBuffer.end()) { //true value means that there is no entry in the epidemicLocalMessageBuffer for the current message identification
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
                                //recording some statistics
//                                cout << "findHost()->getFullName() == w.getTarget(): " << " findHost()->getFullName(): " << findHost()->getFullName() << " w.getTarget():" << w.getTarget() << endl;
                                if(strcmp(findHost()->getFullName(), w.getTarget()) == 0) {
                                    //hopCountVector.record(hopCount - w.getHopCount());
                                    //hopCountStats.collect(hopCount - w.getHopCount());
                                    //messageArrivalTimeVector.record((simTime() - w.getTimestamp()));
                                    //messageArrivalTimeStats.collect((simTime() - w.getTimestamp()));
                                    cout << "Removing an item in my buffer because the size of it was reached" << endl;
                                    //cout << "numMessageReceived++: " << numMessageReceived++ << endl;
                                    //numMessageReceived++;
                                    //emit(arrivalSignal, (hopCount - w.getHopCount()));
                                    //emit(arrivalSignal, (simTime() - w.getTimestamp()));
                                    if(maiortempo < (simTime() - w.getTimestamp()).dbl()) {
                                        maiortempo = (simTime() - w.getTimestamp()).dbl();
                                    }
//                                  cout << "numMessageReceived: " << numMessageReceived << endl;
                                }
                            } else { //An entry in the unordered_map was found
                                //do nothing because the message is already in my epidemicLocalBuffer
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
                 if(wsm->getSenderAddress() < MACToInteger()) {
                   sendLocalSummaryVector(wsm->getSenderAddress());
                 }
             } //end of if
        }// end of else

        //Verifying if I'm the target of a message
        if (strcmp(wsm->getTarget(),findHost()->getFullName()) == 0) {
          std::cout << "I'm " << findHost()->getFullName() << ", the recipient of the message." << " at " << simTime() << std::endl;
          //sendWSM(prepareWSM_epidemic("beacon", beaconLengthBits, type_CCH, beaconPriority, wsm->getSenderAddress(), -1));
        } else { //this node is a relaying node because it is not the target of the message
             //std::cout << findHost()->getFullName() << " will cache the message for forwarding it later." << " at " << simTime() << std::endl;
        }
    }
}

void epidemic_rsu::sendBeacon() {
    t_channel channel = dataOnSch ? type_SCH : type_CCH;
    std::cout << "Channel: " << channel << std::endl;
    WaveShortMessage* wsm = prepareWSM_epidemic("beacon", dataLengthBits, channel, dataPriority, BROADCAST, 2);
    sendWSM(wsm);
}

void epidemic_rsu::sendWSM(WaveShortMessage* wsm) {
    sendDelayedDown(wsm,individualOffset);
}

//Method used to initiate the anti-entropy session sending the epidemicLocalSummaryVector
void epidemic_rsu::sendLocalSummaryVector(unsigned int newRecipientAddress) {
    t_channel channel = dataOnSch ? type_SCH : type_CCH;
    WaveShortMessage* wsm = prepareWSM_epidemic("data", dataLengthBits, channel, dataPriority, newRecipientAddress,2);
    wsm->setSummaryVector(true);
    wsm->setRequestMessages(false);
    //Put the summary vector here, on data wsm field
    wsm->setWsmData(getLocalSummaryVectorData().c_str());
    //Sending the summary vector
    sendWSM(wsm);
}

void epidemic_rsu::sendMessagesRequested(string s, unsigned int recipientAddress) {
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
      if (i%2 == 0) {
          ostringstream ss;
          //cout << "i = " << i << " - looking for the message key(s.substr(0, pos)): " << s.substr(0, pos) << endl;
          WaveShortMessage w;
          unordered_map<string,WaveShortMessage>::const_iterator got = epidemicLocalMessageBuffer.find(s.substr(0, pos));
          if(got == epidemicLocalMessageBuffer.end()){ //true value means that there is no entry in the epidemicLocalSummaryVector for a epidemicRemoteSummaryVector key

          } else {
               w = got->second;
               //WaveShortMessage w = getEpidemicLocalMessageBuffer(s.substr(0, pos));
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

//void epidemic_rsu::sendMessagesRequested(string s, unsigned int recipientAddress) {
//
//    cout << "I'm " << findHost()->getFullName() << "(" << MACToInteger() << "). Sending the following messages resquested: " << s << " to " << recipientAddress << endl;
//    t_channel channel = dataOnSch ? type_SCH : type_CCH;
//    WaveShortMessage* wsm = prepareWSM_epidemic("data", dataLengthBits, channel, dataPriority, recipientAddress,2);
//    string delimiter = "|";
//    size_t pos = 0;
//    string token;
//    unsigned i = 0;
//    string message="";
//
//    printEpidemicLocalMessageBuffer();
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
//          WaveShortMessage w = getEpidemicLocalMessageBuffer(s.substr(0, pos));
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

void epidemic_rsu::sendEpidemicRequestMessageVector(unsigned int newRecipientAddress) {
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

void epidemic_rsu::createEpidemicRemoteSummaryVector(string s) {
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
      if(i%2 == 0) {
        epidemicRemoteSummaryVector.insert(make_pair<string, bool>(s.substr(0, pos),true));
      }
      s.erase(0, pos + delimiter.length());
      i++;
    }
    //std::cout << s << std::endl;
}

void epidemic_rsu::createEpidemicRequestMessageVector() {
    epidemicRequestMessageVector.clear();
    for(auto& x: epidemicRemoteSummaryVector) {
        unordered_map<string,bool>::const_iterator got = epidemicLocalSummaryVector.find(x.first);
        //cout << "I'm in createEpidemicRequestMessageVector().  x.first: " << x.first << " x.second: " << x.second << endl;
        if (got == epidemicLocalSummaryVector.end()) { //true value means that there is no entry in the epidemicLocalSummaryVector for a epidemicRemoteSummaryVector key
            //Putting the message in the EpidemicRequestMessageVector
            string s = x.first;
            epidemicRequestMessageVector.insert(make_pair<string, bool>(s.c_str(),true));
        } else { //An entry in the unordered_map was found
            //cout << "I'm in createEpidemicRequestMessageVector().  got->first: " << got->first << " got->second: " << got->second << endl;
            //cout << "The message " << got->first << " in the epidemicRemoteSummaryVector was found in my epidemicLocalSummaryVector." << endl;
        }
    }
}

string epidemic_rsu::getEpidemicRequestMessageVectorData() {
    ostringstream ss;
    //adding the requester name in order to identify if the requester is also the target of the messages with hopcount == 1.
    //In this case, hopcount == 1, the messages can be sent to the target. Otherwise, the message will not be spread
    ss << findHost()->getFullName() << "|";
    for(auto& x: epidemicRequestMessageVector) {
        ss << x.first << "|" << x.second << "|";
    }
    string s = ss.str();
    s = s.substr(0, s.length() - 1);
    cout << "String format of EpidemicRequestMessageVector from " << findHost()->getFullName() << ": " <<  s  << endl;
    return s.c_str();
}

//WaveShortMessage epidemic_rsu::getEpidemicLocalMessageBuffer(string s){
//    //cout << "Getting the epidemicLocalMessageBuffer[" << s << "] from " << findHost()->getFullName() << endl;
//    unordered_map<string,WaveShortMessage>::const_iterator got = epidemicLocalMessageBuffer.find(s.c_str());
//    return got->second;
//}

//Method used to convert the unordered_map epidemicLocalSummaryVectorData in a string
string epidemic_rsu::getLocalSummaryVectorData() {
    ostringstream ss;
    for(auto& x: epidemicLocalSummaryVector) {
        ss << x.first << "|" << x.second << "|";
    }
    string s = ss.str();
    s = s.substr(0, s.length() - 1);
    //cout << "EpidemicLocalSummaryVector from " << findHost()->getFullName() << "(" << MACToInteger() << "): " <<  s  << endl;
    return s.c_str();
}

void epidemic_rsu::printQueueFIFO(queue<string> qFIFO) {
    int i = 0;
    while(!qFIFO.empty()) {
        cout << "I'm " << findHost()->getFullName() << " - queueFIFO Element " << ++i << ": " << qFIFO.front() << endl;
        qFIFO.pop();
    }
}

void epidemic_rsu::printEpidemicLocalMessageBuffer() {
    if (epidemicLocalMessageBuffer.empty()) {
           cout << "EpidemicLocalMessageBuffer from " << findHost()->getFullName() << " is empty now " << endl;
    } else {
        int i = 0;
        cout << "RSU - Printing the epidemicLocalMessageBuffer from " << findHost()->getFullName() << "(" << MACToInteger() <<"):" << endl;
        for(auto& x: epidemicLocalMessageBuffer) {
            WaveShortMessage wsmBuffered = x.second;
            cout << " Key " << ++i << ": " << x.first << " - Message Content: " << wsmBuffered.getWsmData() << " source: " << wsmBuffered.getSource() << " target: " << wsmBuffered.getTarget() << " Timestamp: " << wsmBuffered.getTimestamp() << " HopCount: " << wsmBuffered.getHopCount() << endl;
            cout << "GlobalID" << wsmBuffered.getGlobalMessageIdentificaton() << endl;
        }
    }
}

void epidemic_rsu::printEpidemicLocalSummaryVectorData() {
    if (epidemicLocalSummaryVector.empty()){
           cout << "EpidemicLocalSummaryVector from " << findHost()->getFullName() << " is empty now " << endl;
    } else {
        ostringstream ss;
        for(auto& x: epidemicLocalSummaryVector) {
           ss << x.first << "|" << x.second << "|";
        }
        string s = ss.str();
        s = s.substr(0, s.length() - 1);
        cout << "EpidemicLocalSummaryVector from " << findHost()->getFullName() << "(" << MACToInteger() << "): " <<  s  << endl;
    }
}

void epidemic_rsu::printEpidemicRemoteSummaryVectorData() {
    if (epidemicRemoteSummaryVector.empty()) {
           cout << "EpidemicRemoteSummaryVector from " << findHost()->getFullName() << " is empty now " << endl;
    } else {
        ostringstream ss;
        for(auto& x: epidemicRemoteSummaryVector) {
            ss << x.first << "|" << x.second << "|";
        }
        string s = ss.str();
        s = s.substr(0, s.length() - 1);
        cout << "EpidemicRemoteSummaryVector from " << findHost()->getFullName() << ": " <<  s  << endl;
    }
}

void epidemic_rsu::printEpidemicRequestMessageVector() {
    if (epidemicRequestMessageVector.empty()) {
        cout << "EpidemicRequestMessageVector from " << findHost()->getFullName() << " is empty now " << endl;
    } else {
        ostringstream ss;
        for(auto& x: epidemicRequestMessageVector)
            ss << x.first << "|" << x.second << "|";
        string s = ss.str();
        s = s.substr(0, s.length() - 1);
        cout << "EpidemicRequestMessageVector from " << findHost()->getFullName() << ": " <<  s  << endl;
    }
}

void epidemic_rsu::printNodesIRecentlySentSummaryVector(){
    if (nodesIRecentlySentSummaryVector.empty()) {
           cout << "NodesIRecentlySentSummaryVector from " << findHost()->getFullName() << " is empty now " << endl;
    } else {
        int i = 0;
        cout << "NodesIRecentlySentSummaryVector from " << findHost()->getFullName() << " (" << MACToInteger() << "):" << endl;
        for(auto& x: nodesIRecentlySentSummaryVector) {
            cout << ++i << " Node: " << x.first << " added at " << x.second << endl;
        }
    }
}

void epidemic_rsu::printWaveShortMessage(WaveShortMessage wsm) {
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
    cout << "wsm.getGlobalMessageIdentificaton(): " << wsm.getGlobalMessageIdentificaton() << endl;
    cout << "wsm.getHopCount(): " << wsm.getHopCount() << endl;
}

void epidemic_rsu::printWaveShortMessage(WaveShortMessage* wsm) {
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
    cout << "wsm.getGlobalMessageIdentificaton(): " << wsm->getGlobalMessageIdentificaton() << endl;
    cout << "wsm.getHopCount(): " << wsm->getHopCount() << endl;
}

void epidemic_rsu::finish() {
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

    cout << "Test1: Count of messages received by the RSU: " << epidemicLocalMessageBuffer.size() << endl;
    printCountMessagesReceived();
    if (experimentNumber == 8) { // maxSpeed 15 m/s
        string comand = "sed -i 's/maxSpeed=.* color/maxSpeed=\"15\" color/g' vehDist.rou.xml";
        system(comand.c_str());
        cout << endl << "Setting speed back to default (15 m/s), command: " << comand << endl;
    }
}
