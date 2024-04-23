/* Copyright (c) 2022, Vrije Universiteit Amsterdam
 * Contributor: Gaosheng Liu
 * All rights reserved.
 */

#ifndef __ICNode_H
#define __ICNode_H

#include <omnetpp.h>

#include <iostream>
#include <fstream>
#include <sstream>

#include <iomanip>
#include <list>
#include <queue>
#include <utility>
#include <random>
#include <string>
#include <vector>
#include <iterator>
#include <map>
#include <math.h>
#include "dataFrame_m.h"

using namespace omnetpp;
using namespace std;

#define ICNODE_INIT 0x00
#define ICNODE_SENDACK 0x01
#define ICNODE_LOGINTERVAL 0x02

#define TO_SINKNODE 0x03
#define ICNODE_WORKCYCLE 0x05
#define ICNODE_SENDINGPACKETS 0x06
#define ICNODE_TRANSDATA 0x07
#define ICNODE_ENDSIMULATION 0x08
#define ICNODE_REPRODUCEMSG 0x09
#define ICNODE_SYSTEMTIMER 0x0A
#define ICNODE_FIND_ACTIONS 0x0B

struct NodeState
{
    NodeState *dup();
    double estimatedNumTxToBaseStation;
    int numPacketsReceived;
    double totalPacketDelay;
};

class ICNode : public cSimpleModule
{
private:
    struct SIM_API Point
    {
        double x, y;
        Point(double x, double y) : x(x), y(y) {}
    };
    metaFrame *transmissionQueue;
    volatile int transQueLen;
    volatile int findCount;
    volatile int IC_actionCounter;
    volatile int ready2Send;
    volatile int find_waitRound;
    volatile int cycleThreshold;
    volatile int cycleFindIndex;
    volatile int receiverChargeTime;
    volatile int visitStep;
    volatile bool reNormalBiasSwitch;

    metaFrame *iniPack;
    volatile int sinkIniClock;
    volatile int nodeID;
    volatile int nodeType;
    volatile int msgOverallID;
    volatile bool sender;
    volatile int lastSendMsgID;
    cGate *radioInGatePtr;
    metaFrame *ack;

    // Simulator Knowledge
    volatile double x_pos;
    volatile double y_pos;
    char dispString[64];
    list<pair<ICNode *, int>> nodesInRadioRange;
    list<int> neighbourNodes;
    volatile int ICNodeHopValue;
    volatile int findingStates;

    cModule *networkPtr;
    volatile double nodesWide;
    volatile bool stayInListening;
    volatile int tx_range;
    vector<ICNode *> networkHandle;
    volatile int waitingCounterReceiver;
    volatile int waitingCounterSender;
    volatile int relayNum;
    volatile int destinationID;
    volatile int nextHopID;
    volatile int transRound;
    volatile int roundNum;
    volatile int trySendFindCount;
    simtime_t firstPacketTime;
    simtime_t timeSlot;
    simsignal_t arrivalSignalHop2Sink;
    simsignal_t arrivalSignalHop;
    simsignal_t arrivalSignalSlot;
    simsignal_t arrivalSignalSourceID;
    simsignal_t findTime;
    simsignal_t simuTime;
    volatile int slotCount;
    volatile bool rx_state;
    volatile int dutyCycle;
    volatile int biasRelativeReceiver;
    volatile int reNormalFromBias;
    volatile int chargeTimeSlots;
    volatile int countFindSlotFlag;
    volatile int reWaitFlag;
    volatile int curPairdSenderID;
    volatile bool executeTrans;
    volatile bool needBackDutyCycle;
    volatile bool adjustToSendingState;
    volatile bool multiPathSwitch;
    volatile bool backoffMechanism;

    void testDistribution();
    void terminalSim();
    void transmitNext();
    void transmitAckNext();
    void receiveMessage(cPacket *mPtr);
    void topologyDistribution();
    void determineNodesInRadioRadio();

    virtual void initialize();
    void IC_initialize();
    virtual void handleMessage(cMessage *msg);
    void IC_handleMessage(cMessage *msg);
    NodeState *dup();
    double distanceTo(ICNode *nodePtr);
    void activeActions();
    int delayTimesRouting();
    metaFrame *produceMsg(int dataID, int seMark, int msgType);
    metaFrame *produceAck(int aimID, int msgID, int srcID);
    void duplicateMsg(metaFrame *ori, metaFrame *&dMsg);
    void placeNodes(int x_pos, int y_pos);
};

Define_Module(ICNode);
#endif
