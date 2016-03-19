//
// Copyright (C) 2014 Luz Marina Santos
//

#ifndef TraCIDemo11p_H
#define TraCIDemo11p_H

#include "BaseWaveApplLayer.h"
#include "modules/mobility/traci/TraCIMobility.h"

using Veins::TraCIMobility;
using Veins::AnnotationManager;

/**
 * Small IVC Demo using 11p
 */
class osdp : public BaseWaveApplLayer {
    public:
        virtual void initialize(int stage);
        opp_string service[40], lastQuery;
        int ns;

    protected:
        TraCIMobility* traci;
        bool sentMessage, sendQuery;
        int query, cont_query;
        opp_string qservice;
        double queryInterval;
        double timeQuery;
        double timeService;
        double LastTime;
        double initQuery;
        //double speed;
        cMessage* serviceQueryEvt;
        cMessage* serviceExpiredEvt;
        cOutVector outVector, outVector2, cont_serv, cont_adv, s_query, c_query, p_query, m_query, r_query;

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
        void randomQuery();
        void deterministQuery();
};

#endif
