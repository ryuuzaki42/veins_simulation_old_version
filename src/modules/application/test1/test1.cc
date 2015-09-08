//
//  Copyright (C) 2015 João Batista and Wellington Branquinho
//

#include "application/test1/test1.h"
#include <iostream>
#include <string>
#include <time.h>

#include <stdio.h>

using Veins::TraCIMobilityAccess;
using Veins::AnnotationManagerAccess;

Define_Module(test1);

void test1::initialize(int stage) {
    BaseWaveApplLayer::initialize_test1(stage);
    //Define t for contact-time in advertisements
    //t=34;
    ns=0;
    if (stage == 0) {
        mobility = TraCIMobilityAccess().get(getParentModule());
        //Defining names to the results
        cont_beac.setName("ReceivedBeacons");  //received beacons of safety
        cont_adv.setName("ReceivedAdvert");    //received advertisements
        cont1_adv.setName("ReceivedAdvert40");
        cont2_adv.setName("ReceivedAdvert80");
        cont3_adv.setName("ReceivedAdvert120");
        cont_serv.setName("StoredServices");   //stored services of queries realized by other vehicles
        cont1_serv.setName("StoredServices40");
        cont2_serv.setName("StoredServices80");
        cont3_serv.setName("StoredServices120");
        p_query.setName("PlanifiedQuery");     //planified = success + unsuccess + cancelled
        s_query.setName("SuccessQuery");
        s1_query.setName("SuccessQuery40");
        s2_query.setName("SuccessQuery80");
        s3_query.setName("SuccessQuery120");
        us_query.setName("UnsuccessfullQuery");
        c_query.setName("CancelledQuery");
        cont_del.setName("ErasedServices");    //erased services on the cache by timelife=0
        m_query.setName("RequestedQuery");
        a_query.setName("AnsweredQuery");
        store_serv.setName("storedCache");
        //taking parameters from omnetpposdp.ini
        qservice = par("qservice").stringValue();
        query = par("query").doubleValue();
        //deterministQuery();
    }
}

//method for scheduling of query
void test1::deterministQuery() {
       std::string road=mobility->getRoadId().c_str();
       int x= strcmp (road.c_str(),"21ato20a");
       int y=myId;
       if (x==0)
            std::cout << myId << "dere" << road << x <<  std::endl;
       else
            std::cout << myId << "izqu" << road << x << std::endl;

       if (x==0)
       {
           if ( y==1 || y==20 || y==38 || y==56 || y==74 || y==92  || y==110 || y==128 || y==146 || y==164
             || y==7 || y==25 || y==43 || y==61 || y==79 || y==97  || y==115 || y==133 || y==151 || y==169
            || y==12 || y==30 || y==48 || y==66 || y==84 || y==102 || y==120 || y==138 || y==156 || y==174) {
              query=1;
       } } //5s

       /*if (x==0)
       {
                 if ( y==1 || y==17 || y==26 || y==38 || y==53 || y==62  || y==80 || y==92 || y==110 || y==125
                   || y==6 || y==19 || y==28 || y==43 || y==55 || y==70  || y==82 || y==97 || y==115 || y==127
                  || y==9 || y==24 || y==33 || y==45 || y==60 || y==72 || y==87 || y==105 || y==117 || y==132) {
                    query=1;
       } } //10s*/

       if (query==1){
               //random time
               //int time = (rand()%400)+1;
               //initQuery = simTime().dbl()+1400.1;
               initQuery = simTime().dbl()+1400;
               std::cout << initQuery << "query" << std::endl;
               timeQuery = par("timeQuery").doubleValue();
               cont_query= timeQuery / par("queryInterval").doubleValue();
               serviceQueryEvt = new cMessage("service query evt", SERVICE_QUERY_EVT);
               scheduleAt(initQuery, serviceQueryEvt);
               p_query.record(simTime());
               //new
               query=0;
      }
}

void test1::onBeacon(WaveShortMessage* wsm) {
     cont_beac.record(simTime());
}

//Received messages from RSU or other vehicles doing query.
//or from other vehicles answering to the query
void test1::onData(WaveShortMessage* wsm) {
        opp_string s=wsm->getWsmData();
        const char* opc = s.c_str();
        std::string op1=opc;
        std::string op2 = op1.substr(0,5);
        int x= strcmp (op2.c_str(),"query");

        if (x==0) {
            receiver=wsm->getSenderAddress();
            queryService(s.c_str());
        }
        else {
            x= strcmp (op2.c_str(),"answe");
            if (x!=0) {
                  insertService(s.c_str());  }
            else {
                 receiver=wsm->getRecipientAddress();
                 insertService2(s.c_str());}
        }
}

//Received messages from simTime
void test1::handleSelfMsg(cMessage* msg) {
              switch (msg->getKind()) {
              case SEND_BEACON_EVT: {
                  sendWSM(prepareWSM("beacon", beaconLengthBits, type_CCH, beaconPriority, 0, -1));
                  scheduleAt(simTime() + par("beaconInterval").doubleValue(), sendBeaconEvt);
                  break;
              }
              case SERVICE_EXPIRED_EVT: {
                  std::string s= msg->getFullName();
                  deleteService(s.c_str());
                  break;
              }
              case SERVICE_QUERY_EVT : {
                  if (cont_query== timeQuery / par("queryInterval").doubleValue())
                  {
                      int service = (rand()%5);
                      std::stringstream ss;
                      ss << service;
                      qservice = "query service"+ss.str();
                      contService(qservice.c_str());
                      //new
                      query=1;
                   }
                   m_query.record(simTime());
                   dataLengthBits=14*8;
                   receiver=-1;
                   sendMessage(qservice.c_str());
                   cont_query --;
                   if (cont_query > 0) {
                           scheduleAt(simTime() + par("queryInterval").doubleValue(), serviceQueryEvt);}
                   else  { qservice="query servicex";
                           us_query.record(simTime());
                   }
                   break;
              }
               default :std::cout << "entering default "<< std::endl;
            }
}

void test1::contService(const char* cat){
        std::string op1 = cat;
        std::string op2 = op1.substr(0,8);
        std::string op3,op4;
        int y;
        for (int i=0; i< ns; i++) {
             op3=service[i].c_str();
             op4 = op3.substr(0,8);
             y= strcmp (op2.c_str(), op4.c_str());
             if (y==0) store_serv.record(simTime());
        }
}

void test1::sendMessage(const char* serv) {
    sentMessage = true;
    t_channel channel = dataOnSch ? type_SCH : type_CCH;
    //Messages of type data, add data
    WaveShortMessage* wsm = prepareWSM("data", dataLengthBits, channel, dataPriority, receiver ,2);
    wsm->setWsmData(serv);
    sendWSM(wsm);
}

//insert new service from RSU
void test1::insertService(const char* serv){
       double speed=mobility->getSpeed();
       bool exist=false;
       int x;
       for (int i=0; i< ns; i++) {
           x= strcmp (service[i].c_str(), serv);
           if (x==0) exist=true;
       }
       if (!exist){
           service[ns]=serv;
           ns++;
           cont_adv.record(simTime());
           if (speed<18)
                 cont1_adv.record(simTime());
           else
                 if (speed <25)
                     cont2_adv.record(simTime());
                 else
                     cont3_adv.record(simTime());
           serviceExpiredEvt= new cMessage(serv, SERVICE_EXPIRED_EVT);
           std::cout << speed << std::endl;
           timeService = par("timeService").doubleValue()/speed;
           scheduleAt(simTime() + timeService, serviceExpiredEvt);
       }
}

//insert new service from other vehicle
void test1::insertService2(const char* serv){
       bool exist=false;
       int x;
       std::string op3 = serv;
       std::string op4 = op3.substr(6, op3.length());
       for (int i=0; i< ns; i++) {
           x= strcmp (service[i].c_str(), op4.c_str());
           if (x==0) exist=true;
       }
       if (!exist){
           service[ns]=op4;
           ns++;
           serviceExpiredEvt= new cMessage(op4.c_str(), SERVICE_EXPIRED_EVT);
           timeService = par("timeService").doubleValue();
           std::cout << timeService << std::endl;
           scheduleAt(simTime() + timeService, serviceExpiredEvt);
           answerQuery(op4.c_str());
       }
}

//query of the service in the cache of the vehicle
void test1::queryService(const char* serv){
    std::string op1=serv;
    std::string op2 = op1.substr(6,op1.length());
    std::string op3,op4;
    int x;
    dataLengthBits=4096;
    for (int i=0; i< ns; i++) {
        op3 = service[i].c_str();
        op4 = op3.substr(0,8);
        x= strcmp (op2.c_str(),op4.c_str());
        std::string answer = "answe "+ op3;
        if (x==0) {
             a_query.record(simTime());
             sendMessage(answer.c_str());
        }
    }
}

//Deleting service when the SERVICE_EXPIRED_EVT is executed
void test1::deleteService(const char* serv){
    bool exist=false;
    int x,p;
    for (int i=0; i< ns; i++) {
        x= strcmp (service[i].c_str(), serv);
        if (x==0) {
             exist=true;
             p=i;
             break;
        }
    }
    if (exist){
        ns--;
        cont_del.record(simTime());
        for (int i=p; i<= ns; i++) {
              service[i]=service[i+1];
        }
    }
}

//identify successful query
void test1::answerQuery(const char* serv){
    double speed=mobility->getSpeed();
    if (receiver==myId)
    {
        s_query.record(simTime());
        if (query==1) {
             cancelEvent(serviceQueryEvt);
             query=0;
        }
        if (speed<18)
            s1_query.record(simTime());
        else
            if (speed <25)
              s2_query.record(simTime());
            else
              s3_query.record(simTime());
     }
     else
     {
          cont_serv.record(simTime());
          if (speed<18)
              cont1_serv.record(simTime());
          else
              if (speed <25)
                   cont2_serv.record(simTime());
              else
                   cont3_serv.record(simTime());
     }
}

//delete scheduled query-event
void test1::cancelQuery(const char* serv){
    std::string op3 = qservice.c_str();
    std::string op4 = op3.substr(6, op3.length());
    int x= strcmp (serv,op4.c_str());
    if (x==0 && query == 1) {
        c_query.record(simTime());
        cancelEvent(serviceQueryEvt);
        qservice="query servicex";
    }
 }
