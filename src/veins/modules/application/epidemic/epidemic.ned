// Copyright (C) 2015-2016 João Batista <joao.b@usp.br>

package org.car2x.veins.modules.application.epidemic;
import org.car2x.veins.modules.application.ieee80211p.BaseWaveApplLayer;

simple epidemic extends BaseWaveApplLayer{
    @class(epidemic);
    @display("i=block/app2");
    bool sendWhileParking  = default(false); //send messages when the car is parked
}
