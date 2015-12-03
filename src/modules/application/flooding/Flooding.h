#ifndef FLOODING_H_
#define FLOODING_H_

#include <map>
#include <BaseApplLayer.h>
#include <Consts80211p.h>
//#include <WaveShortMessage_m.h>
#include <FloodingMessage_m.h>
#include "base/connectionManager/ChannelAccess.h"
#include <WaveAppToMac1609_4Interface.h>

#ifndef DBG
#define DBG EV
#endif

class Flooding : public BaseApplLayer {

	public:
		~Flooding();
		virtual void initialize(int stage);
		virtual void finish();

		virtual  void receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj);

		enum WaveApplMessageKinds {
			SERVICE_PROVIDER = LAST_BASE_APPL_MESSAGE_KIND,
			SEND_BEACON_EVT, GENERATE_MESSAGE
		};

	protected:

		static const simsignalwrap_t mobilityStateChangedSignal;

		/** @brief handle messages from below */
		virtual void handleLowerMsg(cMessage* msg);
		/** @brief handle self messages */
		virtual void handleSelfMsg(cMessage* msg);

		virtual FloodingMessage* prepareWSM(std::string name, int dataLengthBits, t_channel channel, int priority, int rcvId, int serial=0);
		virtual void sendWSM(WaveShortMessage* wsm);

		virtual void handlePositionUpdate(cObject* obj);

		virtual FloodingMessage* generateMessage();

	protected:
		int beaconLengthBits;
		int beaconPriority;
		bool sendData;
		bool sendBeacons;
		simtime_t individualOffset;
		int dataLengthBits;
		bool dataOnSch;
		int dataPriority;
		Coord curPosition;
		int mySCH;
		int myId;

		cMessage* sendBeaconEvt;
		cMessage* generateMessageEvt;

		WaveAppToMac1609_4Interface* myMac;
};

#endif /* FLOODING_H_ */
