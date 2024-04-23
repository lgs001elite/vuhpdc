/* Copyright (c) 2022, Vrije Universiteit Amsterdam
 * Contributor: Gaosheng Liu
 * All rights reserved.
 */

#include "ICNode.h"
#include "dataFrame_m.h"

using namespace std;

void ICNode::topologyDistribution()
{
    int mapWidth = 100;
    int mapHeight = 100;
    this->harvestedNodeNum = mapWidth / 5 * 4;
    int x_sink = mapWidth / 2;
    int y_sink = mapHeight / 2;
    int indexID = 0;

    if ((int)this->networkPtr->par("topologyType") == 1)
    {
        auto &displayString0 = this->networkPtr->getDisplayString();
        displayString0.setTagArg("bgb", mapWidth, mapWidth);
        displayString0.setTagArg("b", mapWidth, mapWidth);
        double x_posR = 0;
        double y_posR = 0;
        for (int j = 0; j < this->relayNum; ++j)
        {
            this->networkHandle.push_back(check_and_cast<ICNode *>(this->networkPtr->getSubmodule("transceiver", j)));
            this->networkHandle[indexID]->nodeType = 1;
            this->networkHandle[indexID]->nodeID = indexID;
            this->networkHandle[indexID]->sender = false;
            auto &displayString = this->networkHandle[indexID]->getDisplayString();
            if (x_posR > mapWidth)
            {
                x_posR = 0;
                y_posR = y_posR + 5;
            }
            if ((x_posR == x_sink) && (y_posR == x_sink))
            {
                x_posR = x_posR + 5;
            }

            if (this->distributedWay == 1)
            {
                // node random distribution
                int gridLen = mapHeight / (mapHeight / this->nodesWide + 1);
                double delta = uniform(0, 2 * PI);
                double alpha = triang(0, 1, 1);
                alpha = pow(alpha, exp(alpha));
                int centerOffset = gridLen / 2;
                int XoffSet = centerOffset + cos(delta) * alpha * 0.5 * gridLen;
                int YoffSet = centerOffset + sin(delta) * alpha * 0.5 * gridLen;
                int cIndex = indexID % (mapHeight / this->nodesWide + 1);
                int rIndex = indexID / (mapHeight / this->nodesWide + 1);
                x_posR = cIndex * gridLen + XoffSet;
                y_posR = rIndex * gridLen + YoffSet;
            }

            ICNode::Point pos(x_posR, y_posR);
            this->networkHandle[indexID]->placeNodes(pos.x, pos.y);
            displayString.setTagArg("p", 0, pos.x);
            displayString.setTagArg("p", 1, pos.y);
            displayString.setTagArg("is", 0, "vs");
            indexID = indexID + 1;
            x_posR = x_posR + 5;
        }
    }
    this->networkHandle.push_back(check_and_cast<ICNode *>(this->networkPtr->getSubmodule("sink")));
    this->networkHandle[indexID]->nodeType = 0;
    this->networkHandle[indexID]->nodeID = indexID;
    this->networkPtr->getSubmodule("sink")->par("nodeID").setIntValue(indexID);
    this->networkHandle[indexID]->sender = false;
    auto &displayString3 = this->networkHandle[indexID]->getDisplayString();
    ICNode::Point pos2(x_sink, y_sink);
    this->networkHandle[indexID]->placeNodes(pos2.x, pos2.y);
    displayString3.setTagArg("p", 0, pos2.x);
    displayString3.setTagArg("p", 1, pos2.y);
    displayString3.setTagArg("is", 0, "vs");
}

/**
 * @brief Actions for harveted nodes and relay nodes
 * all actions need to do in one slot
 */
void ICNode::activeActions()
{
    if ((this->transmissionQueue.empty()) || (this->receiveFlag == true))
    {
        if (this->startFindNeighbour == 1)
        {
            this->iniPack->setHop2Sink(this->shortestPathHop);
            scheduleAt(simTime(), new cMessage("ICNODE_ICINITIAL", ICNODE_INITIALIC));
            return;
        }
        // backing and receiving packets (No actions)

        if ((this->needBackDutyCycle))
        {
            this->needBackDutyCycle = false;
            this->rx_state = false;
            this->reWaitFlag = 1;

            if (variousSwitch == 1)
            {
            }
            scheduleAt(simTime() + (this->recoverySlots) * this->timeSlot, new cMessage("relay node send data with backing", ICNODE_TIMER));
        }
        else
        {
            this->rx_state = true;
            scheduleAt(simTime() + this->timeSlot,
                       new cMessage("timer loop in the working slot; starting receiving", ICNODE_TIMER));
        }
    }
    else
    {
        if (this->needReBackDutyCycle && (this->needBackDutyCycle == false))
        {
            this->reWaitFlag = 1;
            this->rx_state = false;
            this->needReBackDutyCycle = false;
            if (this->variousSwitch)
            {
                scheduleAt(simTime() + this->biasCycleVarious * this->timeSlot, new cMessage("relay node send data with backing", ICNODE_TIMER));
            }
            else
            {
                scheduleAt(simTime() + this->biasCycle * this->timeSlot, new cMessage("relay node send data with backing", ICNODE_TIMER));
            }
            return;
        }
        if ((this->multiPath == 1) && (this->transmissionQueue.size() == 2))
        {
            this->indictValue4Paired = 0;
            this->nextHopID = -1;
            this->sendMatchID = -1;
            this->matrixFindIndex = 0;
        }
        scheduleAt(simTime(), new cMessage("relay node send data", ICNODE_SENDINGPACKETS));
    }
}

/**
 * @brief initialization functions
 *
 */
void ICNode::IC_initialize()
{
    int initFlag = (int)this->networkPtr->par("initFlag");
    this->reverseBiasSwitch = this->networkPtr->par("reverseBiasSwitch");
    this->harvestedNodeNum = (int)this->networkPtr->par("hNumber");
    this->transRound = this->par("tryTransRound");

    this->nodesWide = this->networkPtr->par("nodesWide");
    if (this->nodesWide == 10)
    {
        this->nodesWide = 6;
        this->relayNum = 200;
    }
    else
    {
        this->relayNum = (100 / this->nodesWide + 1) * (100 / this->nodesWide + 1) - 1;
    }
    if (initFlag == 0)
    {
        this->networkPtr->par("initFlag") = 1;
        topologyDistribution();
    }
    this->determineNodesInRadioRadio();
    this->maxSimTime = this->networkPtr->par("maxSimTime");
    this->variousSwitch = this->networkPtr->par("variousSwitch");
    this->backoffMechanism = this->networkPtr->par("backoffMechanism");
    this->radioInGatePtr = this->gate("ICNodeRadioIn");
    this->destinationID = this->networkPtr->getSubmodule("sink")->par("nodeID");
    this->tx_duration = (double)this->par("transmissionDuration");
    this->timeSlot = this->tx_duration;
    this->ack = NULL;
    this->realharvestedNodeNum = 0;
    this->rx_state = false;
    this->slotCount = 0;
    this->dutyCycle = 0;
    this->biasCycle = 0;
    this->feedBackSwitch = this->par("feedBackSwitch");
    if (this->varyingChargingSwitch == 1)
    {
        this->chargeTimeSlots = energyLevel[energyIndex];
        energyIndex = energyIndex + 1;
    }
    else
    {
        this->chargeTimeSlots = this->networkPtr->par("chargingTime");
    }

    this->originalChargingTime = this->chargeTimeSlots;
    if (this->variousSwitch == 1)
    {
        this->chargeTimeSlots = intuniform(this->chargeTimeSlots, this->chargeTimeSlots * 2);
    }
    this->firstPacketTime = 0;
    this->currentPairedID = -1;
    this->indictValue4Paired = 0;
    this->executeTrans = false;
    this->matrixFindIndex = 0;
    this->nextHopID = -1;
    this->recoverySlots = 0;
    this->needBackDutyCycle = false;
    this->needReBackDutyCycle = false;
    this->doDutyCycle = false;
    this->reTransHarvestedNode = false;
    this->trySendAckCount = 0;
    this->findTime = registerSignal("findTime");
    this->arrivalSignalHop = registerSignal("arrivalHop");
    this->arrivalSignalSlot = registerSignal("arrivalSlot");
    this->arrivalSignalSourceID = registerSignal("arrivalSourceID");
    this->arrivalSignalHop2Sink = registerSignal("arrivalHop2Sink");
    this->matrixTrainIndex = 1 + this->chargeTimeSlots;
    this->iniPack = produceMsg(0, -1, 2);

    // initialize routing strategy
    if (this->nodeID != this->destinationID)
    {
        this->matrixCrossList.clear();
        matrixRandom(0);
        geoCal();
    }

    // Create and set display string for this SensorNode module
    sprintf(this->dispString, "p=%f,%f;i=device/accesspoint;is=vs", this->x_pos, this->y_pos);
    this->networkPtr->setDisplayString(this->dispString);
    scheduleAt(simTime(), new cMessage("Sink node initialization", ICNODE_INIT));
}

/**
 * @brief handleMessage functions
 *
 */
void ICNode::IC_handleMessage(cMessage *msg)
{
    // cGate * tempGate = msg->getArrivalGate();
    metaFrame *sendMsg;
    if (msg->isSelfMessage() && msg->getKind() == ICNODE_INIT)
    {
        if (this->nodeType == 0)
        {
            scheduleAt(simTime(), new cMessage("timer loopers for sink nodes", ICNODE_INITIAL));
        }
        else
        {
            simtime_t waitTime = intuniform(0, this->chargeTimeSlots) * this->timeSlot;
            scheduleAt(simTime() + waitTime, new cMessage("timer looper for others", ICNODE_TIMER));
        }
        delete msg;
    }
    else if (msg->isSelfMessage() && msg->getKind() == ICNODE_TRANSDATA)
    {
        if (this->transInternal == 0)
        {
            this->rx_state = false;
            this->transmitNext();
            this->transInternal = this->transInternal + 1;
            scheduleAt(simTime() + 0.1 * this->timeSlot,
                       new cMessage("relay node send data", ICNODE_TRANSDATA));
        }
        else
        {
            this->rx_state = true;
            this->transInternal = 0;
        }

        delete msg;
    }
    else if (msg->isSelfMessage() && msg->getKind() == ICNODE_SENDINGPACKETS)
    {
        this->rx_state = true;
        this->transmitFlag = 1;

        if (this->executeTrans)
        {
            this->executeTrans = false;
            scheduleAt(simTime() + intuniform(0, 5) * this->timeSlot * 0.1, new cMessage("relay node send data", ICNODE_TRANSDATA));
            scheduleAt(simTime() + this->timeSlot, new cMessage("timer loop in the working slot; starting sending", ICNODE_TIMER));
            delete msg;
            return;
        }

        if (this->reTransHarvestedNode)
        {
            this->rx_state = false;
            this->reWaitFlag = 1;
            scheduleAt(simTime() + this->timeSlot * (this->chargeTimeSlots + 1) * 2,
                       new cMessage("timer loop in the working slot; starting sending", ICNODE_TIMER)); // Testing backing mechanism
            this->reTransHarvestedNode = false;
            delete msg;
            return;
        }

        if ((this->indictValue4Paired != 1) || (this->variousMatchKey != 0) || (this->shortestPathHop < this->lastSendHopNum))
        {
            this->executeTrans = true;
            int delayActiveInterval = this->delayTimesRouting();

            int routingStrategy = this->networkPtr->par("overalStrategy");
            if ((routingStrategy == 1) || (routingStrategy == 2))
            {
                delayActiveInterval = geoDelayDistribution(routingStrategy);
            }

            this->biasCycle = (this->biasCycle + delayActiveInterval) % (this->chargeTimeSlots + 1);
            this->recoverySlots = this->chargeTimeSlots + 1 - this->biasCycle; // Recording back slots
            this->recoverySlots = this->recoverySlots % (this->chargeTimeSlots + 1);

            scheduleAt(simTime() + delayActiveInterval * this->timeSlot, new cMessage("relay node send data", ICNODE_SENDINGPACKETS));
        }
        else
        {
            // Maintain the finding matrix after matched nodes
            if (this->variousSwitch == 1)
            {
                if (this->receiverDifference > 0)
                {
                    this->receiverCalibration = this->receiverChargingSlots - this->receiverDifference;
                }
                else
                {
                    this->receiverDifference = this->receiverDifference * -1;
                }

                this->biasCycleVarious = (this->biasCycleVarious + this->receiverDifference) % (receiverChargingSlots + 1);
                this->biasCycleVarious = (receiverChargingSlots + 1 - this->biasCycleVarious) % ((receiverChargingSlots + 1);

                this->biasCycle      = (this->biasCycle + this->receiverDifference) % (this->chargeTimeSlots + 1);
                this->recoverySlots  = this->chargeTimeSlots + 1 - this->biasCycle;
                this->recoverySlots  = this->recoverySlots % (this->chargeTimeSlots + 1);

                if (this->receiverCalibration != 0)
                {
                    this->receiverCalibration = 0;
                    this->receiverDifference = 0;
                    scheduleAt(simTime() + this->receiverCalibration * this->timeSlot,
                               new cMessage("various waiting", ICNODE_SENDINGPACKETS));
                    return;
                }
            }
            this->matrixFindIndex = 0;
            scheduleAt(simTime() + intuniform(0, 5) * this->timeSlot * 0.1, new cMessage("relay node send data", ICNODE_TRANSDATA));
            scheduleAt(simTime() + this->timeSlot, new cMessage("Paired: timer loop in the working slot; starting sending", ICNODE_TIMER));
        }
        delete msg;
    }
    else if (msg->isSelfMessage() && msg->getKind() == ICNODE_SINKNODE)
    {
        this->rx_state = true;
        scheduleAt(simTime(), new cMessage("ICNODE_SYSTEMTIMER", ICNODE_SYSTEMTIMER));
        delete msg;
    }
    else if (msg->isSelfMessage() && msg->getKind() == ICNODE_INITIAL)
    {
        this->rx_state = false;
        this->iniPack->setHop2Sink(0);
        if (simTime() < (1 + this->chargeTimeSlots) * 3 * this->timeSlot)
        {
            this->iniPack->setFindCount(-1);
            for (auto p : this->nodesInRadioRange)
            {
                this->sendDirect(this->iniPack->dup(), p.first, "ICNodeRadioIn");
            }
            scheduleAt(simTime() + this->timeSlot, new cMessage("ICNODE_INITIAL", ICNODE_INITIAL));
        }
        else
        {
            this->startFindNeighbour = 2;
            scheduleAt(simTime(), new cMessage("ICNODE_SINKNODE", ICNODE_SINKNODE));
        }
        delete msg;
    }
    else if (msg->isSelfMessage() && msg->getKind() == ICNODE_INITIALIC)
    {
        if (this->trySendFindCount == 0)
        {
            this->rx_state = true;
            if (this->waitRound == -1)
            {
                this->trySendFindCount++;
                scheduleAt(simTime() + this->timeSlot, new cMessage("timer loop in the charging slots", ICNODE_INITIALIC));
            }
            else if (this->waitRound == 0)
            {
                this->waitRound = this->waitRound - 1;
                if (this->reverseBiasSwitch)
                {
                    scheduleAt(simTime() + this->reverseBias * this->timeSlot, new cMessage("timer loop in the charging slots", ICNODE_INITIALIC));
                }
                else
                {
                    scheduleAt(simTime(), new cMessage("timer loop in the charging slots", ICNODE_INITIALIC));
                }
            }
            else
            {
                this->waitRound = this->waitRound - 1;
                scheduleAt(simTime() + this->timeSlot, new cMessage("timer loop in the charging slots", ICNODE_TIMER));
            }
        }
        else if (this->trySendFindCount == 1)
        {
            this->trySendFindCount++;
            scheduleAt(simTime() + intuniform(0, 8) * this->timeSlot * 0.1,
                       new cMessage("ICNODE_ICINITIAL", ICNODE_INITIALIC));
            scheduleAt(simTime() + this->timeSlot, new cMessage("timer loop in the charging slots", ICNODE_TIMER));
        }
        else if (this->trySendAckCount == 2)
        {
            this->rx_state = false;
            this->trySendFindCount++;
            scheduleAt(simTime() + 0.1 * this->timeSlot,
                       new cMessage("ICNODE_ICINITIAL", ICNODE_INITIALIC));
        }
        else
        {
            this->iniPack->setFindCount(this->sinkIniClock);
            for (auto p : this->nodesInRadioRange)
            {
                this->sendDirect(this->iniPack->dup(), p.first, "ICNodeRadioIn");
            }
            this->trySendFindCount = 0;
            this->rx_state = true;
            this->sinkIniClock = this->sinkIniClock + 1;
            if (this->sinkIniClock == (1 + this->chargeTimeSlots))
            {
                this->startFindNeighbour = 2;
            }
        }
        delete msg;
    }
    else if ((msg->isSelfMessage() && msg->getKind() == ICNODE_SYSTEMTIMER))
    {
        int netSize = this->networkHandle.size();
        int initialCount = 0;
        for (int i = 0; i < netSize; i++)
        {
            if (this->networkHandle[i]->startFindNeighbour == 2)
            {
                initialCount = initialCount + 1;
            }
        }

        if ((netSize == initialCount) && (this->countFindSlotFlag == 0))
        {
            this->countFindSlotFlag = 1;
            emit(this->findTime, simTime(), nullptr);
            scheduleAt(simTime(), new cMessage("ICNODE_SYSTEMTIMER", ICNODE_ENDSIMULATION));
        }

        if (simTime() > this->maxSimTime)
        {
            emit(this->findTime, simTime(), nullptr);
            scheduleAt(simTime(), new cMessage("ICNODE_SYSTEMTIMER", ICNODE_ENDSIMULATION));
        }
        else
        {
            //            emit(this->findTime, simTime(), nullptr);
            scheduleAt(simTime() + 0.001, new cMessage("ICNODE_SYSTEMTIMER", ICNODE_SYSTEMTIMER));
        }
        delete msg;
    }
    else if (msg->isSelfMessage() && msg->getKind() == ICNODE_SENDACK)
    {
        if (this->trySendAckCount == 0)
        {
            this->rx_state = false;
            this->trySendAckCount++;
            this->transmitAckNext();
            scheduleAt(simTime() + 0.1 * this->timeSlot, new cMessage("ICNODE_SENDACK", ICNODE_SENDACK));
        }
        else
        {
            this->trySendAckCount = 0;
            this->rx_state = true;
        }
        delete msg;
    }
    // Received Message from ICNodeRadioIn
    else if ((msg->getArrivalGate() == this->radioInGatePtr) && this->rx_state)
    {
        this->receiveMessage(check_and_cast<metaFrame *>(msg));
    }
    // end simulation
    else if (msg->isSelfMessage() && (msg->getKind() == ICNODE_ENDSIMULATION))
    {
        delete msg;
        int netSize = this->networkHandle.size();
        for (int i = 0; i < netSize; i++)
        {
            this->networkHandle[i]->endSimulation();
        }
    }
    // timer for Slot
    else if (msg->isSelfMessage() && (msg->getKind() == ICNODE_TIMER))
    {
        this->rx_state = false;
        if ((this->reWaitFlag != 1))
        {
            this->slotCount = this->slotCount + 1;
        }
        else
        {
            this->reWaitFlag = 0;
        }
        // calculate the duty cycles. = 0: working state; != 0: charging state
        this->dutyCycle = (this->slotCount) % (this->chargeTimeSlots + 1);

        if (this->dutyCycle != 0)
        {
            scheduleAt(simTime() + this->timeSlot, new cMessage("timer loop in the charging slots", ICNODE_TIMER));
        }
        else
        {
            this->slotCount = 0;
            if (this->firstPacketIndex == 0)
            {
                int qSize = this->transmissionQueue.size() - 1;
                for (int i = 0; i < qSize; i++)
                {
                    metaFrame *preMsg = check_and_cast<metaFrame *>(this->transmissionQueue.front());
                    this->transmissionQueue.pop();
                    this->transmissionQueue.push(preMsg);
                }
                if (!this->transmissionQueue.empty())
                {
                    delete this->transmissionQueue.front();
                    this->transmissionQueue.pop();
                }
                this->firstPacketIndex = -1;
                this->currentPairedID = -1;
            }

            // Do actions
            if (((this->nodeType == 2) || (this->nodeType == 3)) && (this->startFindNeighbour == 2) && (this->firstCollectFlag == 0))
            {
                this->roundNum = this->roundNum + 1;
                this->firstCollectFlag = 1;
                for (int i = 0; i < 2; i++)
                {
                    if (i == 0)
                    {
                        sendMsg = produceMsg(this->msgOverallID, 2, 0);
                    }
                    else
                    {
                        sendMsg = produceMsg(this->msgOverallID, 0, 0);
                    }
                    this->transmissionQueue.push(check_and_cast<metaFrame *>(sendMsg));
                    this->msgOverallID = this->msgOverallID + 1;
                }
                scheduleAt(simTime() + this->timeSlot * ((this->chargeTimeSlots + 1) * 2 + 1), new cMessage("timer loop in the charging slots", ICNODE_TIMER));
            }
            else
            {
                activeActions();
            }
            if (energyIndex >= 10000)
            {
                energyIndex = 0;
            }
            if (this->varyingChargingSwitch == 1)
            {
                this->chargeTimeSlots = energyLevel[energyIndex];
                energyIndex = energyIndex + 1;
            }
        }
        delete msg;
    }
    else
    {
        delete msg;
    }
}

/**
 *
 the next queued data msg to all neighbours in radio range.
 */
void ICNode::transmitNext()
{
    if (!this->transmissionQueue.empty())
    {
        metaFrame *mPtr = (metaFrame *)this->transmissionQueue.front();
        if ((this->nodeID == 45) || (this->nodeID == 46) || (this->nodeID == 47) || (this->nodeID == 48) || (this->nodeID == 49))
        {
            mPtr->setSendTime(simTime());
        }
        // Send a copy of the message to each node in radio range
        if (this->nextHopID != -1)
        {
            mPtr->setNextHopId(this->nextHopID);
        }

        if (this->transmissionQueue.size() == 1)
        {
            mPtr->setEndTrans(1);
        }
        mPtr->setSenderId(this->nodeID);
        this->lastSendMsgID = mPtr->getMsgID();
        mPtr->setHop2Sink(this->shortestPathHop);
        this->lastSendHopNum = this->shortestPathHop;

        for (auto p : this->nodesInRadioRange)
        {
            this->sendDirect(mPtr->dup(), p.first, "ICNodeRadioIn");
        }
    }
}

void ICNode::transmitAckNext()
{
    // Send a copy of the message to each node in radio range
    for (auto p : this->nodesInRadioRange)
    {
        this->sendDirect(this->ack->dup(), p.first, "ICNodeRadioIn");
    }
}

void ICNode::receiveMessage(cPacket *msg)
{
    if (dynamic_cast<metaFrame *>(msg) != NULL)
    {
        metaFrame *dMsg = check_and_cast<metaFrame *>(msg);
        int msgType = dMsg->getMsgType();
        int msgID = dMsg->getMsgID();
        int senderID = dMsg->getSenderId();
        int sourceID = dMsg->getSourceId();
        int hopValue = dMsg->getHop2Sink();

        if (msgType == 2)
        {
            if ((this->shortestPathHop > (hopValue + 1)) && (this->nodeType != 0))
            {
                this->shortestPathHop = hopValue + 1;
                this->sinkIniClock = 0;
                this->startFindNeighbour = 1;
                emit(this->arrivalSignalHop2Sink, this->shortestPathHop, nullptr);

                if (dMsg->getFindCount() == -1)
                {
                    this->waitRound = -1;
                    this->reverseBias = 0;
                }
                else
                {
                    this->waitRound = this->chargeTimeSlots - dMsg->getFindCount();
                    this->reverseBias = this->waitRound;
                }
                if (this->reverseBiasSwitch && (senderID != 45) && (senderID != 46) && (senderID != 47) && (senderID != 48) && (senderID != 49))
                {
                    this->nextHopID = senderID;
                }

                if (this->receiverChargingSlots == 1)
                {
                    this->waitRound = -1;
                }
            }
            delete msg;
            return;
        }

        if (this->startFindNeighbour != 2)
        {
            delete msg;
            return;
        }

        if ((dMsg->getNextHopId() != -1) && (dMsg->getNextHopId() != this->nodeID))
        {
            if ((this->currentPairedID == senderID))
            {
                this->firstPacketIndex = 0;
            }
            delete msg;
            return;
        }

        if (this->shortestPathHop > (hopValue + 1))
        {
            this->shortestPathHop = hopValue + 1;
        }

        // for sink node
        if ((msgType == 0) && (this->nodeType == 0))
        {
            if (this->ack != NULL)
            {
                delete ack;
            }
            this->ack = produceAck(senderID, msgID, sourceID);
            this->ack->setHop2Sink(-1);
            scheduleAt(simTime() + 0.1 * this->timeSlot, new cMessage("sending ack", ICNODE_SENDACK));

            // for next round
            if (dMsg->getMsgMark() == 0)
            {
                int receQueSize = this->transmissionQueue.size();
                bool receivedSignal = true;
                for (int i = 0; i < receQueSize; i++) // clear the repeated msgs
                {
                    metaFrame *preMsg = check_and_cast<metaFrame *>(this->transmissionQueue.front());
                    if ((preMsg->getMsgID() == msgID) && (preMsg->getSenderId() == senderID) && (preMsg->getSourceId() == sourceID))
                    {
                        receivedSignal = false;
                        break;
                    }
                    this->transmissionQueue.pop();
                    this->transmissionQueue.push(preMsg);
                }

                if (receivedSignal)
                {
                    this->transmissionQueue.push(dMsg);
                    ;
                }
                else
                {
                    delete msg;
                    return;
                }

                // Message arrived
                this->firstPacketTime = dMsg->getSendTime();
                int countSlot = (simTime() - this->firstPacketTime) / this->timeSlot;
                int countHop = dMsg->getActualHop();
                int sourceID = dMsg->getSourceId();
                // send a signal
                emit(this->arrivalSignalSlot, countSlot, nullptr);
                emit(this->arrivalSignalHop, countHop, nullptr);
                emit(this->arrivalSignalSourceID, sourceID, nullptr);

                int tRound = dMsg->getRoundMark();
                int dutyRound = this->par("tryTransRound");
                if (tRound == dutyRound)
                {
                    this->realharvestedNodeNum++;
                }

                if ((this->realharvestedNodeNum == this->harvestedNodeNum))
                {
                    scheduleAt(simTime() + this->timeSlot, new cMessage("end Simulation", ICNODE_ENDSIMULATION));
                }
            }
        }
        // for IC node, packets
        else if ((msgType == 0) && ((this->nodeType == 1) || (this->nodeType == 3)))
        {
            if (this->transmitFlag == 1)
            {
                delete msg;
                return;
            }
            if (this->shortestPathHop >= hopValue)
            {
                delete msg;
                return;
            }

            if ((this->currentPairedID != -1) && this->currentPairedID != senderID)
            {
                delete msg;
                return;
            }

            int receQueSize = this->transmissionQueue.size();
            bool receivedSignal = true;
            for (int i = 0; i < receQueSize; i++) // clear the repeated msgs
            {
                metaFrame *preMsg = check_and_cast<metaFrame *>(this->transmissionQueue.front());
                if ((preMsg->getMsgID() == msgID) && (preMsg->getSenderId() == senderID) && (preMsg->getSourceId() == sourceID))
                {
                    receivedSignal = false;
                }
                this->transmissionQueue.pop();
                this->transmissionQueue.push(preMsg);
            }

            if (receivedSignal)
            {
                this->transmissionQueue.push(dMsg);
                if (this->ack != NULL)
                {
                    delete ack;
                }
                this->ack = produceAck(senderID, msgID, sourceID);
                // Response ack before altering info
                scheduleAt(simTime() + 0.1 * this->timeSlot, new cMessage("sending ack", ICNODE_SENDACK));
            }
            // Starting receiving signals
            if ((dMsg->getMsgMark() == 2) && (receivedSignal == true)) // first msg
            {
                this->currentPairedID = senderID;
                this->receiveFlag = true;
            }
            else if ((dMsg->getEndTrans() == 1) && (receivedSignal == true))
            {
                this->rx_state = false;
                this->currentPairedID = -1; // One node pairs one node switch
                this->receiveFlag = false;
                int hopIndex = dMsg->getActualHop() + 1;
                dMsg->setActualHop(hopIndex);
                if (this->doDutyCycle) // After first transmission, rebacking enbale
                {
                    this->doDutyCycle = false;
                    this->needReBackDutyCycle = true;
                    this->reverseBiasBack = 0;
                }
            }
            else
            {
                delete msg;
                return;
            }
        }
        // for IC node, acks
        else if (((msgType == 1) || (msgType == 3)) && (this->nodeType != 0))
        {
            if (this->lastSendHopNum < hopValue)
            {
                delete msg;
                return;
            }
            if (((this->indictValue4Paired != 1) || (this->variousMatchKey != 0)) && (this->backoffMechanism == 1))
            {
                this->indictValue4Paired = 1;
                this->maxTryCount = 0;
                // this->biasQueue4Tr.push_back(make_pair(this->currSynFront, this->biasCycle));
                this->variousMatchKey = 0;
            }
            if (this->sendMatchID != -1)
            {
                if (this->sendMatchID != senderID)
                {
                    delete msg;
                    return;
                }
            }

            this->nextHopID = senderID;
            this->sendMatchID = senderID;
            this->receiverChargingSlots = dMsg->getChargeSlot();
            this->receiverDifference = this->chargeTimeSlots - this->receiverChargingSlots;

            if (!this->transmissionQueue.empty() && (this->lastSendMsgID == msgID))
            {
                int qSize = this->transmissionQueue.size();
                for (int i = 0; i < qSize; i++)
                {
                    metaFrame *preMsg = check_and_cast<metaFrame *>(this->transmissionQueue.front());
                    if ((msgID == this->transmissionQueue.front()->getMsgID()) &&
                        (sourceID == this->transmissionQueue.front()->getSourceId()))
                    {
                        delete this->transmissionQueue.front();
                        this->transmissionQueue.pop(); // transmit next the packet
                    }
                    else
                    {
                        this->transmissionQueue.pop();
                        this->transmissionQueue.push(preMsg);
                    }
                }
            }

            // Switch for backing mechanism
            if ((this->transmissionQueue.empty()) && ((this->nodeType == 1) || (this->nodeType == 3)) && (this->backoffMechanism == 1))
            {
                // relay finish, back duty cycle for waiting next receiving.
                this->needBackDutyCycle = true;
                this->doDutyCycle = true;
                this->transmitFlag = 0;
            }

            if ((this->transmissionQueue.empty()) && (this->nodeType == 2 || (this->nodeType == 3 && this->topologyType == 1))) // for testing backing mechanism
            {
                this->reTransHarvestedNode = true;
                this->transmitFlag = 0;
                metaFrame *sendMsg;
                if (this->transRound == this->roundNum)
                {
                    delete msg;
                    return;
                }
                this->roundNum = this->roundNum + 1;
                for (int i = 0; i < 2; i++)
                {
                    if (i == 0)
                    {
                        sendMsg = produceMsg(this->msgOverallID, 2, 0);
                    }
                    else
                    {
                        sendMsg = produceMsg(this->msgOverallID, 0, 0);
                    }
                    this->transmissionQueue.push(check_and_cast<metaFrame *>(sendMsg));
                    this->msgOverallID = this->msgOverallID + 1;
                }
            }
            delete msg;
        }
        else
        {
            delete msg;
        }
    }
}
