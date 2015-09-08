//
// Copyright (C) 2014 Luz Marina Santos
//

package org.car2x.veins.modules.application.service_discovery;
import org.car2x.veins.modules.application.ieee80211p.BaseWaveApplLayer;

simple service_discovery_RSU extends BaseWaveApplLayer
{
    parameters:
        int serv1  = default(1);  // number of services to send by RSU - minimum 1 - maximum 10
        int serv2  = default(1); 
        volatile double serviceInterval = default(1s) @unit(s); //the intervall between 2 forward of service messages by RSU
        
    @class(service_discovery_RSU);
    @display("i=device/antennatower_vl");

}
