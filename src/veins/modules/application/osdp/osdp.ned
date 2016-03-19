//
// Copyright (C) 2014 Luz Marina Santos
//

package org.car2x.veins.modules.application.osdp;
import org.car2x.veins.modules.application.ieee80211p.BaseWaveApplLayer;

simple osdp extends BaseWaveApplLayer {
    parameters:
        int query  = default(0);   // 0 car doesn`t query - 1 car queries
        volatile double queryInterval = default(1s) @unit(s); //the intervall between 2 query messages by Car
        volatile double timeService = default(30s) @unit(s); //the lifetime of the service
        volatile double initQuery = default(140s) @unit(s); //the lifetime of the service
        string qservice = default ("query service1");  // limit time for to do query
        volatile double timeQuery = default(180s) @unit(s);  
        
    @class(osdp);
    @display("i=block/app2");
    bool sendWhileParking  = default(false); //send messages when the car is parked
}
