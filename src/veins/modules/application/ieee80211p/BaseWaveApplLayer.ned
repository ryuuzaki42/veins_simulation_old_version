//
// Copyright (C) 2011 David Eckhoff <eckhoff@cs.fau.de>
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

package org.car2x.veins.modules.application.ieee80211p;
import org.car2x.veins.base.modules.IBaseApplLayer;

//
// WAVE application layer base class.
//
// @author David Eckhoff
//
// @see BaseWaveApplLayer
// @see Nic80211p
// @see Mac1609_4
// @see PhyLayer80211p
//
simple BaseWaveApplLayer like IBaseApplLayer {
    parameters:
        bool debug = default(false); //whether to print debug output
        int headerLength = default(88bit) @unit(bit); //header length of the application

        bool sendBeacons = default(false); //tell the applayer to periodically send beacons
        int beaconLengthBits = default(256bit) @unit(bit); //the length of a beacon packet
        int beaconPriority = default(3); //the access category of the beacon messages
        volatile double beaconInterval = default(1s) @unit(s); //the intervall between 2 beacon messages

        double maxOffset = default(0s) @unit(s); //the maximum delay of messages sent down to avoid synchronous channel access
        bool sendData = default(false); //whether this applayer should send data
        int dataLengthBits = default(1024bit) @unit(bit); //the length of a data packet
        bool dataOnSch = default(true); //tells the applayer whether to use a service channel for datapackets or the control channel
        int dataPriority = default(3); //the access category for data packets

        // Add for Epidemic
        int sendSummaryVectorInterval @unit("s") = default(0s); //define the minimum slide window length among contacts to send new version of summary vector
        int maximumEpidemicBufferSize = default(0); //define the maximum buffer size (in number of messages) that a node is willing to allocate for epidemic messages.
        int hopCount = default(0); //define the maximum number of hopes that a message can be forward before reach the target

        int maximumMfcvEpidemicBufferSize = default(0); //define the maximum buffer size (in number of messages) that a node is willing to allocate for mfcv_epidemic messages.

        //@signal[numberHops](type="long");
        //@statistic[hopCount](title="Hop Count"; source="numberHops"; record=vector,histogram,stats);

        //@signal[delay](type="double");
        //@statistic[delayTime](source=delay; title="End-to-End Delay"; record=stats,histogram,vector);

        //@signal[messageReceived](type="long");
        //@statistic[numMessageReceived](source=sum(messageReceived); title="Total Number or Message Received"; record=stats,histogram,vector);

        //jonh test
        int vehBeaconMessageTarget_x = default(0);
        int vehBeaconMessageTarget_y = default(0);

        int beaconMessageBufferSize = default(50);
        int beaconMessageInterval @unit("s") = default(1s); // interval to try send message(s)
        int beaconMessageHopLimit = default(0); // Limit of hops
        int timeGenerateBeaconMessage @unit("s") = default(30s); // time for generate messages

        int ttlBeaconMessage_one @unit("s") = default(60s); // ttl beacon message
        int ttlBeaconMessage_two @unit("s") = default(60s);
        int countGenerateBeaconMessage_one = default(0); // count messages generate by time
        int countGenerateBeaconMessage_two = default(0);

        int experimentNumber = default(0); // Experiment number by table 2^k, number to use _one or _two variables
        int ttlBeaconStatus @unit("s") = default(3s);
        int beaconStatusBufferSize = default(50);
        int vehTimeUpdatePosition @unit("s") = default(1s); //time for update vehicle position

    gates:
        input lowerLayerIn; // from mac layer
        output lowerLayerOut; // to mac layer
        input lowerControlIn;
        output lowerControlOut;
}
