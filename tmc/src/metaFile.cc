/* Copyright (c) 2023, Vrije Universiteit Amsterdam
 * Contributor: Gaosheng Liu
 * All rights reserved.
 */

#include "ICNode.h"
#include "dataFrame_m.h"
#include <fstream>
#include <sstream>

using namespace std;

/**
 * @brief Copy node states for sending packets
 *
 * @return NodeState*
 */
NodeState *NodeState::dup()
{
    NodeState *nsPtr = new NodeState();
    return nsPtr;
}

void ICNode::initialize()
{
    this->reNormalBiasSwitch = true;
    this->transmissionQueue = new metaFrame[100000];
    this->transQueLen = 0;
    this->receiverChargeTime = 0;
    this->cycleFindIndex = 0;
    this->findCount = 0;
    this->ready2Send = 1;
    this->waitingCounterSender = 0;
    this->waitingCounterReceiver = 0;
    this->countFindSlotFlag = 0;
    this->visitStep = 0;
    this->find_waitRound = 0;
    this->trySendFindCount = 0;
    this->ICNodeHopValue = 10000;
    this->findingStates = 0;
    this->reWaitFlag = 0;
    this->networkPtr = this->getParentModule();
    this->lastSendMsgID = -1;
    this->msgOverallID = 0;
    this->roundNum = 0;
    this->sinkIniClock = 0;
    this->stayInListening = false;
    IC_initialize();
}

void ICNode::handleMessage(cMessage *msg)
{
    IC_handleMessage(msg);
}

/**
 * @brief Select/pick deviation values according to different strategies
 *
 * @return int
 */
int ICNode::delayTimesRouting()
{
    int delayTimes = 0;
    // chargeTimeSlots ã€�Should measure the correct positions, calculate stepsã€‘

    if ((this->cycleFindIndex % this->cycleThreshold) == 0)
    {
        this->cycleFindIndex = 0;
        this->cycleFindIndex = this->cycleFindIndex + 1;
        if (this->visitStep == 10)
        {
            this->visitStep = 1;
        }
        else
        {
            this->visitStep = this->visitStep + 1;
        }
        return 0;
    }

    delayTimes = this->visitStep;
    this->cycleFindIndex = this->cycleFindIndex + 1;
    return delayTimes;
    /**
     * @brief prime based
     * 
     */
    // return intuniform(0, 6);

    /**
     * @brief geometric based
     * 
     */
    // fstream file;
    // file.open("opt_scale.csv");
    // string line;
    // double par = 0;
    // while(getline(file, line, '\n' ))
    // {
    //     vector<double> lineData;
    //     istringstream templine(line);
    //     string data;
    //     while(getline(templine, data, ','))
    //     {
    //         lineData.push_back(double(atof(data.c_str())));
    //     }

    //     if (lineData[0] >= this->chargeTimeSlots)
    //     {
    //         par = lineData[1];
    //         break;
    //     }
    // }
    // return geometric(par);
}

/**
 * @brief setting nodes' coordinators
 *
 * @param x_pos
 * @param y_pos
 */
void ICNode::placeNodes(int x_pos, int y_pos)
{
    this->x_pos = x_pos;
    this->y_pos = y_pos;
}

/**
 *duplicate frame
 */
void ICNode::duplicateMsg(metaFrame *ori, metaFrame *&dMsg)
{
    ori->setFindCount(dMsg->getFindCount());
    ori->setActualHop(dMsg->getActualHop());
    ori->setHop2Sink(dMsg->getHop2Sink());
    ori->setSenderId(dMsg->getSenderId());
    ori->setSourceId(dMsg->getSourceId());
    ori->setDestinationId(dMsg->getDestinationId());
    ori->setNextHopId(dMsg->getNextHopId());
    ori->setSendTime(dMsg->getSendTime());
    ori->setMsgID(dMsg->getMsgID());
    ori->setMsgType(dMsg->getMsgType());
    ori->setChargeSlot(dMsg->getChargeSlot());
    ori->setEndTrans(dMsg->getEndTrans());
    ori->setName("Packets");
}

/**
 * @brief produce msg packets
 *
 * @param dataID
 * @param conTrans
 * @return metaFrame*
 */
metaFrame *ICNode::produceMsg(int dataID, int seMark, int msgType)
{
    metaFrame *dMsg = new metaFrame();
    dMsg->setFindCount(-1);
    dMsg->setActualHop(0);
    dMsg->setHop2Sink(this->ICNodeHopValue);
    dMsg->setSenderId(this->nodeID);
    dMsg->setSourceId(0);
    dMsg->setDestinationId(this->destinationID);
    dMsg->setNextHopId(this->nextHopID);
    dMsg->setSendTime(0);
    dMsg->setMsgID(dataID);
    dMsg->setMsgType(msgType);
    dMsg->setChargeSlot(this->chargeTimeSlots);
    dMsg->setEndTrans(0);
    dMsg->setName("Packets");
    return dMsg;
}

/**
 * @brief produce ack packets
 * @param dataType
 * @param destination
 * @param messsageID
 * @return metaFrame*
 */
metaFrame *ICNode::produceAck(int aimID, int msgID, int srcID)
{
    metaFrame *dMsg = new metaFrame();
    dMsg->setFindCount(0);
    dMsg->setSenderId(this->nodeID);
    dMsg->setDestinationId(0);
    dMsg->setSourceId(srcID);
    dMsg->setNextHopId(aimID);
    dMsg->setActualHop(0);
    dMsg->setHop2Sink(this->ICNodeHopValue);
    dMsg->setSendTime(0);
    dMsg->setMsgID(msgID);
    dMsg->setMsgType(1);
    dMsg->setEndTrans(0);
    dMsg->setName("Acks");
    return dMsg;
}

/**
 * @brief The distance from nodePtr
 *
 * @param nodePtr
 * @return int
 */
double ICNode::distanceTo(ICNode *nodePtr)
{
    Enter_Method_Silent(); // Omnet requires this
    return sqrt((this->x_pos - nodePtr->x_pos) * (this->x_pos - nodePtr->x_pos) + (this->y_pos - nodePtr->y_pos) * (this->y_pos - nodePtr->y_pos));
}

void ICNode::determineNodesInRadioRadio()
{
    ICNode *nPtr;
    double distance;

    // relay nodes, acuiqre neighbour info
    for (int i = 0; i < this->relayNum; ++i)
    {
        nPtr = check_and_cast<ICNode *>(this->networkPtr->getSubmodule("transceiver", i));
        distance = distanceTo(nPtr);
        if (distance <= this->tx_range && nPtr->nodeID != this->nodeID)
        {
            this->nodesInRadioRange.push_back(std::make_pair(nPtr, distance));
        }
    }

    // sink node
    nPtr = check_and_cast<ICNode *>(this->networkPtr->getSubmodule("sink"));
    distance = distanceTo(nPtr);
    if (distance <= this->tx_range && nPtr->nodeID != this->nodeID)
    {
        this->nodesInRadioRange.push_back(std::make_pair(nPtr, distance));
    }
    this->nodesInRadioRange.sort([](const std::pair<ICNode *, int> &p1, const std::pair<ICNode *, int> &p2)
                                 { return p1.second < p2.second; });
}
