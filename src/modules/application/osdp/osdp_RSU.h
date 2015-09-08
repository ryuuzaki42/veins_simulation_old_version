//
// Copyright (C) 2014 Luz Marina Santos
//

#ifndef osdp_RSU_H
#define osdp_RSU_H

#include "BaseWaveApplLayer.h"
#include "modules/world/annotations/AnnotationManager.h"

using Veins::AnnotationManager;

/**
 * Small RSU Demo using 11p
 */
class osdp_RSU : public BaseWaveApplLayer {
	public:
		virtual void initialize(int stage);
		opp_string service[40];
		int sleep;

	protected:
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

};

#endif
