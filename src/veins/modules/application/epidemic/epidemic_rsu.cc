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

        epidemic_InitializeVariables();

        std::cout << "I'm " << source <<  " myMac: " << myMac << " MACToInteger: " << MACToInteger() << endl;
        cout << "sendSummaryVectorInterval: " << sendSummaryVectorInterval << endl;
        cout << "maximumEpidemicBufferSize: " << maximumEpidemicBufferSize << endl;
        cout << "beaconMessageHopLimit: " << beaconMessageHopLimit << endl;
    }
}

void epidemic_rsu::epidemic_InitializeVariables() {
    string seedNumber = ev.getConfig()->getConfigValue("seed-set");
    repeatNumber = atoi(seedNumber.c_str()); // number of execution (${repetition})

    expNumber = par("expNumber");

    string resultFolder = "results/epidemic_resultsEnd/E" + to_string(expNumber);
    resultFolder += "_" + to_string((static_cast<int>(ttlBeaconMessage))) + "_" + to_string(countGenerateBeaconMessage) +"/";

    fileMessagesBroadcast = fileMessagesUnicast = fileMessagesCount = resultFolder + source;

    fileMessagesBroadcast += "_Broadcast_Messages.r";
    fileMessagesUnicast += "_Messages_Received.r";
    fileMessagesCount += "_Count_Messages_Received.r";

    //fileMessagesDrop and fileMessagesGenerated not used yet to RSU

    if ((myId == 0) && (repeatNumber == 0)) { //Open a new file (blank)
        if (expNumber <= 4) { // maxSpeed 15 m/s
            string comand = "sed -i 's/maxSpeed=.* color/maxSpeed=\"15\" color/g' vehDist.rou.xml";
            system(comand.c_str());
            cout << endl << "Change the spped to 15 m/s, command: " << comand << endl;
        } else if (expNumber >= 5){ // maxSpeed 25 m/s
            string comand = "sed -i 's/maxSpeed=.* color/maxSpeed=\"25\" color/g' vehDist.rou.xml";
            system(comand.c_str());
            cout << endl << "Change the spped to 25 m/s, command: " << comand << endl;
        }

        string commadCreateFolder = "mkdir -p " + resultFolder + " > /dev/null";
        cout << endl << "Created the folder, command: \"" << commadCreateFolder << "\"" << endl;
        cout << "repeatNumber " << repeatNumber << endl;
        system(commadCreateFolder.c_str()); //create a folder results

        openFileAndClose(fileMessagesBroadcast, false, ttlBeaconMessage, countGenerateBeaconMessage);
        openFileAndClose(fileMessagesUnicast, false, ttlBeaconMessage, countGenerateBeaconMessage);
        openFileAndClose(fileMessagesCount, false, ttlBeaconMessage, countGenerateBeaconMessage);
    } else { // (repeatNumber != 0)) // for just append
        openFileAndClose(fileMessagesBroadcast, true, ttlBeaconMessage, countGenerateBeaconMessage);
        openFileAndClose(fileMessagesUnicast, true, ttlBeaconMessage, countGenerateBeaconMessage);
        openFileAndClose(fileMessagesCount, true, ttlBeaconMessage, countGenerateBeaconMessage);
    }
}

void epidemic_rsu::onBeacon(WaveShortMessage* wsm) {
    receivedOnBeacon(wsm);
}

void epidemic_rsu::onData(WaveShortMessage* wsm) {
    receivedOnData(wsm);
}

void epidemic_rsu::finish() {
    toFinishRSU();
}
