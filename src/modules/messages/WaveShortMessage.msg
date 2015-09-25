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

cplusplus {{
#include "Coord.h"
}}

class noncobject Coord;

packet WaveShortMessage {
	//Version of the Wave Short Message
	int wsmVersion = 0;
	//Determine which security mechanism was used
	int securityType = 0;
	//Channel Number on which this packet was sent
	int channelNumber;
	//Data rate with which this packet was sent
	int dataRate = 1;
	//Power Level with which this packet was sent
	int priority = 3;
	//Unique number to identify the service
	int psid = 0;
	//Provider Service Context
	string psc = "Service with some Data";
	//Length of Wave Short Message
	int wsmLength;
	//Data of Wave Short Message
	string wsmData = "Some Data";
	
	// Adicionado (Minicurso_UFPI) - Send ID da pista e velocidade
	
	// Current ID of the road, e. g., 1ato2b
	string roadId = "";
	// Speed of vehicle in the moment which send a message WSM, wsm->setSenderSpeed(traci->getSpeed());
	double senderSpeed = 0.0;

	//
	// Category of vehicle (1 to 10): 1 is bus, 2 is cab, ... need to complete 
	int category = 0;
	// vehicle ID (wsm->setVehicleId(traci->getId());)
	int vehicleId;
	// Path history of the vehicle, calculated from time to time
	//string pathHistory;
	// Radius of Gyration of the vehicle, calculated from time to time.
	// The vehicle send yours Radius of Gyration to another vehicles 
	//double radius_of_Gyration;
	
	int senderAddress = 0;
	int recipientAddress = -1;
	int serial = 0;
	Coord senderPos;
	simtime_t timestamp = 0;
}
