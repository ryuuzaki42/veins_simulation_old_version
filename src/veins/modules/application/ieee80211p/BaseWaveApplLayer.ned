// Copyright (C) 2015-2016 João Batista <joao.b@usp.br>

package org.car2x.veins.modules.application.ieee80211p;
import org.car2x.veins.base.modules.IBaseApplLayer;

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

        // Jonh added
        int vehBeaconMessageTarget_x = default(0);
        int vehBeaconMessageTarget_y = default(0);

        int beaconMessageBufferSize = default(0);
        int beaconMessageInterval @unit("s") = default(0s); // Interval to try send message(s)
        int beaconMessageHopLimit = default(0); // Limit of hops
        int timeGenerateBeaconMessage @unit("s") = default(0s); // Time for generate messages

        int expNumber = default(1); // Experiment number by table 2^k, number to use _one or _two variables
        int expPart_one_or_two = default(1); // Experiment part, can be 1 or 2, to use 4 values of ttlBM
        int timeLimitGenerateBeaconMessage @unit("s") = default(0s); // ttl beacon message

        int ttlBeaconMessage_one @unit("s") = default(0s); // ttl beacon message
        int ttlBeaconMessage_two @unit("s") = default(0s);
        int countGenerateBeaconMessage_one = default(0); // Count messages generate by time
        int countGenerateBeaconMessage_two = default(0);

        int ttlBeaconStatus @unit("s") = default(0s);
        int beaconStatusBufferSize = default(0);
        int vehTimeUpdatePosition @unit("s") = default(0s); // Time for update vehicle position
        int expSendbyDSCR = default(0);

        int percentP = default(0); // Percentage that send to vehicle from category P
        bool usePathHistory = default(false); // User or not path history when send a message
        bool allowMessageCopy = default(false); // Allow or not copy of messages
        bool selectFromAllVehicles = default(false); // Chose for all vehicle or for only new
        bool useMessagesSendLog = default(false); // Use or not message send log
        bool vehSendWhileParking = default(false); // Send or not while is parking
        bool vehDistTrueEpidemicFalse = default(false); // Is vehDist (true), is Epidemic (false)
        bool vehDistCreateEventGenerateMessage = default(false); // Use the event to generate message or not
        int vehTimeLimitToAcceptGenerateMgs @unit("s") = default(0s); // Time limit that one vehile enter in the scenario
        bool useRateTimeToSend = default(false); // Use or not the rateTimeToSend

    gates:
        input lowerLayerIn; // from mac layer
        output lowerLayerOut; // to mac layer
        input lowerControlIn;
        output lowerControlOut;
}
