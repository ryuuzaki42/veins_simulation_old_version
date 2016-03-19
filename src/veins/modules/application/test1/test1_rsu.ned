//
//  Copyright (C) 2015 João Batista and Wellington Branquinho
//

package org.car2x.veins.modules.application.test1;
import org.car2x.veins.modules.application.ieee80211p.BaseWaveApplLayer;

simple test1_rsu extends BaseWaveApplLayer {
    parameters:
        int serv1  = default(1);  // number of services to send by RSU - minimum 1 - maximum 10
        int serv2  = default(1); 
        volatile double serviceInterval = default(1s) @unit(s); //the intervall between 2 forward of service messages by RSU

    @class(test1_rsu);
    @display("i=device/antennatower_vl");

}
