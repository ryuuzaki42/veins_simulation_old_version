#include "Flooding.h"

const simsignalwrap_t Flooding::mobilityStateChangedSignal = simsignalwrap_t(MIXIM_SIGNAL_MOBILITY_CHANGE_NAME);

Define_Module(Flooding);

void Flooding::initialize(int stage) {
	BaseApplLayer::initialize(stage);

	if (stage==0) {
		myMac = FindModule<WaveAppToMac1609_4Interface*>::findSubModule(
		            getParentModule());
		assert(myMac);

		myId = getParentModule()->getIndex();

		headerLength = par("headerLength").longValue();
		double maxOffset = par("maxOffset").doubleValue();
		beaconLengthBits = par("beaconLengthBits").longValue();
		beaconPriority = par("beaconPriority").longValue();

		sendData = par("sendData").boolValue();
		dataLengthBits = par("dataLengthBits").longValue();

		//simulate asynchronous channel access
		double offSet = dblrand() * (par("beaconInterval").doubleValue()/2);
		offSet = offSet + floor(offSet/0.050)*0.050;
		individualOffset = dblrand() * maxOffset;

		findHost()->subscribe(mobilityStateChangedSignal, this);

		generateMessageEvt = new cMessage("generate Message", GENERATE_MESSAGE);

		// TODO: Select a random node (myId == ?) with some random delay to generate and broadcast (using generateMessageEvt) a message once during the simulation.

		// test Jonh
		//std::cout << findHost()->getFullName() << findHost()->getFullName() << endl;
		std::string source = findHost()->getFullName();
		if (source.compare("node[0]")){
		    std::cout << findHost()->getFullName() << findHost()->getFullName() << endl;
		    scheduleAt(simTime() + offSet, generateMessageEvt);
		}


	}
}

FloodingMessage*  Flooding::prepareWSM(std::string name, int lengthBits, t_channel channel, int priority, int rcvId, int serial) {
    FloodingMessage* flm = new FloodingMessage(name.c_str());
    flm->addBitLength(headerLength);
    flm->addBitLength(lengthBits);

	switch (channel) {
		case type_SCH: flm->setChannelNumber(Channels::SCH1); break; //will be rewritten at Mac1609_4 to actual Service Channel. This is just so no controlInfo is needed
		case type_CCH: flm->setChannelNumber(Channels::CCH); break;
	}
	flm->setPsid(0);
	flm->setPriority(priority);
	flm->setWsmVersion(1);
	flm->setTimestamp(simTime());
	flm->setSenderAddress(myId);
	flm->setRecipientAddress(rcvId);
	flm->setSenderPos(curPosition);
	flm->setSerial(serial);

	if (name == "beacon") {
		DBG << "Creating Beacon with Priority " << priority << " at Applayer at " << flm->getTimestamp() << std::endl;
	}
	if (name == "data") {
		DBG << "Creating Data with Priority " << priority << " at Applayer at " << flm->getTimestamp() << std::endl;
	}

	return flm;
}

void Flooding::receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj) {
	Enter_Method_Silent();
	if (signalID == mobilityStateChangedSignal) {
		handlePositionUpdate(obj);
	}
}

void Flooding::handlePositionUpdate(cObject* obj) {
	ChannelMobilityPtrType const mobility = check_and_cast<ChannelMobilityPtrType>(obj);
	curPosition = mobility->getCurrentPosition();
}

void Flooding::handleLowerMsg(cMessage* msg) {

	FloodingMessage* flm = dynamic_cast<FloodingMessage*>(msg);
	ASSERT(flm);

	// TODO: here the magic happens ;-) ideally you need exactly 13 characters to implement flooding, maybe a lot more for the statistics and to avoid continuous rebroadcasts

}

void Flooding::handleSelfMsg(cMessage* msg) {
	switch (msg->getKind()) {
		case GENERATE_MESSAGE: {
			sendWSM(generateMessage());
			break;
		}
		default: {
			if (msg)
				DBG << "APP: Error: Got Self Message of unknown kind! Name: " << msg->getName() << endl;
			break;
		}
	}
}

void Flooding::sendWSM(WaveShortMessage* wsm) {
	sendDelayedDown(wsm,individualOffset);
}

void Flooding::finish() {

	// TODO Store scalars here

	findHost()->unsubscribe(mobilityStateChangedSignal, this);

}

FloodingMessage* Flooding::generateMessage() {

    FloodingMessage* flm = prepareWSM("flm", dataLengthBits, type_CCH, beaconPriority, 0, -1);
    flm->setMsgId(intuniform(0, 1000));

    return flm;
}

Flooding::~Flooding() {

}
