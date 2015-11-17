//
// Copyright (C) 2014 Luz Marina Santos
//

#include "application/osdp/osdp_RSU.h"

using Veins::AnnotationManagerAccess;

Define_Module(osdp_RSU);

void osdp_RSU::initialize(int stage) {
    serv1 = par("serv1").longValue();
    serv2 = par("serv2").longValue();
    BaseWaveApplLayer::initialize_osdp(stage);
    std::cout << "Starting services " << serv1 << " - "<< serv2 << std::endl;
    setServices();
    sleep = 1;
    if (stage == 0) {
            //Register of sent advertisements by the RSU
           sent_adv.setName("SentAdvertisements");
        sendServiceEvt = new cMessage("service evt", SERVICE_EVT);
        scheduleAt(simTime() + par("serviceInterval").doubleValue(), sendServiceEvt);
        mobi = dynamic_cast<BaseMobility*> (getParentModule()->getSubmodule("mobility"));
        ASSERT(mobi);
        annotations = AnnotationManagerAccess().getIfExists();
        ASSERT(annotations);
    }
}

void osdp_RSU::onBeacon(WaveShortMessage* wsm) {
    //Code to realize by the RSU when it receives a beacon
   
}

void osdp_RSU::onData(WaveShortMessage* wsm) {
    //Code to realize by the RSU when it receives a data
}

void osdp_RSU::handleSelfMsg(cMessage* msg) {
    //Code to realize by the RSU when it receives scheduled events
    switch (msg->getKind()) {
         case SEND_BEACON_EVT: {
             sendWSM(prepareWSM("beacon", beaconLengthBits, type_CCH, beaconPriority, 0, -1));
             scheduleAt(simTime() + par("beaconInterval").doubleValue(), sendBeaconEvt);
             break;
         }
         //Add the next event, the RSUs is charged with the services from serv1 to serv2 in omnetpposdp.ini
         case SERVICE_EVT: {
             for (int i=serv1-1; i< serv2; i++) {
                   sendMessage(service[i].c_str()); }
             scheduleAt(simTime() + par("serviceInterval").doubleValue(), sendServiceEvt);
             break;
         }
         default: {
             if (msg)
                 DBG << "APP: Error: Got Self Message of unknown kind! Name: " << msg->getName() << endl;
                 break;
         }
     }
}

void osdp_RSU::sendMessage(const char* serv) {
    sentMessage = true;
    t_channel channel = dataOnSch ? type_SCH : type_CCH;
    //Message of type data
    WaveShortMessage* wsm = prepareWSM("data", dataLengthBits, channel, dataPriority, -1,2);
    //add the next line with the information of the service in the field WsmData of WaveShortMessage
    wsm->setWsmData(serv);
    sendWSM(wsm);
}

void osdp_RSU::sendWSM(WaveShortMessage* wsm) {
    sendDelayedDown(wsm,individualOffset);
}

void osdp_RSU::setServices() {
    //format of service, "type description address"
    service[0]="service0 description1 address1";
    service[1]="service1 description2 address2";
    service[2]="service2 description3 address3";
    service[3]="service3 description4 address4";
    service[4]="service0  description5 address5";
    service[5]="service1  description6 address6";
    service[6]="service1  description7 address8";
    service[7]="service1  description8 address8";
    service[8]="service1  description9 address9";
    service[9]="service1  description10 address10";
    service[10]="service2 description11 address11";
    service[11]="service2 description12 address12";
    service[12]="service2 description13 address13";
    service[13]="service2 description14 address14";
    service[14]="service2 description15 address15";
    service[15]="service3 description16 address16";
    service[16]="service3 description17 address17";
    service[17]="service3 description18 address18";
    service[18]="service3 description19 address19";
    service[19]="service3 description20 address20";
    service[20]="service0 description21 address21";
    service[21]="service1 description22 address22";
    service[22]="service2 description23 address23";
    service[23]="service3 description24 address24";
    service[24]="service0 description25 address25";
    service[25]="service1 description26 address26";
    service[26]="service2 description27 address27";
    service[27]="service3 description28 address28";
    service[28]="service0 description29 address29";
    service[29]="service1 description30 address30";
    service[30]="service2 description31 address31";
    service[31]="service3 description32 address32";
    service[32]="service0 description33 address33";
    service[33]="service1 description34 address34";
    service[34]="service2 description35 address35";
    service[35]="service3 description36 address36";
    service[36]="service0 description37 address37";
    service[37]="service1 description38 address38";
    service[38]="service2 description39 address39";
    service[39]="service3 description40 address40";
}
