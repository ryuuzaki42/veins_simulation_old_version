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

//        if (source.compare("RSU[0]") == 0) {
            //create a folder results
            system("mkdir results");
            //Open a new file for the current simulation
            myfile.open ("results/RSUmessages.txt");
            myfile.close();

            myfile.open ("results/RSUBroadcastmessages.txt");
            myfile.close();
//            //Open a new file for the current simulation
//            myfile.open ("results/LocalMessageBuffer_veh.txt");
//            myfile.close();
//        }
           //cout << " " << findHost()->getFullName() << "entreing " << endl;

    }
}

void vehDist_rsu::onBeacon(WaveShortMessage* wsm) {

}

void vehDist_rsu::onData(WaveShortMessage* wsm){

    if (strcmp(wsm->getTarget(), findHost()->getFullName()) == 0){
        findHost()->bubble("Received data");
        wsm->setTimestamp(simTime());
        recordOnFileMessages(wsm);
    }
    else if (wsm->getRecipientAddress() == 268435455){
           recordOnFileMessagesBroadcast(wsm);
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
    if (strcmp(wsm->getName(), "data") == 0){

        //Open file for just apeend
        myfile.open ("results/RSUmessages.txt", std::ios_base::app);

        //Send "strings" to be saved on the file onBeacon_veh.txt
        myfile << "Data from " << wsm->getSenderAddress() << " at " << simTime();
        myfile << " to " << wsm->getRecipientAddress() << endl;
        myfile << "wsm->getName(): " << wsm->getName() << endl;
        myfile << "wsm->getBitLength(): " << wsm->getBitLength() << endl;
        myfile << "wsm->getChannelNumber(): " << wsm->getChannelNumber() << endl;
        myfile << "wsm->getPsid(): " << wsm->getPsid() << endl;
        myfile << "wsm->getPriority(): " << wsm->getPriority() << endl;
        myfile << "wsm->getWsmVersion(): " << wsm->getWsmVersion() << endl;
        myfile << "wsm->getTimestamp(): " << wsm->getTimestamp() << endl;
        myfile << "wsm->getMessageTimestampGenerate(): " << wsm->getMessageTimestampGenerate() << endl;
        myfile << "wsm->getSenderAddress(): " << wsm->getSenderAddress() << endl;
        myfile << "wsm->getHeading(): " << wsm->getHeading() << endl;
        myfile << "wsm->getRecipientAddress(): " << wsm->getRecipientAddress() << endl;
        myfile << "wsm->getSource(): " << wsm->getSource() << endl;
        myfile << "wsm->getTarget(): " << wsm->getTarget() << "findHost()->getFullName()" << findHost()->getFullName() << endl;
        myfile << "wsm->getSenderPos(): " << wsm->getSenderPos() << endl;
        myfile << "wsm->getSerial(): " << wsm->getSerial() << endl;
        myfile << "wsm->getSummaryVector(): " << wsm->getSummaryVector() << endl;
        myfile << "wsm->getRequestMessages(): " << wsm->getRequestMessages() << endl;
        myfile << "wsm->getWsmData(): " << wsm->getWsmData() << endl;
        myfile << "Time to generate and recived: " << (wsm->getTimestamp() - wsm->getMessageTimestampGenerate()) << endl;
        myfile << endl << endl;
        myfile.close();
    }
}

void vehDist_rsu::recordOnFileMessagesBroadcast(WaveShortMessage* wsm){
    if (strcmp(wsm->getName(), "data") == 0){

        //Open file for just apeend
        myfile.open ("results/RSUBroadcastmessages.txt", std::ios_base::app);

        //Send "strings" to be saved on the file onBeacon_veh.txt
        myfile << "Data from " << wsm->getSenderAddress() << " at " << simTime();
        myfile << " to " << wsm->getRecipientAddress() << endl;
        myfile << "wsm->getName(): " << wsm->getName() << endl;
        myfile << "wsm->getBitLength(): " << wsm->getBitLength() << endl;
        myfile << "wsm->getChannelNumber(): " << wsm->getChannelNumber() << endl;
        myfile << "wsm->getPsid(): " << wsm->getPsid() << endl;
        myfile << "wsm->getPriority(): " << wsm->getPriority() << endl;
        myfile << "wsm->getWsmVersion(): " << wsm->getWsmVersion() << endl;
        myfile << "wsm->getTimestamp(): " << wsm->getTimestamp() << endl;
        myfile << "wsm->getMessageTimestampGenerate(): " << wsm->getMessageTimestampGenerate() << endl;
        myfile << "wsm->getSenderAddress(): " << wsm->getSenderAddress() << endl;
        myfile << "wsm->getHeading(): " << wsm->getHeading() << endl;
        myfile << "wsm->getRecipientAddress(): " << wsm->getRecipientAddress() << endl;
        myfile << "wsm->getSource(): " << wsm->getSource() << endl;
        myfile << "wsm->getTarget(): " << wsm->getTarget() << "findHost()->getFullName()" << findHost()->getFullName() << endl;
        myfile << "wsm->getSenderPos(): " << wsm->getSenderPos() << endl;
        myfile << "wsm->getSerial(): " << wsm->getSerial() << endl;
        myfile << "wsm->getSummaryVector(): " << wsm->getSummaryVector() << endl;
        myfile << "wsm->getRequestMessages(): " << wsm->getRequestMessages() << endl;
        myfile << "wsm->getWsmData(): " << wsm->getWsmData() << endl;
        myfile << "Time to generate and recived: " << (wsm->getTimestamp() - wsm->getMessageTimestampGenerate()) << endl;
        myfile << endl << endl;
        myfile.close();
    }
}
