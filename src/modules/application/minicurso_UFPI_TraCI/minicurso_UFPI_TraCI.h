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

#ifndef minicurso_UFPI_TraCI_H
#define minicurso_UFPI_TraCI_H

#include "BaseWaveApplLayer.h"
#include "modules/mobility/traci/TraCIMobility.h"

using Veins::TraCIMobility;
using Veins::AnnotationManager;

 // Adicionado (Minicurso_UFPI)
using namespace std;

/**
 * Small IVC Demo using 11p
 */
class minicurso_UFPI_TraCI : public BaseWaveApplLayer {
    public:
        virtual void initialize(int stage);
        virtual void receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj);
    protected:
        TraCIMobility* traci;
        AnnotationManager* annotations;
        simtime_t lastDroveAt;

        // Adicionado (Minicurso_UFPI)
        //estrutura de dados com as velocidades recebidas dos vizinhos
        vector<pair<simtime_t, double> > speedsList;
        //CongestionTable(mapa onde a chave é o identificador da via e o valor é um vetor com os valores
        //das velocidades recebidas para aquela via e o tempo em que esta velocidade foi recebida
        map<string, vector<pair<simtime_t, double> > > congestionTable;
        //lastSent representa o tempo a última mensagem do tipo CongestionMessage foi recebida ou transmitida
        simtime_t lastSent;
        //updateTablesEvt é utilizada para a cada 5 s atualizar a CongestionTable
        cMessage* updateTablesEvt;
        //insertCurrentSpeedEvt é utilizada para a cada 1 s adicionar a velocidade atual do veículo à speedsList
        cMessage* insertCurrentSpeedEvt;
        // sentMessages identifica todas as congestionMessage já recebidas pelo veículo e é utilizada pelo algoritmo de broadcast multi-hop
        std::map<int,bool> sentMessages;
        //messageId é uma variável de classe (comum a todos os veículos inseridos) que é utilizada para criar os identificadores únicos de cada congestionMessage
        static int messageId;

        //bool sentMessage;

        bool isParking;
        bool sendWhileParking;
        static const simsignalwrap_t parkingStateChangedSignal;

    protected:

        // Adicionado (Minicurso_UFPI)
        virtual void handleSelfMsg(cMessage* msg);
        void updateSpeedList();
        void updateCongestionTable();
        void verifyAndSendCongestionMessage();

        virtual void onBeacon(WaveShortMessage* wsm);
        virtual void onData(WaveShortMessage* wsm);
        void sendMessage(std::string blockedRoadId);
        virtual void handlePositionUpdate(cObject* obj);
        virtual void handleParkingUpdate(cObject* obj);
        virtual void sendWSM(WaveShortMessage* wsm);
};

    // Adicionado (Minicurso_UFPI)
    int minicurso_UFPI_TraCI::messageId = 0;

#endif
