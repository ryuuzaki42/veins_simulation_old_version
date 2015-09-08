//
//  Copyright (C) 2015 João Batista and Wellington Branquinho
//

#ifndef test1_H
#define test1_H

#include "BaseWaveApplLayer.h"
#include "modules/world/annotations/AnnotationManager.h"

using Veins::AnnotationManager;

/**
 * Small RSU Demo using 11p
 */
class test1_rsu : public BaseWaveApplLayer {
	public:
		virtual void initialize(int stage);
		opp_string service[240];
		int sleep;

	protected:
		simtime_t t;
		AnnotationManager* annotations;
		BaseMobility* mobi;
		bool sentMessage;
		int serv1, serv2;
		cMessage* sendServiceEvt;
		cOutVector sent_adv;


	protected:
		virtual void onBeacon(WaveShortMessage* wsm);
		virtual void onData(WaveShortMessage* wsm);
		void sendMessage(const char* serv);
		virtual void sendWSM(WaveShortMessage* wsm);
		virtual void handleSelfMsg(cMessage* msg);


	public:
		void setServices();
		std::string prepareServices(int x);
};

#endif
