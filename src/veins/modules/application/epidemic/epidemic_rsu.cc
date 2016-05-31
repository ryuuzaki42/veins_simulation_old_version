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

        epidemicInitializeVariables();

        cout << source << " myMac: " << myMac << " MACToInteger: " << MACToInteger() << endl;
        cout << "sendSummaryVectorInterval: " << sendSummaryVectorInterval << endl;
        cout << "maximumEpidemicBufferSize: " << maximumEpidemicBufferSize << endl;
        cout << "beaconMessageHopLimit: " << beaconMessageHopLimit << endl;
    }
}

void epidemic_rsu::epidemicInitializeVariables() {
    string seedNumber = ev.getConfig()->getConfigValue("seed-set");
    repeatNumber = atoi(seedNumber.c_str()); // Number of execution (${repetition})

    expNumber = par("expNumber");

    generalInitializeVariables_executionByExpNumberVehDist();

    string folderResult = "results/epidemic_resultsEnd/E" + to_string(expNumber);
    folderResult += "_" + to_string((static_cast<int>(ttlBeaconMessage))) + "_" + to_string(countGenerateBeaconMessage) +"/";

    restartFilesResultRSU(folderResult);
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
