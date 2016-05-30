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
    #include "veins/base/utils/Coord.h"
}}

class noncobject Coord;

packet WaveShortMessage {
    int wsmVersion = 0; // Version of the Wave Short Message
    int securityType = 0; // Determine which security mechanism was used
    int channelNumber; // Channel Number on which this packet was sent
    int dataRate = 1; // Data rate with which this packet was sent
    int priority = 3; // Power Level with which this packet was sent
    int psid = 0; // Unique number to identify the service
    string psc = "Service with some Data"; // Provider Service Context
    int wsmLength; // Length of Wave Short Message
    string wsmData = "Some Data"; // Data of Wave Short Message

    int serial = 0; // Not sure, but I think is like a port number or identification of an application
    Coord senderPos;
    simtime_t timestamp = 0;

    // Epidemic changes
    //bool ackRequest = false; // Determine if a message has been successfully received or not
    unsigned int senderAddress = 0; // int senderAddress = 0;
    unsigned int recipientAddress = 0; // int recipientAddress = -1;

    string source; // Set the source which generate the message, e.g., car[0], car[1] etc.
    string target; // Set the target which the message will be delivered, e.g., rsu[0], rsu[1] etc.
    bool summaryVector = false; // Set if this is a summary vector or not. true value means summaryvector
    bool requestMessages = false; // Set if this is a request vector or not. true value means requestvector
    string globalMessageIdentificaton; // Unique global message identification
    unsigned short hopCount; // Hop Count works like TTL in the IP packets

    // Add Minicurso_UFPI - Send ID da pista e velocidade
    string roadId = ""; // Current ID of the road, e. g., 1ato2b
    double senderSpeed = 0; // Speed of vehicle in the moment

    //string pathHistory; // Path history of the vehicle, calculated from time to time
    //double radius_of_Gyration; // Radius of Gyration of the vehicle, calculated from time to time.
    unsigned short heading;

    unsigned short type; // Type of message
    string category; // Category of vehicle, e.g., T for Taxi, P for Private car
    Coord TargetPos; // Target position for de message
    Coord senderPosPrevious; // Position previous of the veh
    string senderAddressTemporary;
    string recipientAddressTemporary;
    unsigned short rateTimeToSend; // TimeToSend to try another message
}
