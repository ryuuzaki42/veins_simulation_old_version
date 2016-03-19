//
// Copyright (C) 2014 Luz Marina Santos
//
#include "application/osdp/osdp.h"
#include <iostream>
#include <string>
#include <time.h>

using Veins::TraCIMobilityAccess;
using Veins::AnnotationManagerAccess;

Define_Module(osdp);

void osdp::initialize(int stage) {
    BaseWaveApplLayer::initialize_default_veins_TraCI(stage);
    query = par("query").longValue();
    timeService = par("timeService").doubleValue();
    timeQuery = par("timeQuery").doubleValue();
    ns=0;
    sendQuery = false;
    LastTime =0;
    lastQuery="servicex";
    if (stage == 0) {
        traci = TraCIMobilityAccess().get(getParentModule());
        outVector.setName("TimeQuery");
        outVector2.setName("TimeContact");
        cont_adv.setName("Advertisements");
        cont_serv.setName("Services");
        s_query.setName("SuccessQuery");
        c_query.setName("CancelledQuery");
        p_query.setName("PlanifiedQuery");
        m_query.setName("MessagesQuery");
        r_query.setName("AnsweredTotalQuery");
        //u_query.setName("UnsuccessfulQuery");
        //traci->getCommandInterface()->setSpeed("query",par("speed").doubleValue());
        //randomQuery();
        deterministQuery();
    }
}

void osdp::deterministQuery() {
    query=par("query").doubleValue();
    //std::cout << "entre a planear la consulta " << query << std::endl;
    qservice="query servicex";
    if (query==1){
         cont_query= timeQuery / par("queryInterval").doubleValue();
         qservice="query service2";
         serviceQueryEvt = new cMessage("service query evt", SERVICE_QUERY_EVT);
         scheduleAt(par("initQuery").doubleValue(), serviceQueryEvt);
         sendQuery = true;
         simtime_t eed = simTime();
         p_query.record(eed);
    }
}

void osdp::randomQuery() {
    int query_car=(rand()%2);
    query=query_car;
    qservice="query servicex";
    if (query==1){
       int query_ser=1+rand()%4;
       int inf,sup;
       inf=20;
       //disminuir tiempo de planificacion, porque puede ser que no alcance
       sup=120;
       int t=rand()%(sup-inf+1)+inf;
       switch (query_ser){
       case 1:
            qservice="query service0";
            break;
       case 2 :
            qservice="query service1";
            break;
       case 3 :
            qservice="query service2";
            break;
       case 4 :
            qservice="query service3";
            break;
       default :
            std::cout << "default servicio para consultar" << std::endl;
       }
            cont_query= timeQuery / par("queryInterval").doubleValue();
            serviceQueryEvt = new cMessage("service query evt", SERVICE_QUERY_EVT);
            scheduleAt(simTime()+t, serviceQueryEvt);
            simtime_t eed = simTime();
            p_query.record(eed);
    }
}

void osdp::onBeacon(WaveShortMessage* wsm) {
    //std::cout << "entering Beacon" << std::endl;
    simtime_t eed = simTime();
    outVector2.record(eed);
}

//Received messages from RSU or other vehicles doing query.
//or the other vehicles answer to query
void osdp::onData(WaveShortMessage* wsm) {
        //Velocity of the vehicle
        // std::ostringstream os;
        //  os << traci->getSpeed();
        // std::string str = os.str();
        //std::cout << str << std::endl;

        opp_string s=wsm->getWsmData();
        //std::string s2= wsm->getFullPath();
        //std::cout << "--" << s2 << std::endl;

        const char* opc = s.c_str();
        std::string op1=opc;
        std::string op2 = op1.substr(0,5);
        std::string op3 = "query";
        int x= strcmp (op2.c_str(),op3.c_str());
        if (x==0) {
            queryService(s.c_str());
        }
        else {
            op3 = "answe";
            x= strcmp (op2.c_str(),op3.c_str());
            if (x!=0) {
                   simtime_t eed = simTime();
                   cont_adv.record(eed);
                   insertService(s.c_str());  }
            else {
                   insertService2(s.c_str());
            }
        }
}

//Received messages from simTime
void osdp::handleSelfMsg(cMessage* msg) {
           //std::string s2= msg->getFullPath();
           //std::cout << "--" << s2 << std::endl;

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
                   simtime_t eed = simTime();
                   m_query.record(eed);
                   sendMessage(qservice.c_str());
                   if (cont_query== timeQuery / par("queryInterval").doubleValue())
                         {   //Registering of query first time
                             simtime_t eed = simTime() - msg->getCreationTime();
                             outVector.record(eed);
                             sendQuery = true;
                         }
                   cont_query --;
                   if (cont_query > 0) {
                          scheduleAt(simTime() + par("queryInterval").doubleValue(), serviceQueryEvt);}
                  break; }
              default :std::cout << "entering default "<< std::endl;
            }
}

void osdp::sendMessage(const char* serv) {
    sentMessage = true;
    t_channel channel = dataOnSch ? type_SCH : type_CCH;
    //Messages of type data
    WaveShortMessage* wsm = prepareWSM("data", dataLengthBits, channel, dataPriority, -1,2);
    wsm->setWsmData(serv);
    sendWSM(wsm);
}

//insert new service from RSU
void osdp::insertService(const char* serv){
       bool exist=false;
       int x;
       for (int i=0; i< ns; i++) {
           x= strcmp (service[i].c_str(), serv);
           if (x==0) exist=true;
       }
       if (!exist){
           service[ns]=serv;
           //std::cout << "adding service de los anuncios " << serv << std::endl;
           ns++;
           simtime_t eed = simTime();
           cont_serv.record(eed);
           serviceExpiredEvt= new cMessage(serv, SERVICE_EXPIRED_EVT);
           scheduleAt(simTime() + timeService, serviceExpiredEvt);
       }
       //else
           //std::cout << "no adding service " << serv << std::endl;
}

//insert new service due to answer of query (var vquery=1), valida si solicito consulta o no
void osdp::insertService2(const char* serv){
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
           serviceExpiredEvt= new cMessage(serv, SERVICE_EXPIRED_EVT);
           scheduleAt(simTime() + timeService, serviceExpiredEvt);
       }
       answerQuery(op4.c_str());
       //else
           //std::cout << "no adding service " << serv << std::endl;
}

void osdp::queryService(const char* serv){
    std::string op1=serv;
    int l=op1.length();
    std::string op2 = op1.substr(6,l);
    //std::cout << "It is looking the service " << op2 << std::endl;
    int x,y;
    for (int i=0; i< ns; i++) {
        std::string op3 = service[i].c_str();
        std::string op4 = op3.substr(0,8);
        //std::cout << "comparing with " << op4 << std::endl;
        x= strcmp (op2.c_str(),op4.c_str());
        std::string answer = "answe "+ op3;
        if (x==0) {
                 simtime_t eed = simTime();
                 //pregunto si es necesario enviar esta respuesta porque ya es alguna que realizo antes.
                 y= strcmp (lastQuery.c_str(),op4.c_str());
                 if ((simTime()-LastTime)> 1 && (y!=0)) {
                      r_query.record(eed);
                      sendMessage(answer.c_str());
                      lastQuery=op4;
                 }
        }
    }
    //si no encontro el servicio en su cache va preguntar si el mismo estaba consultando este servicio
    //agregar tambien si el habia respondido antes el mismo servicio en el ultimo tiempo xxx
    std::string op3 = serv;
    std::string op4 = op3.substr(6, op3.length());
    cancelQuery(op4.c_str());
}

//Deleting service when the SERVICE_EXPIRED_EVT is executed
void osdp::deleteService(const char* serv){
    bool exist=false;
    int x,p;
    for (int i=0; i< ns; i++) {
        x= strcmp (service[i].c_str(), serv);
        if (x==0) {exist=true; p=i;
                break;
        }
    }
    if (exist){
        //std::cout << "deleting service " << serv << " indice de servicio " << p  << std::endl;
        ns--;
        for (int i=p; i<= ns; i++) {
              service[i]=service[i+1];
        }
    }
}

//End answer of query
void osdp::answerQuery(const char* serv){
    std::string op1=serv;
    std::string op2 = op1.substr(0,8);
    //std::string op3 = par("qservice");
    std::string op3 = qservice.c_str();
    std::string op4 = op3.substr(6, op3.length());
    //esta preguntando si el servicio que recibio es el que consulta el mismo
    int x= strcmp (op2.c_str(),op4.c_str());
    if (x==0) {
        std::cout << "successful query response" << std::endl;
        //Registering time of first answer, tambien esta contabilizando otras respuestas que ya almaceno
        //sean parte de la consulta que el hizo o no
        simtime_t eed = simTime();
        s_query.record(eed);
        if (sendQuery== true) {
            simtime_t eed = simTime();
            cont_serv.record(eed);
            cancelEvent(serviceQueryEvt);
            sendQuery=false;
        }
        //qservice="query servicex";
     }
}

//cancela servicio si detecta en la respuesta que tiene planificado una consulta
void osdp::cancelQuery(const char* serv){
    std::string op1=serv;
    std::string op2 = op1.substr(0,8);
    //std::string op3 = par("qservice");
    std::string op3 = qservice.c_str();
    std::string op4 = op3.substr(6, op3.length());
    //esta preguntando si el servicio que recibio es el que consulta
    int x= strcmp (op2.c_str(),op4.c_str());
    if (x==0) {
        //cancela el envio de la consulta
        //std::cout << "entrando a cancelar los mensajes planificados de consulta"  <<op2 << op4<<std::endl;
        cancelEvent(serviceQueryEvt);
        simtime_t eed = simTime();
        c_query.record(eed);
        sendQuery = true;
        //qservice="query servicex";
    }
 }
