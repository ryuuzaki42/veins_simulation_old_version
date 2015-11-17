//
// Copyright (C) 2015 Jpão Batista and Wellington Branquinho
//

#ifndef test1_H
#define test1_H

#include "BaseWaveApplLayer.h"
#include "modules/mobility/traci/TraCIMobility.h"
#include "modules/mobility/traci/TraCICommandInterface.h"

using Veins::TraCIMobility;
using Veins::AnnotationManager;
using Veins::TraCICommandInterface;

/**
 * Small IVC Demo using 11p
 */
class test1 : public BaseWaveApplLayer {
    public:
        virtual void initialize(int stage);
        opp_string service[640];
        int ns;
        static int nq;
    protected:
        TraCIMobility* mobility;
        TraCICommandInterface* traci;
        bool sentMessage, sendQuery;
        int query, cont_query, c;
        opp_string qservice;
        double queryInterval;
        double timeQuery;
        double timeService;
        double initQuery;
        int receiver;
        simtime_t t;
        cMessage* serviceQueryEvt;
        cMessage* serviceExpiredEvt;
        cMessage* mobilityEvt;
        cOutVector cont_beac, cont_adv, cont1_adv, cont2_adv, cont3_adv, c_query, p_query, cont_del,us_query, m_query,a_query;
        cOutVector  s_query,s1_query,s2_query,s3_query, cont_serv, cont1_serv, cont2_serv, cont3_serv, store_serv;

    protected:
        virtual void onBeacon(WaveShortMessage* wsm);
        virtual void onData(WaveShortMessage* wsm);
        void sendMessage(const char* serv);
        virtual void handleSelfMsg(cMessage* msg);

    public:
        void insertService(const char* serv);
        void insertService2(const char* serv);
        void queryService(const char* serv);
        void answerQuery(const char* serv);
        void cancelQuery(const char* serv);
        void deleteService(const char* serv);
        void contService(const char* cat);
        void deterministQuery();
};

#endif
