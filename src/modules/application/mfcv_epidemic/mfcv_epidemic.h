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

#ifndef mfcv_epidemic_H
#define mfcv_epidemic_H


#include <unordered_map>
#include "BaseWaveApplLayer.h"
#include "modules/mobility/traci/TraCIMobility.h"

using Veins::TraCIMobility;
using Veins::AnnotationManager;

using namespace std;

/**
 * Small IVC Demo using 11p
 */
class mfcv_epidemic : public BaseWaveApplLayer {
    public:
        virtual void initialize(int stage);
        virtual void receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj);
    protected:
        TraCIMobility* traci;
        AnnotationManager* annotations;
        simtime_t lastDroveAt;
        bool isParking;
        bool sendWhileParking;
        static const simsignalwrap_t parkingStateChangedSignal;
        //To store the number of recent contacts of a mobile node
        int NumberOfContacts[50];
        //Unique 16bits identification of a message
        static unsigned short int messageId;
        //Hash table to represent the node buffer. It stores messages generated by itself as well messages carried on behalf of other nodes
        typedef unordered_map<string, WaveShortMessage> MyMapMfcv_EpidemicMessageBuffer;
        typedef pair<string, WaveShortMessage> MyPairMfcv_EpidemicMessageBuffer;
        //Creating a unordered_map to represent the local mfcv_epidemic messages buffer
        MyMapMfcv_EpidemicMessageBuffer mfcv_epidemicLocalMessageBuffer;
        //Bit Vector to represent a summary vector that indicates which entries in their local hash table are setted.
        unordered_map<string, bool> mfcv_epidemicLocalSummaryVector;
        //Bit Vector to represent a summary vector that indicates which entries in remote hash table are setted.
        unordered_map<string, bool> mfcv_epidemicRemoteSummaryVector;
        //Bit Vector to represent a result summary vector that will be used to make a resquest of messages.
        unordered_map<string, bool> mfcv_epidemicRequestMessageVector;
        //bitset<BROADCAST> mfcv_epidemicSummaryVector;
        //Cache with nodes that I recently met
        unordered_map<unsigned int, simtime_t> nodesRecentlyFoundList;
        //Cache with nodes that I recently sent the summary vector
        unordered_map<unsigned int, simtime_t> nodesIRecentlySentSummaryVector;
        //implementation of FIFO in order to maintain the limited length of the buffer activated
        queue<string> queueFIFO;

        // record in file
        std::ofstream myfile;

    protected:
        virtual void onBeacon(WaveShortMessage* wsm);
        virtual void onData(WaveShortMessage* wsm);
        void sendLocalSummaryVector(unsigned int newRecipientAddress);
        void sendMfcv_EpidemicRequestMessageVector(unsigned int newRecipientAddress);
        void sendMessagesRequested(string s, unsigned int recipientAddress);
        void sendBeacon();
        void sendBeacon(unsigned int target);
        virtual void sendWSM(WaveShortMessage* wsm);
        virtual void handlePositionUpdate(cObject* obj);
        virtual void handleParkingUpdate(cObject* obj);
        //Return a string with the whole summaryvector
        string getLocalSummaryVectorData();
        string getMfcv_EpidemicRequestMessageVectorData();
        WaveShortMessage getMfcv_EpidemicLocalMessageBuffer(string s);
        void printNodesIRecentlySentSummaryVector();
        void printMfcv_EpidemicLocalMessageBuffer();
        void printMfcv_EpidemicRequestMessageVector();
        void printWaveShortMessage(WaveShortMessage wsm);
        void printWaveShortMessage(WaveShortMessage* wsm);
        void printMfcv_EpidemicLocalSummaryVectorData();
        void printMfcv_EpidemicRemoteSummaryVectorData();
        void createMfcv_EpidemicRequestMessageVector();
        void printQueueFIFO(queue<string> qFIFO);
        void createMfcv_EpidemicRemoteSummaryVector(string s);

        //To manipulate self messages
        //virtual void handleSelfMsg(cMessage* msg);
        void updateNumberOfContacts();

        //generate a target to send a message
        void generateTarget();
        //generate a message to send a message
        void generateMessage();

        virtual void finish();

        //test Jonh
        void recordOnFile(WaveShortMessage* wsm);
        void printMfcv_EpidemicLocalMessageBufferOnFile();
        unsigned int getHeading();
        void handleSelfMsg(cMessage* msg);
        WaveShortMessage* prepareWSM_mfcv_epidemic(std::string name, int lengthBits, t_channel channel, int priority, unsigned int rcvId, int serial);
};

unsigned short int mfcv_epidemic::messageId = 0;

// test Jonh
unsigned short int BaseWaveApplLayer::vehCount= 0;

#endif
