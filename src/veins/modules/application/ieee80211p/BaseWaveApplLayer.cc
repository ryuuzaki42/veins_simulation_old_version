// Copyright (C) 2015-2016 João Batista <joao.b@usp.br>

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

void BaseWaveApplLayer::openFileAndClose(string fileName, bool justForAppend) {
    if (justForAppend) {
        myfile.open(fileName, std::ios_base::app);
    } else {
        myfile.open(fileName);
    }

    printHeaderfileExecution();
    myfile.close();
}

void BaseWaveApplLayer::printHeaderfileExecution() {
    if (SrepeatNumber != 0) {
        myfile << endl;
    }

    string expSendbyDSCRText;
    if (SexpSendbyDSCR < 10) {
        expSendbyDSCRText = "000";
    } else if (SexpSendbyDSCR < 100) {
        expSendbyDSCRText = "00";
    } else if (SexpSendbyDSCR < 1000) {
        expSendbyDSCRText = "0";
    }
    expSendbyDSCRText += to_string(SexpSendbyDSCR);

    myfile << "Exp: " << SexpNumber << " expSendbyDSCR: " << expSendbyDSCRText.c_str() << " ################";
    myfile << "##########################################################################" << endl;
    myfile << "Exp: " << SexpNumber << " expSendbyDSCR: " << expSendbyDSCRText.c_str() << " ### ExpNumber: " << SexpNumber << " RepeatNumber: " << SrepeatNumber;
    myfile << " ttlBeaconMessage: " << SttlBeaconMessage << " countGenerateBeaconMessage: " << ScountGenerateBeaconMessage << endl << endl;
}

void BaseWaveApplLayer::generalInitializeVariables_executionByExpNumberVehDist() {
    source = findHost()->getFullName();
    msgBufferMaxUse = 0;

    if (myId == 0) { // Vehicle must be the first to generate messages, so your offset is 0;
        SbeaconMessageHopLimit = par("beaconMessageHopLimit").longValue();
        string seedNumber = ev.getConfig()->getConfigValue("seed-set");
        SrepeatNumber = atoi(seedNumber.c_str()); // Number of execution (${repetition})
        SexpSendbyDSCR = par("expSendbyDSCR").longValue();

        SallowMessageCopy = par("allowMessageCopy").boolValue();
        SvehSendWhileParking = par("vehSendWhileParking").boolValue();
        SselectFromAllVehicles = par("selectFromAllVehicles").boolValue();
        SusePathHistory = par("usePathHistory").boolValue(); // User or not path history when will send a message
        SuseMessagesSendLog = par("useMessagesSendLog").boolValue();
        SvehDistTrueEpidemicFalse = par("vehDistTrueEpidemicFalse").boolValue();
        SvehDistCreateEventGenerateMessage = par("vehDistCreateEventGenerateMessage").boolValue();

        SttlBeaconStatus = par("ttlBeaconStatus");
        SbeaconMessageBufferSize = par("beaconMessageBufferSize");
        SbeaconStatusBufferSize = par("beaconStatusBufferSize");
        StimeToUpdatePosition = par("vehTimeUpdatePosition");
        StimeLimitGenerateBeaconMessage = par("timeLimitGenerateBeaconMessage");
        SpercentP = par("percentP"); // 20 meaning 20% of message send to category P

        SexpNumber = par("expNumber").longValue();
        if ((SexpNumber == 1) || (SexpNumber == 5)) {
            SttlBeaconMessage = par("ttlBeaconMessage_one").longValue();
            ScountGenerateBeaconMessage = par("countGenerateBeaconMessage_one").longValue();
        } else if ((SexpNumber == 2) || (SexpNumber == 6)) {
            SttlBeaconMessage = par("ttlBeaconMessage_one").longValue();
            ScountGenerateBeaconMessage = par("countGenerateBeaconMessage_two").longValue();
        } else if ((SexpNumber == 3) || (SexpNumber == 7)) {
            SttlBeaconMessage = par("ttlBeaconMessage_two").longValue();
            ScountGenerateBeaconMessage = par("countGenerateBeaconMessage_one").longValue();
        } else if ((SexpNumber == 4) || (SexpNumber == 8)) {
            SttlBeaconMessage = par("ttlBeaconMessage_two").longValue();
            ScountGenerateBeaconMessage = par("countGenerateBeaconMessage_two").longValue();
        } else {
            cout << "Error: Number of experiment not configured. Go to BaseWaveApplLayer.cc line 155" << endl;
            exit(33);
        }

        ScountMesssageDrop = ScountMsgPacketSend = SmsgBufferUseGeneral = ScountVehicleAll = 0;
        SmsgDroppedbyTTL = SmsgDroppedbyCopy = SmsgDroppedbyBuffer = 0;
        ScountMeetPshortestT = ScountTwoCategoryN = ScountMeetN = 0;
        SbeaconMessageId = 1;

        // Initialize random seed (Seed the RNG) # Inside of IF because must be executed one time (the seed is "static")
        mt_veh.seed(SrepeatNumber); // Instead another value, for make the experiment more reproducible, so seed = reapeatNumber
        srand(SrepeatNumber + 1); // repeatNumber + 1, because srand(0) == srand(1)

        // To run with different routes files use only one seed
        //mt_veh.seed(1);
        //srand(1);

        string texTmp = "\nExp: " + to_string(SexpNumber);
        SprojectInfo = texTmp;
        SprojectInfo += texTmp + " Project informations:";
        SprojectInfo += texTmp + " vehDistTrueEpidemicFalse: " + boolToString(SvehDistTrueEpidemicFalse);
        SprojectInfo += texTmp + " vehDistCreateEventGenerateMessage: " + boolToString(SvehDistCreateEventGenerateMessage);
        SprojectInfo += texTmp + " Experiment: " + to_string(SexpNumber);
        SprojectInfo += texTmp + " repeatNumber: " + to_string(SrepeatNumber);
        SprojectInfo += texTmp + " ttlBeaconMessage: " + to_string(SttlBeaconMessage);
        SprojectInfo += texTmp + " countGenerateBeaconMessage: " + to_string(ScountGenerateBeaconMessage);
        SprojectInfo += texTmp + " allowMessageCopy: " + boolToString(SallowMessageCopy);
        SprojectInfo += texTmp + " sendWhileParking: " + boolToString(SvehSendWhileParking);
        SprojectInfo += texTmp + " selectFromAllVehicles: " + boolToString(SselectFromAllVehicles);
        SprojectInfo += texTmp + " timeLimitGenerateMessage: " + to_string(StimeLimitGenerateBeaconMessage);
        SprojectInfo += texTmp + " beaconMessageHopLimit: " + to_string(SbeaconMessageHopLimit);
        SprojectInfo += texTmp + " expSendbyDSCR: " + to_string(SexpSendbyDSCR);
        SprojectInfo += texTmp + " ttlBeaconStatus: " + to_string(SttlBeaconStatus);
        SprojectInfo += texTmp + " beaconMessageBufferSize: " + to_string(SbeaconMessageBufferSize);
        SprojectInfo += texTmp + " beaconStatusBufferSize:" + to_string(SbeaconStatusBufferSize);
        SprojectInfo += texTmp + "\n";

        if (SvehDistTrueEpidemicFalse) {
            SprojectInfo += texTmp + " percentP: " + to_string(SpercentP);
            SprojectInfo += texTmp + " usePathHistory: " + boolToString(SusePathHistory);
            SprojectInfo += texTmp + " useMessagesSendLog " + boolToString(SuseMessagesSendLog);
            SprojectInfo += texTmp + " timeToUpdatePosition: " + to_string(StimeToUpdatePosition);
        } else {
            SprojectInfo += texTmp + " sendSummaryVectorInterval: " + to_string(sendSummaryVectorInterval);
            SprojectInfo += texTmp + " maximumEpidemicBufferSize: " + to_string(maximumEpidemicBufferSize);
        }

        SprojectInfo += texTmp + "\n";
        cout << endl << SprojectInfo << endl;
    }
}

string BaseWaveApplLayer::boolToString(bool value) {
    if (value) {
        return "true";
    } else {
        return "false";
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
        case 99:
            expSendbyDSCRText = "0099_epidemic";
            break;
        default:
            cout << "Error, expSendbyDSCR: " << expSendbyDSCR << " not defined, class in BaseWaveApplLayer.cc line 183";
            exit(1);
    }

    unsigned short int expPartOneOrTwo = par("expPart_one_or_two");
    resultFolderPart = "results/vehDist_resultsEnd_" + to_string(expPartOneOrTwo) + "/" + expSendbyDSCRText + "/";
    resultFolderPart += "E" + to_string(SexpNumber) + "_" + to_string(SttlBeaconMessage) + "_";
    resultFolderPart += to_string(ScountGenerateBeaconMessage) +"/";

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
        if (SexpNumber <= 4) { // Set the maxSpeed to 15 m/s in the expNumber 1 to 4
            string comand = "sed -i 's/maxSpeed=.* color/maxSpeed=\"15\" color/g' vehDist.rou.xml";
            system(comand.c_str());
            cout << endl << "Change the speed to 15 m/s, command: " << comand << endl;
        } else if (SexpNumber >= 5) { // Set the maxSpeed to 25 m/s in the expNumber 5 to 8
            string comand = "sed -i 's/maxSpeed=.* color/maxSpeed=\"25\" color/g' vehDist.rou.xml";
            system(comand.c_str());
            cout << endl << "Change the speed to 25 m/s, command: " << comand << endl;
        }

        string commandCreateFolder = "mkdir -p " + folderResult + " > /dev/null";
        cout << endl << "Created the folder, command: \"" << commandCreateFolder << "\"" << endl;
        cout << "repeatNumber: " << SrepeatNumber << endl;
        system(commandCreateFolder.c_str()); // Try create a folder to save the results

        if (SrepeatNumber == 0) {
            justAppend = false; // Open a new file (blank)
        } else {
            justAppend = true;
        }
    } else { // repeatNumber != 0 just append
        justAppend = true;
    }

    openFileAndClose(fileMessagesCount, justAppend);
    openFileAndClose(fileMessagesUnicast, justAppend);
    openFileAndClose(fileMessagesBroadcast, justAppend);
}

void BaseWaveApplLayer::restartFilesResultVeh(string projectInfo, Coord initialPos) {
    string resultFolder = getFolderResultVehDist(SexpSendbyDSCR);
    saveVehStartPositionVeh(resultFolder, initialPos); // Save the start position of vehicle. Just for test of the seed.

    fileMessagesUnicast = fileMessagesDrop = fileMessagesGenerated = resultFolder;
    fileMessagesUnicast += "Veh_Unicast_Messages.r";
    fileMessagesDrop += "Veh_Messages_Drop.r";
    fileMessagesGenerated += "Veh_Messages_Generated.r";

    // fileMessagesBroadcast and fileMessagesCount not used yet to vehicle

    bool justAppend;
    if (myId == 0) {
        if (SrepeatNumber == 0) {
            justAppend = false;
        } else { // (repeatNumber != 0)) // open just for append
            justAppend = true;
        }

        openFileAndClose(fileMessagesUnicast, justAppend);
        openFileAndClose(fileMessagesDrop, justAppend);
        openFileAndClose(fileMessagesGenerated, justAppend);

        myfile.open(fileMessagesDrop, std::ios_base::app);
        myfile << projectInfo << endl;
        myfile.close();
    }
}

void BaseWaveApplLayer::saveVehStartPositionVeh(string fileNameLocation, Coord initialPos) {
    fileNameLocation += "Veh_Position_Initialize.r";
    if (myId == 0) {
        if (SrepeatNumber == 0) {
            myfile.open(fileNameLocation);
        } else {
            myfile.open(fileNameLocation, std::ios_base::app);
        }

        printHeaderfileExecution();
        myfile << "Start Position Vehicles" << endl;
    } else {
        myfile.open(fileNameLocation, std::ios_base::app);
    }

    myfile << source << " : " << initialPos << endl;
    myfile.close();
}

void BaseWaveApplLayer::vehGenerateBeaconMessageBeginVeh(double vehOffSet) {
    if (sendData) {
        sendGenerateBeaconMessageEvt = new cMessage("Event generate beacon message", SendEvtGenerateBeaconMessage);
        //cout << source << " at: " << simTime() << " schedule created sendGenerateMessageEvt to: "<< (simTime() + vehOffSet) << endl;
        scheduleAt((simTime() + vehOffSet), sendGenerateBeaconMessageEvt);
    }
}

void BaseWaveApplLayer::vehGenerateBeaconMessageAfterBeginVeh() {
    selectVehGenerateMessage();

    auto itVeh = find(SvehGenerateMessage.begin(), SvehGenerateMessage.end(), source);
    if (itVeh != SvehGenerateMessage.end()) { // If have "vehNumber" on buffer, will generate one message
        if (SvehDistTrueEpidemicFalse) {
            generateBeaconMessageVehDist();;
        } else {
            generateMessageEpidemic();
        }

        SvehGenerateMessage.erase(itVeh);
    }
}

void BaseWaveApplLayer::generateTarget() { // Set the target node to who the message has to be delivered
    target = "rsu[0]";
    target_x = par("vehBeaconMessageTarget_x").longValue();
    target_y = par("vehBeaconMessageTarget_y").longValue();
}

void BaseWaveApplLayer::generateBeaconMessageVehDist() {
    WaveShortMessage* wsm = new WaveShortMessage("beaconMessage");
    wsm->setType(2); // Beacon of Message
    wsm->addBitLength(headerLength);
    wsm->addBitLength(dataLengthBits);
    t_channel channel = dataOnSch ? type_SCH : type_CCH;
    switch (channel) {
        case type_SCH: // Will be rewritten at Mac1609_4 to actual Service Channel. This is just so no controlInfo is needed
            wsm->setChannelNumber(Channels::SCH1);
            break;
        case type_CCH:
            wsm->setChannelNumber(Channels::CCH);
            break;
    }
    wsm->setPsid(0);
    wsm->setPriority(dataPriority);
    wsm->setWsmVersion(1);
    wsm->setSenderPos(curPosition);
    wsm->setSerial(2);
    wsm->setTimestamp(simTime());

    wsm->setSource(source.c_str());
    generateTarget(); // target = rsu[0], rsu[1] or car[*] and the respective position.
    wsm->setTarget(target.c_str());
    wsm->setTargetPos(Coord(target_x, target_y, 3));
    wsm->setSenderAddressTemporary(source.c_str());
    wsm->setRecipientAddressTemporary("Initial"); // Defined in time before send

    wsm->setHopCount(SbeaconMessageHopLimit + 1); // Is beaconMessageHopLimit+1 because hops count is equals to routes in the path, not hops.
    string wsmDataTmp = "WSMData generated by " + source;
    wsm->setWsmData(wsmDataTmp.c_str());

    myfile.open(fileMessagesGenerated, std::ios_base::app); // Save info (Id and vehicle generate) on fileMessagesGenerated
    myfile << "                                                                     ";
    if (SbeaconMessageId < 10) {
        string msgIdTmp = "00" + to_string(SbeaconMessageId);
        wsm->setGlobalMessageIdentificaton(msgIdTmp.c_str()); // Id 01 to 09
    } else if (SbeaconMessageId < 100) {
        string msgIdTmp = "0" + to_string(SbeaconMessageId);
        wsm->setGlobalMessageIdentificaton(msgIdTmp.c_str()); // Id 11 to 99
    } else {
        wsm->setGlobalMessageIdentificaton(to_string(SbeaconMessageId).c_str()); // Id 100 and going on
    }
    myfile << "### " << source << " generated the message ID: " << wsm->getGlobalMessageIdentificaton() << " at: " << simTime() << endl;
    cout << "### " << source << " generated the message ID: " << wsm->getGlobalMessageIdentificaton() << " at: " << simTime() << endl;
    myfile.close();

    //bool insert = sendOneNewMessageToOneNeighborTarget(*wsm);
    bool insert = true;
    if (insert) {
        messagesBuffer.insert(make_pair(wsm->getGlobalMessageIdentificaton(),*wsm)); // Adding the message on the buffer
        if (messagesBuffer.size() > msgBufferMaxUse) {
            msgBufferMaxUse = messagesBuffer.size();
        }
        messagesOrderReceived.push_back(wsm->getGlobalMessageIdentificaton());
    }

    colorCarryMessageVehDist(messagesBuffer); // Change the range-color in the vehicle (GUI)
    SbeaconMessageId++;
}

void BaseWaveApplLayer::printCountBeaconMessagesDropVeh() {
    myfile.open (fileMessagesDrop, std::ios_base::app);

    myfile << endl << "messagesDrop from " << source << endl;
    unsigned short int messageDropbyOneVeh = 0;
    map <string, struct messagesDropStruct>::iterator itMessageDrop;
    for (itMessageDrop = messagesDrop.begin(); itMessageDrop != messagesDrop.end(); itMessageDrop++) {
        myfile << "Message Id: " << itMessageDrop->first << endl;
        myfile << "By Buffer: " << itMessageDrop->second.byBuffer << endl;
        myfile << "By Time: " << itMessageDrop->second.byTime << endl;
        myfile << "By Copy: " << itMessageDrop->second.byCopy << endl;
        myfile << "TimeGenerate: " << itMessageDrop->second.timeGenerate << endl;
        myfile << "TimeDroped: " << itMessageDrop->second.timeDroped << endl;
        myfile << "TimeDifference: " << itMessageDrop->second.timeDifference << endl << endl;
        messageDropbyOneVeh += itMessageDrop->second.byBuffer + itMessageDrop->second.byTime;
        SmsgDroppedbyTTL += itMessageDrop->second.byTime;
        SmsgDroppedbyCopy += itMessageDrop->second.byCopy;
        SmsgDroppedbyBuffer += itMessageDrop->second.byBuffer;
    }

    ScountMesssageDrop += messageDropbyOneVeh;
    myfile << "### " << source << " dropped: " << messageDropbyOneVeh << endl;
    myfile << "### " << source << " use message buffer: " << msgBufferMaxUse << endl;
    SmsgBufferUseGeneral += msgBufferMaxUse;

    if (SnumVehicles.size() == 1) {
        myfile << endl << "Exp: " << SexpNumber << " ### Final count messages drop: " << ScountMesssageDrop << endl;
        myfile << "Exp: " << SexpNumber << " ### Final count message dropped by buffer: " << SmsgDroppedbyBuffer << endl;
        myfile << "Exp: " << SexpNumber << " ### Final count message dropped by copy (Only valid if copy of message are not allowed): " << SmsgDroppedbyCopy << endl;
        myfile << "Exp: " << SexpNumber << " ### Final count message dropped by ttl: " << SmsgDroppedbyTTL << endl;
        myfile << "Exp: " << SexpNumber << " ### Final average buffer use: " << double(SmsgBufferUseGeneral)/ScountVehicleAll << endl;
        myfile << "Exp: " << SexpNumber << " ### Count of vehicle in the scenario: " << ScountVehicleAll << endl;
        myfile << "Exp: " << SexpNumber << " ### Count meetings: " << (ScountTwoCategoryN + ScountMeetN) << endl;
        myfile << "Exp: " << SexpNumber << " ### Count meetings two category: " << ScountTwoCategoryN << endl;
        myfile << "Exp: " << SexpNumber << " ### Count meetings another: " << ScountMeetN << endl;
        myfile << "Exp: " << SexpNumber << " ### Count meetings with real difference (Only valid with Category test): " << ScountMeetPshortestT << endl;
        myfile << "Exp: " << SexpNumber << " ### Final count packets messages send: " << ScountMsgPacketSend << endl << endl;
    }
    myfile.close();
}

void BaseWaveApplLayer:: finishVeh() {
    printCountBeaconMessagesDropVeh();

    auto itVeh = find(SnumVehicles.begin(), SnumVehicles.end(), source);
    if (itVeh != SnumVehicles.end()) {
        SnumVehicles.erase(itVeh);
        SvehScenario.erase(source);
    } else {
        cout << "Error in vehDist::numVehicles, need to have the same entries as the number of vehicles" << endl;
        exit (1);
    }
}

void BaseWaveApplLayer::colorCarryMessageVehDist(unordered_map <string, WaveShortMessage> bufferOfMessages) {
    if (!bufferOfMessages.empty()) {
        unordered_map <string, WaveShortMessage>::iterator itMessage = bufferOfMessages.begin();
        for (unsigned int i = 0; i < bufferOfMessages.size(); i++) {
            if (source.compare(itMessage->second.getSource()) == 0) {
                findHost()->getDisplayString().updateWith("r=12,green"); // Has message(s) with himself was generated
                return; // Most important messages generated by himself
            } else {
                findHost()->getDisplayString().updateWith("r=12,blue"); // Has (only) message(s) with another was generated
            }
            itMessage++;
        }
    } else {
        findHost()->getDisplayString().updateWith("r=0"); // Remove the range color
    }
}

void BaseWaveApplLayer::selectVehGenerateMessage() {
    if (myId == 0) { // If true, some vehicle has (in past) selected the vehicle to generate messages
        if (simTime() <= StimeLimitGenerateBeaconMessage) { // Modify the generate message and test vehDist::timeLimitGenerateBeaconMessage
            cout << source << " at " << simTime() << " StimeLimitGenerateBeaconMessage: " << StimeLimitGenerateBeaconMessage << endl;

            unsigned short int vehSelected;
            myfile.open(fileMessagesGenerated, std::ios_base::app); // To save info (Id and vehicle generate) on fileMessagesGenerated
            unsigned short int trySelectVeh = 0;
            for (unsigned short int i = 0; i < ScountGenerateBeaconMessage;) { // select <countGenerateBeaconMessage> distinct vehicle to generate messages
                //vehSelected = intuniform(1, (vehDist::numVehicles -1), repeatNumber); // Get random number
                //cout << getRNG(0) << endl; // Get seed used on random number

                //uniform_int_distribution <int> dist(0, (vehDist::numVehicles.size() - 1));

                // TODO Need of new way to generate messages
                //uniform_int_distribution <int> dist(0, 9);
                uniform_int_distribution <int> dist(0, (SnumVehicles.size() - 1));
                vehSelected = dist(mt_veh);
                string vehSelectedId = SnumVehicles[vehSelected];

                if (SselectFromAllVehicles) {
                    auto itVehSelected = find(SvehGenerateMessage.begin(), SvehGenerateMessage.end(), vehSelectedId);
                    if (itVehSelected == SvehGenerateMessage.end()) {
                        SvehGenerateMessage.push_back(vehSelectedId);
                        cout << endl << source << " selected the " << vehSelectedId << " to generate a message at: " << simTime() << endl;
                        myfile << source << " selected the " << vehSelectedId << " to generate a message at: " << simTime() << endl;
                        i++;
                    }
                } else {
                    if ((SvehScenario[vehSelectedId].getTimestamp() + 60) >= simTime()) { // Test if vehicle are less than 60 s in the scenario
                    //if (((vehDist::vehScenario[vehSelectedId].getTimestamp() + 60) >= simTime()) || (strcmp(vehDist::vehScenario[vehSelectedId].getCategory(), "T") == 0)) { // Test if vehicle are less than 60 s in the scenario
                        auto itVehSelected = find(SvehGenerateMessage.begin(), SvehGenerateMessage.end(), vehSelectedId);
                        if (itVehSelected == SvehGenerateMessage.end()) {
                            SvehGenerateMessage.push_back(vehSelectedId);
                            cout << endl << source << " selected the " << vehSelectedId << " to generate a message at: " << simTime() << endl;
                            myfile << source << " selected the " << vehSelectedId << " to generate a message at: " << simTime() << endl;
                            i++;
                        }
                    } else {
                        cout << source << " selected " << vehSelectedId << " to generate " << SbeaconMessageId;
                        cout << " message, but has Timestamp: " << SvehScenario[vehSelectedId] <<" at " << simTime() << endl;
                        if (trySelectVeh > (SvehScenario.size() * 4)) {
                            cout << "Error loop in select vehicle to generate message, go to vehDist.cc line 941" << endl;
                            cout << "trySelectVeh: " << trySelectVeh << " vehDist::vehScenario.size(): " << SvehScenario.size() << endl;
                            exit(3);
                        }
                        trySelectVeh++;
                    }
                }
            }
            myfile.close();
        }
    }
}

void BaseWaveApplLayer::insertMessageDropVeh(string messageId, unsigned short int type, simtime_t timeGenarted) {
    if (messagesDrop.empty() || (messagesDrop.find(messageId) == messagesDrop.end())) {
        struct messagesDropStruct mD_tmp;
        mD_tmp.byBuffer = mD_tmp.byTime = mD_tmp.byCopy = 0;

        if (type == 1) { // by buffer limit
            mD_tmp.byBuffer = 1;
        } else if (type == 2) { // By copy not allow
            mD_tmp.byCopy = 1;
        } else if (type == 3) { // by TTL/time limit
            mD_tmp.byTime = 1;
        }

        mD_tmp.timeGenerate = timeGenarted;
        mD_tmp.timeDroped = simTime();
        mD_tmp.timeDifference = mD_tmp.timeDroped - mD_tmp.timeGenerate;

        messagesDrop.insert(make_pair(messageId, mD_tmp));
    } else {
        if (type == 1) { // Increment the number of byBuffer (limit)
            messagesDrop[messageId].byBuffer++;
        } else if (type == 2) {  // Increment the number of byCopy (limit)
            messagesDrop[messageId].byCopy++;
        } else if (type == 3) { // Increment the number of byTime (limit)
            messagesDrop[messageId].byTime++;
        }
    }
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

        string textTmp = "Exp: " + to_string(SexpNumber) + " ###";
        myfile << endl << endl << textTmp + " Messages received in the " << source << endl;
        myfile << textTmp + " Count messages received: " << messagesReceived.size() << endl;
        myfile << textTmp + " Count messages with hop count equal of zero received: " << messageCountHopZero << endl;
        myfile << textTmp + " Count messages with hop count different of zero Received: " << (messagesReceived.size() - messageCountHopZero) << endl;
        myfile << textTmp + " Average time to receive: " << avgGeneralTimeMessageReceived << endl;
        myfile << textTmp + " Count copy message received: " << avgGeneralCopyMessageReceived << endl;
        avgGeneralCopyMessageReceived /= messagesReceived.size();
        myfile << textTmp + " Average copy received: " << avgGeneralCopyMessageReceived << endl;
        myfile << textTmp + " Average hops to received: " << avgGeneralHopsMessage << endl;
        myfile << textTmp + " Hops by category T general: " << countT << endl;
        myfile << textTmp + " Hops by category P general: " << countP << endl;
    } else {
        myfile << "messagesReceived from " << source << " is empty" << endl;
        myfile << endl << "Exp: " << SexpNumber << " ### Count messages received: " << 0 << endl;
    }
    myfile.close();
}

void BaseWaveApplLayer::messagesReceivedMeasuringRSU(WaveShortMessage* wsm) {
    string wsmData = wsm->getWsmData();
    simtime_t timeToArrived = (simTime() - wsm->getTimestamp());
    unsigned short int countHops = (SbeaconMessageHopLimit - wsm->getHopCount());
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

        source = findHost()->getFullName();
        string seedNumber = ev.getConfig()->getConfigValue("seed-set");
        SrepeatNumber = atoi(seedNumber.c_str()); // Number of execution (${repetition})

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

void BaseWaveApplLayer::generateMessageEpidemic() { //Generate a message in order to be sent to a target
    generateTarget();

    cout << source << " generating one message Id: " << SbeaconMessageId << " (" << source << " -> " << target << ")"<< endl;
    WaveShortMessage* wsm = new WaveShortMessage("beaconMessage");
    wsm->setName("data");
    t_channel channel = dataOnSch ? type_SCH : type_CCH;
    wsm->addBitLength(headerLength);
    wsm->addBitLength(dataLengthBits);
    switch (channel) {
        case type_SCH: // Will be rewritten at Mac1609_4 to actual Service Channel. This is just so no controlInfo is needed
            wsm->setChannelNumber(Channels::SCH1);
            break;
        case type_CCH:
            wsm->setChannelNumber(Channels::CCH);
            break;
    }
    wsm->setPsid(0);
    wsm->setPriority(dataPriority);
    wsm->setWsmVersion(1);
    wsm->setSerial(2);

    wsm->setTimestamp(simTime());
    wsm->setSenderAddress(MACToInteger());

    // wsm.setRecipientAddress(); // Set when will send

    wsm->setSource(source.c_str());
    wsm->setTarget(target.c_str());
    wsm->setSenderPos(curPosition);

    wsm->setSummaryVector(false);
    wsm->setRequestMessages(false);

    string data = "WSMData generated by " + source;
    wsm->setWsmData(data.c_str());
    wsm->setHopCount(SbeaconMessageHopLimit + 1);

    myfile.open(fileMessagesGenerated, std::ios_base::app); // Save info (Id and vehicle generate) on fileMessagesGenerated
    myfile << "                                                                     ";
    if (SbeaconMessageId < 10) {
        string msgIdTmp = "00" + to_string(SbeaconMessageId);
        wsm->setGlobalMessageIdentificaton(msgIdTmp.c_str()); // Id 001 to 009
    } else if (SbeaconMessageId < 100) {
        string msgIdTmp = "0" + to_string(SbeaconMessageId);
        wsm->setGlobalMessageIdentificaton(msgIdTmp.c_str()); // Id 011 to 099
    } else {
        wsm->setGlobalMessageIdentificaton(to_string(SbeaconMessageId).c_str()); // Id 100 and going on
    }
    myfile << "### " << source << " generated the message ID: " << wsm->getGlobalMessageIdentificaton() << " at: " << simTime() << endl;
    cout << "### " << source << " generated the message ID: " << wsm->getGlobalMessageIdentificaton() << " at: " << simTime() << endl;
    myfile.close();

    SbeaconMessageId++;

    epidemicLocalMessageBuffer.insert(make_pair(wsm->getGlobalMessageIdentificaton(), *wsm));
    epidemicLocalSummaryVector.insert(make_pair(wsm->getGlobalMessageIdentificaton(), true));
    if (epidemicLocalMessageBuffer.size() > msgBufferMaxUse) {
        msgBufferMaxUse = epidemicLocalMessageBuffer.size();
    }
    colorCarryMessageVehDist(epidemicLocalMessageBuffer);
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

//Method used to initiate the anti-entropy session sending the epidemicLocalSummaryVector
void BaseWaveApplLayer::sendLocalSummaryVector(unsigned int newRecipientAddress) {
    string idMessage;
    unsigned short int countMessage = epidemicLocalMessageBuffer.size();
    unordered_map <string , WaveShortMessage>::iterator itMsg = epidemicLocalMessageBuffer.begin();
    //printEpidemicLocalMessageBuffer();
    while (countMessage > 0) {
        if ((itMsg->second.getTimestamp() + SttlBeaconMessage) < simTime()) {
            idMessage = itMsg->second.getGlobalMessageIdentificaton();

            if (countMessage == 1) {
                countMessage = 0;
            } else {
                countMessage--;
                itMsg++;
            }

            insertMessageDropVeh(idMessage, 3, itMsg->second.getTimestamp()); // Removed by the value of tyRemoved (1 buffer, 2 hop, 3 time)

            epidemicLocalMessageBuffer.erase(idMessage);
            epidemicLocalSummaryVector.erase(idMessage);
            colorCarryMessageVehDist(epidemicLocalMessageBuffer);
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
        cout << "NodesIRecentlySentSummaryVector from " << source << " is empty at: " << simTime() << endl;
    }
}

void BaseWaveApplLayer::printEpidemicLocalMessageBuffer() {
    if (!epidemicLocalMessageBuffer.empty()) {
        cout << endl << "Printing the epidemicLocalMessageBuffer from " << source << " (" << MACToInteger() <<") at: " << simTime() << endl;

        for (auto& x: epidemicLocalMessageBuffer) {
            WaveShortMessage wsmBuffered = x.second;

            cout << " GlobalID " << ": " << wsmBuffered.getGlobalMessageIdentificaton() << endl;
            cout << " Message Content: " << wsmBuffered.getWsmData() << endl;
            cout << " source: " << wsmBuffered.getSource() << endl;
            cout << " target: " << wsmBuffered.getTarget() << endl;
            cout << " Timestamp: " << wsmBuffered.getTimestamp() << endl;
            cout << " HopCount: " << wsmBuffered.getHopCount() << endl << endl;
        }
    } else {
        cout << "EpidemicLocalMessageBuffer from " << source << " is empty at: " << simTime() << endl;
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
        cout << "EpidemicRequestMessageVector from " << source << " is empty at: " << simTime() << endl;
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
        cout << "EpidemicLocalSummaryVector from " << source << " is empty at: " << simTime() << endl;
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
        cout << "EpidemicRemoteSummaryVector from " << source << " is empty at: " << simTime() << endl;
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
                //cout << "EpidemicRequestMessageVector from " << source << " is empty" << endl;
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
                cout << source << " (" << MACToInteger() << ") received a message requested " << wsm->getGlobalMessageIdentificaton();
                cout << " |> " << wsm->getWsmData() << " <| from " << wsm->getSenderAddress() << " at " << simTime() << endl;

                if (source.compare(wsm->getTarget()) == 0) { // Verifying if is the target of this message
                    cout << source << " received a message for him at " << simTime() << endl;
                    if (source.substr(0, 3).compare("rsu") == 0) {
                        findHost()->bubble("Received Message");
                        saveMessagesOnFile(wsm, fileMessagesUnicast);

                        messagesReceivedMeasuringRSU(wsm);
                    } else { // This message has target destination one vehicle
                        saveMessagesOnFile(wsm, fileMessagesBroadcast);
                    }
                } else {
                    cout << "Before message processing" << endl;
                    printEpidemicLocalMessageBuffer();
                    cout << "Before message processing" << endl;
                    printEpidemicLocalSummaryVectorData();

                    // Verifying if there is no entry for current message received in my epidemicLocalMessageBuffer
                    unordered_map <string, WaveShortMessage>::iterator got = epidemicLocalMessageBuffer.find(wsm->getGlobalMessageIdentificaton());

                    if (got == epidemicLocalMessageBuffer.end()) { // True value means that there is no entry in the epidemicLocalMessageBuffer for the current message identification
                        if (epidemicLocalMessageBuffer.size() > maximumEpidemicBufferSize) { // The maximum buffer size was reached, so remove the oldest item
                            string idMessage;
                            simtime_t minTime = DBL_MAX;
                            for (auto& x: epidemicLocalMessageBuffer) {
                                WaveShortMessage wsmBuffered = x.second;

                                if (minTime > wsmBuffered.getTimestamp()) {
                                    minTime = wsmBuffered.getTimestamp();
                                    idMessage = wsmBuffered.getGlobalMessageIdentificaton();
                                }
                            }

                            insertMessageDropVeh(idMessage, 1, minTime);

                            epidemicLocalMessageBuffer.erase(idMessage);
                            epidemicLocalSummaryVector.erase(idMessage);
                            colorCarryMessageVehDist(epidemicLocalMessageBuffer);
                        }

                        epidemicLocalMessageBuffer.insert(make_pair(wsm->getGlobalMessageIdentificaton(), *wsm));
                        epidemicLocalSummaryVector.insert(make_pair(wsm->getGlobalMessageIdentificaton(), true));
                        if (epidemicLocalMessageBuffer.size() > msgBufferMaxUse) {
                            msgBufferMaxUse = epidemicLocalMessageBuffer.size();
                        }
                        colorCarryMessageVehDist(epidemicLocalMessageBuffer);
                    }

                    cout << "After message processing" << endl;
                    printEpidemicLocalMessageBuffer();
                    cout << "After message processing" << endl;
                    printEpidemicLocalSummaryVectorData();
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

                    if (SendOrNotLocalSummaryVector) {
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
        case SendEvtGenerateBeaconMessage: {
            vehGenerateBeaconMessageAfterBeginVeh();
            scheduleAt((simTime() + par("timeGenerateBeaconMessage").doubleValue()), sendGenerateBeaconMessageEvt);
            break;
        }
        case Send_EpidemicMessageRequestEvt: {
            unordered_map <string, WaveShortMessage>::iterator itEpidemicMessageSend;
            itEpidemicMessageSend = epidemicMessageSend.begin();

            sendWSM(itEpidemicMessageSend->second.dup());
            ScountMsgPacketSend++;

            string idMessage = itEpidemicMessageSend->second.getGlobalMessageIdentificaton();

            if (!SallowMessageCopy) {
                cout << source << " send the message " << idMessage << " to " << epidemicMessageSend[idMessage].getRecipientAddress() << " and removing at " << simTime() << endl;
                insertMessageDropVeh(idMessage, 2, epidemicMessageSend[idMessage].getTimestamp());

                epidemicLocalMessageBuffer.erase(idMessage);
                epidemicLocalSummaryVector.erase(idMessage);
                colorCarryMessageVehDist(epidemicLocalMessageBuffer);
            }

            epidemicMessageSend.erase(idMessage);

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
