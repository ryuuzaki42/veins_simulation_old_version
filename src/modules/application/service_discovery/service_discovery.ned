//
// Copyright (C) 2014 Luz Marina Santos
//

package org.car2x.veins.modules.application.service_discovery;
import org.car2x.veins.modules.application.ieee80211p.BaseWaveApplLayer;

simple service_discovery extends BaseWaveApplLayer
{
    parameters:
        volatile int query  = default(0);   // 0 car doesn`t query - 1 car queries
        volatile double queryInterval = default(1s) @unit(s); //the intervall between 2 query messages by Car
        volatile double timeService = default(30s) @unit(s); //the lifetime of the service
        volatile double initQuery = default(140s) @unit(s); //time for doing query
        volatile string qservice = default ("query servicex");  // service default to do query
        volatile double timeQuery = default(180s) @unit(s);  
        
    @class(service_discovery);
	@display("i=block/app2");
    bool sendWhileParking  = default(false); //send messages when the car is parked
}
