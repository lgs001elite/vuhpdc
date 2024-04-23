/* Copyright (c) 2023, Vrije Universiteit Amsterdam
 * Contributor: Gaosheng Liu
 * All rights reserved.
 */

#include "ICNode.h"
#include "dataFrame_m.h"

using namespace std;

void ICNode::topologyDistribution()
{
    // Checking topology by using GUI before doing this first
    double mapWidth = (double)this->networkPtr->par("mapWidth");
    double mapHeight = (double)this->networkPtr->par("mapHeight");
    double x_sink = mapHeight;
    double y_sink = mapWidth / 2;
    int indexID = 0;

    auto &displayString0 = this->networkPtr->getDisplayString();
    ICNode::Point pos(mapWidth, mapWidth);
    displayString0.setTagArg("bgb", pos.x, pos.y);
    displayString0.setTagArg("b", mapWidth, mapWidth);
    double x_posR = 0;
    double y_posR = 0;
    for (int j = 0; j < (this->relayNum); ++j)
    {
        this->networkHandle.push_back(check_and_cast<ICNode *>(this->networkPtr->getSubmodule("transceiver", j)));
        this->networkHandle[indexID]->nodeID = indexID;
        this->networkHandle[indexID]->sender = false;
        auto &displayString = this->networkHandle[indexID]->getDisplayString();

        // node random distribution
        double gridLen = mapHeight / (mapHeight / this->nodesWide + 1);
        double delta = uniform(0, 2 * PI);
        double alpha = triang(0, 1, 1);
        alpha = pow(alpha, exp(alpha));
        double centerOffset = gridLen / 2;
        double XoffSet = centerOffset + cos(delta) * alpha * 0.5 * gridLen;
        double YoffSet = centerOffset + sin(delta) * alpha * 0.5 * gridLen;
        double cIndex = indexID % int(mapHeight / this->nodesWide + 1);
        double rIndex = indexID / (mapWidth / this->nodesWide + 1);
        x_posR = cIndex * (gridLen + 1) + XoffSet;
        y_posR = rIndex * gridLen + YoffSet;

        if (y_posR > mapWidth)
        {
            y_posR = intuniform(0, int(mapWidth));
        }
        if (x_posR > mapHeight)
        {
            x_posR = intuniform(0, int(mapHeight));
        }
        ICNode::Point pos(x_posR, y_posR);
        this->networkHandle[indexID]->placeNodes(pos.x, pos.y);
        displayString.setTagArg("p", 0, pos.x);
        displayString.setTagArg("p", 1, pos.y);
        displayString.setTagArg("is", 0, "vs");
        indexID = indexID + 1;
    }

    this->networkHandle.push_back(check_and_cast<ICNode *>(this->networkPtr->getSubmodule("sink")));
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
    if (this->findingStates == 0) // Initial finding state, IC nodes keep listening
    {
        this->rx_state = true;
        scheduleAt(simTime() + this->timeSlot,
                   new cMessage("IC node enters its working state", ICNODE_WORKCYCLE));
        return;
    }

    if (this->findingStates == 1) // Do finding actions
    {
        this->iniPack->setHop2Sink(this->ICNodeHopValue);
        scheduleAt(simTime() + this->timeSlot * this->delayTimesRouting(),
                   new cMessage("ICNODE_ICINITIAL", ICNODE_FIND_ACTIONS));
        return;
    }

    // IC nodes keep listening states, if their queue len is zero
    if (this->transQueLen == 0)
    {
        // Close transmittion related states when entering listening states
        this->waitingCounterSender = 0;
        // Have found their next hops
        if ((this->needBackDutyCycle) && (this->reNormalFromBias != 0) && (this->backoffMechanism == true))
        {
            this->reWaitFlag = 1;
            this->needBackDutyCycle = false;
            scheduleAt(simTime() + this->reNormalFromBias * this->timeSlot,
                       new cMessage("relay node send data with backing", ICNODE_WORKCYCLE));
            if (this->receiverChargeTime != 0)
            {
                int tempInterval = this->receiverChargeTime + 1 - this->biasRelativeReceiver;
                tempInterval = (tempInterval + this->reNormalFromBias) % (this->receiverChargeTime + 1);
                this->biasRelativeReceiver = this->receiverChargeTime + 1 - tempInterval;
                this->biasRelativeReceiver = this->biasRelativeReceiver % (this->receiverChargeTime + 1);
            }
            this->reNormalFromBias = 0;
            return;
        }

        // When charging gap is not 0, the back-forwrd mechanism should be more adaptive.
        if (this->receiverChargeTime != 0)
        {
            int tempInterval = this->receiverChargeTime + 1 - this->biasRelativeReceiver;
            int gapCharTime = (this->chargeTimeSlots + 1 + tempInterval) % (this->receiverChargeTime + 1);
            this->biasRelativeReceiver = this->receiverChargeTime + 1 - gapCharTime; // Recording back slots
            this->biasRelativeReceiver = this->biasRelativeReceiver % (this->receiverChargeTime + 1);
        }

        // Enter listening state, no needs to do back-forwards operations
        this->rx_state = true;
        scheduleAt(simTime() + this->timeSlot,
                   new cMessage("timer loop in the working slot; starting receiving", ICNODE_WORKCYCLE));
        return;
    }

    // listening
    if (this->ready2Send == 0)
    {
        // Close transmission related states when entering listening states
        this->waitingCounterSender = 0;

        // Have found their next hops
        if ((this->needBackDutyCycle) && (this->reNormalFromBias != 0) && (this->backoffMechanism == true))
        {
            this->needBackDutyCycle = false;
            this->reWaitFlag = 1;
            scheduleAt(simTime() + this->reNormalFromBias * this->timeSlot,
                       new cMessage("relay node send data with backing", ICNODE_WORKCYCLE));
            if (this->receiverChargeTime != 0)
            {
                int tempInterval = this->receiverChargeTime + 1 - this->biasRelativeReceiver;
                tempInterval = (tempInterval + this->reNormalFromBias) % (this->receiverChargeTime + 1);
                this->biasRelativeReceiver = this->receiverChargeTime + 1 - tempInterval; // Recording back slots
                this->biasRelativeReceiver = this->biasRelativeReceiver % (this->receiverChargeTime + 1);
            }
            this->reNormalFromBias = 0;
            return;
        }

        // Charging gap is not 0, the back-forwrd mechanism should be more adaptive.
        if (this->receiverChargeTime != 0)
        {
            int tempInterval = this->receiverChargeTime + 1 - this->biasRelativeReceiver;
            int gapCharTime = (this->chargeTimeSlots + 1 + tempInterval) % (this->receiverChargeTime + 1);
            this->biasRelativeReceiver = this->receiverChargeTime + 1 - gapCharTime; // Recording back slots
            this->biasRelativeReceiver = this->biasRelativeReceiver % (this->receiverChargeTime + 1);
        }

        // Enter listening state, no needs to do back-forwards operations
        if ((this->waitingCounterReceiver > this->IC_actionCounter) && (this->stayInListening == false))
        {
            // Enter to listening states when they reach the listening threshold
            this->ready2Send = 1;
            this->adjustToSendingState = true;
            if (this->nextHopID != -1)
            {
                int tempInterval = this->cycleFindIndex % this->cycleThreshold;
                this->cycleFindIndex = this->cycleFindIndex - tempInterval;
            }
            this->waitingCounterSender = 0;
            this->waitingCounterReceiver = 0;
            scheduleAt(simTime() + this->timeSlot,
                       new cMessage("timer loop in the working slot; starting receiving", ICNODE_WORKCYCLE));
        }
        else
        {
            this->rx_state = true;
            this->waitingCounterReceiver = this->waitingCounterReceiver + 1;
            scheduleAt(simTime() + this->timeSlot,
                       new cMessage("timer loop in the working slot; starting receiving", ICNODE_WORKCYCLE));
        }
    }
    else // sending
    {
        // Close listening related states when entering transmission states
        this->waitingCounterReceiver = 0;

        // Judge if turn off adjust aga depending on charging gap between receivers and senders
        if ((this->biasRelativeReceiver != 0) && (this->reNormalBiasSwitch == true) && (this->backoffMechanism == true))
        {
            // Updating reNormalFromBias
            this->reNormalFromBias = this->chargeTimeSlots + 1 - this->reNormalFromBias;
            this->reNormalFromBias = (this->biasRelativeReceiver + this->reNormalFromBias) % (this->chargeTimeSlots + 1);
            this->reNormalFromBias = this->chargeTimeSlots + 1 - this->reNormalFromBias;
            this->reNormalFromBias = this->reNormalFromBias % (this->chargeTimeSlots + 1);
            this->reNormalBiasSwitch = false;

            this->reWaitFlag = 1;
            scheduleAt(simTime() + this->biasRelativeReceiver * this->timeSlot,
                       new cMessage("relay node send data with backing (DiffChargeTimes)", ICNODE_WORKCYCLE));
            this->biasRelativeReceiver = 0;
            return;
        }
        this->reNormalBiasSwitch = true; // Do bias computation when every sending

        this->waitingCounterSender = this->waitingCounterSender + 1;
        scheduleAt(simTime(), new cMessage("relay node send data", ICNODE_SENDINGPACKETS));
    }
}
/**
 * @brief initialization functions
 *
 */
void ICNode::IC_initialize()
{
    this->relayNum = (int)this->networkPtr->par("nodeNum");
    this->transRound = this->par("tryTransRound");
    this->nodesWide = (double)this->networkPtr->par("nodesWide");
    this->nodeType = (int)this->par("nodeType");
    this->tx_range = (int)this->par("transmissionRange");

    this->multiPathSwitch = (bool)this->networkPtr->par("multiPathSwitch");
    this->backoffMechanism = (bool)this->networkPtr->par("backoffMechanism");
    int initFlag = (int)this->networkPtr->par("initFlag");
    if (initFlag == 0)
    {
        this->networkPtr->par("initFlag") = 1;
        topologyDistribution();
    }
    // retrieve parameters from Find
    this->determineNodesInRadioRadio();
    this->radioInGatePtr = this->gate("ICNodeRadioIn");
    this->destinationID = this->networkPtr->getSubmodule("sink")->par("nodeID");
    this->timeSlot = (double)this->par("transmissionDuration");
    this->ack = NULL;
    this->rx_state = false;
    this->slotCount = 0;
    this->dutyCycle = 0;
    this->biasRelativeReceiver = 0;
    this->chargeTimeSlots = this->networkPtr->par("chargingTime");
    this->chargeTimeSlots = intuniform(this->chargeTimeSlots, this->chargeTimeSlots * 3);
    this->IC_actionCounter = (1 + this->chargeTimeSlots) * 3;
    this->cycleThreshold = 3 * (this->chargeTimeSlots + 1);
    this->firstPacketTime = 0;
    this->curPairdSenderID = -1;
    this->executeTrans = false;
    this->cycleFindIndex = 0;
    this->nextHopID = -1;
    this->reNormalFromBias = 0;
    this->needBackDutyCycle = false;
    this->adjustToSendingState = false;
    this->findTime = registerSignal("findTime");
    this->simuTime = registerSignal("simuTime");
    this->arrivalSignalHop = registerSignal("arrivalHop");
    this->arrivalSignalSlot = registerSignal("arrivalSlot");
    this->arrivalSignalSourceID = registerSignal("arrivalSourceID");
    this->arrivalSignalHop2Sink = registerSignal("arrivalHop2Sink");
    this->iniPack = produceMsg(0, -1, 2);
    this->iniPack->setSourceId(this->nodeID);

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
    // Initailize node charging time
    if (msg->isSelfMessage() && msg->getKind() == ICNODE_INIT)
    {
        if (this->nodeType == 0)
        {
            scheduleAt(simTime(), new cMessage("timer loopers for the sink node", TO_SINKNODE));
        }
        else
        {
            simtime_t waitTime = intuniform(0, this->chargeTimeSlots) * this->timeSlot;
            scheduleAt(simTime() + waitTime, new cMessage("timer looper IC nodes", ICNODE_WORKCYCLE));
        }
        delete msg;
    }
    else if (msg->isSelfMessage() && msg->getKind() == ICNODE_TRANSDATA)
    {
        this->transmitNext();
        delete msg;
    }
    else if (msg->isSelfMessage() && msg->getKind() == ICNODE_SENDINGPACKETS)
    {
        if (this->executeTrans)
        {
            this->rx_state = true;
            this->executeTrans = false;
            scheduleAt(simTime() + intuniform(1, 3) * this->timeSlot * 0.1,
                       new cMessage("relay node send data", ICNODE_TRANSDATA));
            scheduleAt(simTime() + this->timeSlot,
                       new cMessage("timer loop in the working slot; starting sending", ICNODE_WORKCYCLE));
            delete msg;
            return;
        }

        if ((this->nextHopID == -1))
        {
            this->executeTrans = true;
            int delayActiveInterval = this->delayTimesRouting();
            // For diff
            this->reNormalFromBias = (this->chargeTimeSlots + 1) - this->reNormalFromBias;
            this->reNormalFromBias = this->reNormalFromBias + delayActiveInterval;
            this->reNormalFromBias = this->reNormalFromBias % (this->chargeTimeSlots + 1);
            this->reNormalFromBias = (this->chargeTimeSlots + 1) - this->reNormalFromBias;
            this->reNormalFromBias = this->reNormalFromBias % (this->chargeTimeSlots + 1);
            scheduleAt(simTime() + delayActiveInterval * this->timeSlot,
                       new cMessage("relay node send data", ICNODE_SENDINGPACKETS));
        }
        else
        {
            // Maintain the finding matrix after matched nodes
            this->rx_state = true;
            scheduleAt(simTime() + intuniform(1, 3) * this->timeSlot * 0.1,
                       new cMessage("relay node send data", ICNODE_TRANSDATA));
            scheduleAt(simTime() + this->timeSlot,
                       new cMessage("Paired: timer loop in the working slot; starting sending", ICNODE_WORKCYCLE));
        }
        delete msg;
    }
    else if (msg->isSelfMessage() && msg->getKind() == TO_SINKNODE)
    {
        this->rx_state = false;
        this->iniPack->setHop2Sink(0);
        if (simTime() < (1 + this->chargeTimeSlots) * 6 * this->timeSlot)
        {
            this->iniPack->setFindCount(-1);
            for (auto p : this->nodesInRadioRange)
            {
                this->sendDirect(this->iniPack->dup(), p.first, "ICNodeRadioIn");
            }
            scheduleAt(simTime() + this->timeSlot, new cMessage("TO_SINKNODE", TO_SINKNODE));
        }
        else
        {
            this->findingStates = 2; // Sink node enters the cycle listeing state
            this->rx_state = true;
            scheduleAt(simTime(), new cMessage("ICNODE_SYSTEMTIMER", ICNODE_SYSTEMTIMER));
        }
        delete msg;
    }
    else if (msg->isSelfMessage() && msg->getKind() == ICNODE_FIND_ACTIONS)
    {
        this->rx_state = true;
        if (this->find_waitRound > 0)
        {
            this->find_waitRound = this->find_waitRound - 1;
            scheduleAt(simTime() + this->timeSlot, new cMessage("Find process backs to slot cycles", ICNODE_WORKCYCLE));
            delete msg;
            return;
        }
        if (this->trySendFindCount == 0)
        {
            this->trySendFindCount++;
            // Pick a randoms sending delay within a time slot
            scheduleAt(simTime() + intuniform(0, 9) * this->timeSlot * 0.1,
                       new cMessage("ICNODE_ICINITIAL", ICNODE_FIND_ACTIONS));
            scheduleAt(simTime() + this->timeSlot,
                       new cMessage("Find process backs to slot cycles", ICNODE_WORKCYCLE));
        }
        else
        {
            this->rx_state = false;
            this->trySendFindCount = 0;
            // Do the actual finding actions
            int datedWaitRound = (1 + this->chargeTimeSlots) * 6 - this->sinkIniClock;
            if (datedWaitRound > 0)
            {
                this->iniPack->setFindCount(datedWaitRound);
            }
            else
            {
                this->iniPack->setFindCount(0);
            }

            for (auto p : this->nodesInRadioRange)
            {
                this->sendDirect(this->iniPack->dup(), p.first, "ICNodeRadioIn");
            }
            this->sinkIniClock = this->sinkIniClock + 1;
            // the longest charging gap between senders and receivers is 3 times
            if (this->sinkIniClock > (1 + this->chargeTimeSlots) * 6)
            {
                this->findingStates = 2; // IC node enters normal working states
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
            if (this->networkHandle[i]->findingStates == 2)
            {
                initialCount = initialCount + 1;
            }
        }

        if ((netSize == initialCount) && (this->countFindSlotFlag == 0))
        {
            this->countFindSlotFlag = 1;
            emit(this->findTime, simTime(), nullptr);
            // scheduleAt(simTime(), new cMessage("ICNODE_SYSTEMTIMER", ICNODE_ENDSIMULATION));
        }

        if (simTime() > (SIMTIME_MAX - this->timeSlot * 10))
        {
            emit(this->simuTime, simTime(), nullptr);
            scheduleAt(simTime(), new cMessage("ICNODE_SYSTEMTIMER", ICNODE_ENDSIMULATION));
        }
        else
        {
            int icNodeCount = 0;
            int netSize = this->networkHandle.size();
            for (int i = 0; i < netSize; i++)
            {
                if ((this->networkHandle[i]->transQueLen == 0) && (this->networkHandle[i]->findingStates == 2))
                {
                    icNodeCount = icNodeCount + 1;
                }
            }
            if (icNodeCount == (netSize - 1))
            {
                scheduleAt(simTime() + 1 * this->timeSlot, new cMessage("ICNODE_SYSTEMTIMER", ICNODE_ENDSIMULATION));
            }
            scheduleAt(simTime() + this->timeSlot, new cMessage("ICNODE_SYSTEMTIMER", ICNODE_SYSTEMTIMER));
        }
        delete msg;
    }
    else if (msg->isSelfMessage() && msg->getKind() == ICNODE_SENDACK)
    {
        this->transmitAckNext();
        delete msg;
    }
    // Received Message from ICNodeRadioIn
    else if ((msg->getArrivalGate() == this->radioInGatePtr) && this->rx_state)
    {
        /**
         *This msg cannot be deleted because it is the interfaces with other nodes (send/receive messages)
         */
        this->receiveMessage(check_and_cast<metaFrame *>(msg));
    }
    // end simulation
    else if (msg->isSelfMessage() && (msg->getKind() == ICNODE_ENDSIMULATION))
    {
        emit(this->simuTime, simTime(), nullptr);
        int netSize = this->networkHandle.size();
        for (int i = 0; i < netSize; i++)
        {
            this->networkHandle[i]->endSimulation();
        }
        delete msg;
    }
    // timer for Slot
    else if (msg->isSelfMessage() && (msg->getKind() == ICNODE_WORKCYCLE))
    {
        // At the beginging of working slot, turn off the receiving switch. Turn on/off depanding on IC nodes actions
        this->rx_state = false;
        // This is a switch for waiting a slot
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
            scheduleAt(simTime() + this->timeSlot, new cMessage("IC nodes count their duty cycles", ICNODE_WORKCYCLE));
        }
        else
        {
            this->slotCount = 0;
            // Produce msgs after the finding state
            if ((this->nodeType == 1) && (this->findingStates == 2) && (this->transRound != 0) &&
                (this->transQueLen == 0) && (this->curPairdSenderID == -1))
            {
                this->roundNum = this->roundNum + 1;
                this->transRound = this->transRound - 1;
                for (int i = 0; i < 2; i++)
                {
                    metaFrame *sendMsg;
                    if (i == 0)
                    {
                        sendMsg = produceMsg(this->msgOverallID, 0, 0);
                    }
                    else
                    {
                        sendMsg = produceMsg(this->msgOverallID, 2, 0);
                    }
                    sendMsg->setSourceId(this->nodeID);
                    this->transQueLen = this->transQueLen + 1;
                    duplicateMsg(&this->transmissionQueue[this->transQueLen - 1], sendMsg);
                    this->msgOverallID = this->msgOverallID + 1;
                    delete sendMsg;
                }
                this->ready2Send = 1; // Enter sending states after producing all needed packets
                if ((this->nextHopID != -1) && (this->receiverChargeTime != 0))
                {
                    int tempInterval = this->receiverChargeTime + 1 - this->biasRelativeReceiver;
                    int gapCharTime = ((this->chargeTimeSlots + 1) * 3 + tempInterval) % (this->receiverChargeTime + 1);
                    this->biasRelativeReceiver = this->receiverChargeTime + 1 - gapCharTime; // Recording back slots
                    this->biasRelativeReceiver = this->biasRelativeReceiver % (this->receiverChargeTime + 1);
                }
                // When charging gap is not 0, the back - forwrd mechanism should be more adaptive.
                int TimeFromProducePackets = ((this->chargeTimeSlots + 1) * 2 + 1); // Produce two packets every time
                scheduleAt(simTime() + this->timeSlot * TimeFromProducePackets,
                           new cMessage("time consumption for producing packets", ICNODE_WORKCYCLE));
            }
            else
            {
                activeActions();
            }
        }
        delete msg;
    }
    else
    {
        delete msg;
    }
    /**
     * Attention
     * Cannot ber delete msg here, It will cause nodes cannot receive any messages.
     */
}

/**
 the next queued data msg to all neighbours in radio range.
 */
void ICNode::transmitNext()
{
    if (this->transQueLen > 0)
    {
        metaFrame *mPtr = &this->transmissionQueue[this->transQueLen - 1];
        if (mPtr->getSendTime() == 0)
        {
            mPtr->setSendTime(simTime());
        }
        // Send a copy of the message to each node in radio range
        if ((this->nextHopID != -1))
        {
            mPtr->setNextHopId(this->nextHopID);
            mPtr->setMsgType(0);
        }
        else
        {
            mPtr->setMsgType(4); // Senders try to establish the transmission connection and find its next hop
        }

        if (this->multiPathSwitch == true)
        {
            mPtr->setMsgType(0); // Multiple path do not need to establish connection
        }
        if (this->transQueLen == 1)
        {
            mPtr->setEndTrans(1);
        }
        mPtr->setSenderId(this->nodeID);
        this->lastSendMsgID = mPtr->getMsgID();
        mPtr->setHop2Sink(this->ICNodeHopValue);
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
        /*
         * Variables for all received packets
         */
        metaFrame *dMsg = check_and_cast<metaFrame *>(msg);
        int receivedMsgType = dMsg->getMsgType();
        int receivedMsgID = dMsg->getMsgID();
        int senderID = dMsg->getSenderId();
        int sourceID = dMsg->getSourceId();
        int receivedHopValue = dMsg->getHop2Sink();
        int aimID = dMsg->getNextHopId();

        /**
         * @brief Processing in the Finding phrase
         *
         */
        // MsgType = 2, it is the Msg type to construct routing topology
        if (receivedMsgType == 2)
        {
            if ((this->ICNodeHopValue > (receivedHopValue + 1)) && (this->nodeType != 0))
            {
                if (this->findingStates != 2)
                {
                    this->sinkIniClock = 0;
                    emit(this->arrivalSignalHop2Sink, this->ICNodeHopValue, nullptr);
                    this->ICNodeHopValue = receivedHopValue + 1;
                    this->findingStates = 1;
                    if (receivedHopValue != 0)
                    {
                        this->find_waitRound = dMsg->getFindCount();
                    }
                }
            }
            delete msg;
            return;
        }
        // If IC nodes have not completed Finding operations, do not do any other packets receiving opeartions
        if (this->findingStates != 2)
        {
            delete msg;
            return;
        }

        /**
         * @brief Finishing finding actions, all the nodes enter normal listening states.
         * @param receivedMsgType
         */
        // For the sink node, If receivers and senders need to establish connections, they follow the following process.
        // MsyType = 0 or MsyType= 4, It is for the establishment process.
        // nodeType = 0, it indicates this node is the sink node.
        if (((receivedMsgType == 0) || (receivedMsgType == 4)) && (this->nodeType == 0))
        {
            // Response acks firstly
            if (this->ack != NULL)
            {
                delete ack;
            }
            this->ack = produceAck(senderID, receivedMsgID, sourceID);
            this->ack->setHop2Sink(0);   // This ack parameter is for establishment process.
            this->ack->setChargeSlot(0); // This ack parameter is for establishment process.
            scheduleAt(simTime() + this->timeSlot * 0.1, new cMessage("sending ack", ICNODE_SENDACK));

            // Judge if receiving packets
            bool receivedSignal = true;
            for (int i = 0; i < this->transQueLen; i++) // clear the repeated msgs
            {
                // check_and_cast<metaFrame*>(this->transmissionQueue.front());
                metaFrame *preMsg = &this->transmissionQueue[i];
                if ((preMsg->getMsgID() == receivedMsgID) && (preMsg->getSourceId() == sourceID))
                {
                    receivedSignal = false;
                    delete msg;
                    return;
                }
            }
            if (receivedSignal)
            {
                // Message arrived
                this->firstPacketTime = dMsg->getSendTime();
                int countSlot = (simTime() - this->firstPacketTime) / this->timeSlot;
                int countHop = dMsg->getActualHop();
                emit(this->arrivalSignalSlot, countSlot, nullptr);
                emit(this->arrivalSignalHop, countHop, nullptr);
                emit(this->arrivalSignalSourceID, sourceID, nullptr);
                this->transQueLen = this->transQueLen + 1;
                duplicateMsg(&this->transmissionQueue[this->transQueLen - 1], dMsg);
                int nodeNum = (int)this->networkPtr->par("nodeNum");
                int sumPacketsNum = nodeNum * 100 * 2;
                if (sumPacketsNum == this->transQueLen)
                {
                    emit(this->simuTime, simTime(), nullptr);
                    scheduleAt(simTime(), new cMessage("ICNODE_SYSTEMTIMER", ICNODE_ENDSIMULATION));
                }
                delete dMsg;
            }
            return;
        }
        // But, for  the sink node is a powerful node, we don't need to establish a handshake process.
        if (receivedHopValue == 0)
        {
            this->nextHopID = senderID;
        }

        /**
         * @brief Construct a new if object
         * This Construct process is for common IC nodes
         * IC nodes,  filting packet by judging hop values
         */
        if (ready2Send == 1)
        {
            // Expecting to receive acks (Hops are smaller) from other nodes
            if (this->ICNodeHopValue <= receivedHopValue)
            {
                delete msg;
                return;
            }

            if (receivedMsgType != 1)
            {
                delete msg;
                return;
            }

            if (this->nodeID != aimID)
            {
                delete msg;
                return;
            }

            if (multiPathSwitch == true)
            {
                if ((this->transQueLen > 0) && (this->lastSendMsgID == receivedMsgID))
                {
                    metaFrame *preMsg = &this->transmissionQueue[this->transQueLen - 1];
                    if ((receivedMsgID == preMsg->getMsgID()) &&
                        (sourceID == preMsg->getSourceId()))
                    {
                        this->transQueLen = this->transQueLen - 1;
                        this->nextHopID = -1;
                        this->cycleFindIndex = 0;
                        this->lastSendMsgID = -1;
                    }
                }
                if (this->transQueLen == 0) // for  backing mechanism
                {
                    this->ready2Send = 0;
                    this->waitingCounterReceiver = 0;
                    this->needBackDutyCycle = true;
                    this->rx_state = false;
                }
                return;
            }

            if (this->nextHopID == -1)
            {
                this->nextHopID = senderID;
                this->receiverChargeTime = dMsg->getChargeSlot();
                int gapChargeTime = (this->chargeTimeSlots + 1) % (this->receiverChargeTime + 1);
                this->biasRelativeReceiver = (this->receiverChargeTime + 1) - gapChargeTime;
                this->biasRelativeReceiver = this->biasRelativeReceiver % (this->receiverChargeTime + 1);
                /**
                 * @brief Do not do any packets operations before connection eastablishment
                 */
                if (this->ack != NULL)
                {
                    delete this->ack;
                }
                this->ack = produceAck(senderID, receivedMsgID, sourceID);
                this->ack->setMsgType(3); // Sender sends its second-hand shake after receiving receivers'acks
                this->ack->setChargeSlot(this->chargeTimeSlots);
                scheduleAt(simTime() + this->timeSlot * 0.1, new cMessage("sending ack", ICNODE_SENDACK));
                delete msg;
                return;
            }
            else
            {
                int gapChargeTime = (this->chargeTimeSlots + 1) % (this->receiverChargeTime + 1);
                this->biasRelativeReceiver = (this->receiverChargeTime + 1) - gapChargeTime;
                this->biasRelativeReceiver = this->biasRelativeReceiver % (this->receiverChargeTime + 1);
            }
        }
        else // Receivers judge those establishment packets
        {
            if (this->ICNodeHopValue >= receivedHopValue)
            {
                delete msg;
                return;
            }
            if ((receivedMsgType == 4) && (this->curPairdSenderID == -1) && (this->stayInListening == false))
            {
                this->curPairdSenderID = senderID;
            }
            /**
             * @brief Do not do any packets operatiosn before connection eastablishment
             */
            if ((this->stayInListening == false) && (receivedMsgType == 4) && (this->curPairdSenderID == senderID))
            {
                if (this->ack != NULL)
                {
                    delete this->ack;
                }
                this->ack = produceAck(senderID, receivedMsgID, sourceID);
                this->ack->setChargeSlot(this->chargeTimeSlots);
                scheduleAt(simTime() + this->timeSlot * 0.1, new cMessage("sending ack", ICNODE_SENDACK));
                delete msg;
                return;
            }

            if ((receivedMsgType == 3) && (this->curPairdSenderID == senderID) && (aimID != this->nodeID))
            {
                this->curPairdSenderID = -1;
                delete msg;
                return;
            }
            if ((receivedMsgType == 3) && (this->curPairdSenderID == senderID) && (aimID == this->nodeID))
            {
                this->stayInListening = true;
                delete msg;
                return;
            }
            if (receivedMsgType != 0)
            {
                delete msg;
                return;
            }
        }

        /**
         * @brief Actions when the msgs'type are acks
         *
         */
        if ((receivedMsgType == 1) && (ready2Send == 1))
        {
            if (senderID != this->nextHopID)
            {
                delete msg;
                return;
            }

            if ((this->transQueLen > 0) && (this->lastSendMsgID == receivedMsgID))
            {
                metaFrame *preMsg = &this->transmissionQueue[this->transQueLen - 1];
                if ((receivedMsgID == preMsg->getMsgID()) &&
                    (sourceID == preMsg->getSourceId()))
                {
                    this->transQueLen = this->transQueLen - 1;
                    this->lastSendMsgID = -1;
                }
            }
            if (this->transQueLen == 0) // for  backing mechanism
            {
                this->ready2Send = 0;
                this->waitingCounterReceiver = 0;
                this->needBackDutyCycle = true;
                this->rx_state = false;
            }
        }

        /**
         * @brief Actions when the msgs'type are packets
         *
         */
        if ((receivedMsgType == 0) && (ready2Send == 0))
        {
            if (this->multiPathSwitch == false)
            {
                if (aimID != -1)
                {
                    if (this->nodeID != aimID)
                    {
                        delete msg;
                        return;
                    }
                }

                if ((this->curPairdSenderID == -1) && (this->stayInListening == false))
                {
                    this->curPairdSenderID = senderID;
                    this->stayInListening = true;
                }
                else
                {
                    if (this->curPairdSenderID != senderID)
                    {
                        delete msg;
                        return;
                    }
                }
            }

            // Response acks whatever
            if (this->ack != NULL)
            {
                delete this->ack;
            }
            this->ack = produceAck(senderID, receivedMsgID, sourceID);
            this->ack->setChargeSlot(this->chargeTimeSlots);
            scheduleAt(simTime() + this->timeSlot * 0.1, new cMessage("sending ack", ICNODE_SENDACK));
            // Response ack before altering info
            this->rx_state = false;

            bool receivedSignal = true;
            for (int i = 0; i < this->transQueLen; i++) // clear the repeated msgs
            {
                metaFrame *preMsg = &this->transmissionQueue[i]; // check_and_cast<metaFrame*>(this->transmissionQueue.front());
                if ((preMsg->getMsgID() == receivedMsgID) && (preMsg->getSourceId() == sourceID))
                {
                    receivedSignal = false;
                    break;
                }
            }

            if (receivedSignal == true)
            {
                int hopIndex = dMsg->getActualHop() + 1;
                dMsg->setActualHop(hopIndex);
                this->transQueLen = this->transQueLen + 1;
                if (this->multiPathSwitch == true)
                {
                    this->curPairdSenderID = -1;
                    this->stayInListening = false;
                }
                if (dMsg->getEndTrans() == 1)
                {
                    this->curPairdSenderID = -1;
                    this->stayInListening = false;
                    this->ready2Send = 1;
                    dMsg->setEndTrans(0);
                    this->waitingCounterSender = 0;
                    this->adjustToSendingState = true;
                }
                duplicateMsg(&this->transmissionQueue[this->transQueLen - 1], dMsg);
            }
        }
        delete msg;
    }
}
