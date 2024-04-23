/* Copyright (c) 2022, Vrije Universiteit Amsterdam
 * Contributor: Gaosheng Liu
 * All rights reserved.
 */

#include "ICNode.h"
#include "dataFrame_m.h"

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
    this->energyLevel = 0;
    this->receiverDifference = 0;
    this->receiverCalibration = 0;
    this->receiverChargingSlots = 0;
    this->countFindSlotFlag = 0;
    this->transmitFlag = 0;
    this->reverseBiasRe = 0;
    this->reverseBias = 0;
    this->reverseBiasBack = 0;
    this->firstCollectFlag = 0;
    this->waitRound = -1;
    this->transInternal = 0;
    this->trySendFindCount = 0;
    this->lastSendHopNum = -1;
    this->shortestPathHop = 10000;
    this->tx_range = 10;
    this->startFindNeighbour = 0;
    this->maxSimTime = 0;
    this->maxTrans = 0;
    this->maxWait = 0;
    this->maxTryCount = 0;
    this->reWaitFlag4Re = 0;
    this->biasCycle4Re = 0;
    this->needBackDutyCycle4Re = false;
    this->variousMatchKey = 0;
    this->periodicWaitCount = 0;
    this->originalChargingTime = 0;
    this->lastDisSlots4Rx = 0;
    this->disSlots4Rx = 0;
    this->reWaitFlag = -1;
    this->networkPtr = this->getParentModule();
    this->backoffMechanism = -1;
    this->topologyType = this->networkPtr->par("topologyType");
    this->distributedWay = this->networkPtr->par("distributedWay");
    this->multiPath = this->networkPtr->par("multiPath");
    this->varyingChargingSwitch = this->networkPtr->par("varyingChargingSwitch");
    this->firstPacketIndex = -1;
    this->lastSendMsgID = -1;
    this->receiveFlag = false;
    this->msgOverallID = 0;
    this->roundNum = 0;
    this->sendMatchID = -1;
    this->sinkIniClock = 0;
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

    if ((this->matrixFindIndex == this->matrixTrainIndex))
    {
        matrixFindIndex = 0;
        if (this->maxTryCount > (2 * this->chargeTimeSlots))
        {
            this->maxTryCount = 0;
            matrixRandom(1);
        }
        this->maxTryCount++;
    }

    if ((this->matrixFindIndex < this->matrixTrainIndex))
    {
        delayTimes = matrixCrossList[this->matrixFindIndex];
        this->matrixFindIndex = this->matrixFindIndex + 1;
    }
    return delayTimes;
}

/**
 * @brief Select/pick deviation values according to different strategies
 *
 * @return int
 */
int ICNode::geoDelayDistribution(int routingMethod)
{
    int delayTimes = 0;
    if (routingMethod == 1)
    {
        delayTimes = geometric(maxSinPara);
    }
    else if (routingMethod == 2)
    {
        delayTimes = geometric(maxBiPara);
    }
    return delayTimes;
}

/**
 * @brief Throughout/matrix method
 * *******************************
 */
void ICNode::matrixRandom(int updateFlag)
{
    // for cross matrix methiod
    int lastStep = 0;
    if (updateFlag != 0)
    {
        lastStep = 1;
        if (this->matrixCrossList[1] == 2)
        {
            lastStep = 0;
        }
        matrixCrossList.clear();
    }
    for (int i = 0; i < this->chargeTimeSlots + 1; i++)
    {
        if (i == 0)
        {
            this->matrixCrossList.push_back(0);
        }
        else
        {
            this->matrixCrossList.push_back(lastStep + 1);
        }
    }
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
 * @brief produce msg packets
 *
 * @param dataID
 * @param conTrans
 * @return metaFrame*
 */
metaFrame *ICNode::produceMsg(int dataID, int seMark, int msgType)
{
    metaFrame *dMsg = new metaFrame("packets");
    dMsg->setFindCount(-1);
    dMsg->setActualHop(0);
    dMsg->setHop2Sink(this->shortestPathHop);
    dMsg->setSenderId(this->nodeID);
    dMsg->setSourceId(this->nodeID);
    dMsg->setDestinationId(this->destinationID);
    dMsg->setNextHopId(this->nextHopID);
    dMsg->setSendTime(0);
    dMsg->setMsgID(dataID);
    dMsg->setMsgType(msgType);
    dMsg->setMsgMark(seMark);
    dMsg->setRoundMark(this->roundNum);
    dMsg->setChargeSlot(this->chargeTimeSlots);
    dMsg->setPairMark(this->indictValue4Paired);
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
    metaFrame *dMsg = new metaFrame("acks");
    dMsg->setFindCount(0);
    dMsg->setSenderId(this->nodeID);
    dMsg->setDestinationId(0);
    dMsg->setSourceId(srcID);
    dMsg->setNextHopId(aimID);
    dMsg->setActualHop(0);
    dMsg->setHop2Sink(this->shortestPathHop);
    dMsg->setSendTime(0);
    dMsg->setMsgID(msgID);
    dMsg->setMsgType(1);
    dMsg->setMsgMark(0);
    dMsg->setPairMark(this->indictValue4Paired);
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

    if (this->topologyType == 1)
    {
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

// For compare
void ICNode::inidoubleercept()
{
    if (Deltadoubleercept < 0)
    {
        B_Ti = this->chargeTimeSlots + Deltadoubleercept;
    }
    else
    {
        B_Ti = Deltadoubleercept;
    }
}

double ICNode::calGeoEncounter()
{
    inidoubleercept();
    double count = 0;
    A_Ti = 0;
    while (DeltaWaitTime != 0)
    {
        int T_wait1 = geometric(pLama);
        A_Ti = A_Ti + T_wait1;
        if (A_Ti >= (chargeTimeSlots + 1))
        {
            A_Ti = (int)A_Ti % ((int)chargeTimeSlots);
        }
        DeltaWaitTime = A_Ti - B_Ti;
        count = count + 1;
    }
    return count;
}

double ICNode::calBiGeoEncounter()
{
    inidoubleercept();
    double count = 0;
    A_Ti = 0;
    while (DeltaWaitTime != 0)
    {
        int T_wait1 = geometric(pLama);
        int T_wait2 = geometric(pLama);
        A_Ti = A_Ti + T_wait1;
        if (A_Ti >= (chargeTimeSlots + 1))
        {
            A_Ti = (int)A_Ti % ((int)chargeTimeSlots);
        }
        B_Ti = B_Ti + T_wait2;
        if (B_Ti >= (chargeTimeSlots + 1))
        {
            B_Ti = (int)B_Ti % ((int)chargeTimeSlots);
        }
        DeltaWaitTime = A_Ti - B_Ti;
        count = count + 1;
    }
    return count;
}

void ICNode::initABNode()
{
    A_Ti = intuniform(0, this->chargeTimeSlots);
    B_Ti = intuniform(0, this->chargeTimeSlots);
    Deltadoubleercept = B_Ti - A_Ti;
}

void ICNode::geoCal()
{
    double sSum = 0;
    double biSum = 0;

    initABNode();
    double x = 0.05;
    while (x < 0.99)
    {
        double sTem = 0;
        double biTem = 0;
        pLama = x;
        if (x == 0.05)
        {
            for (double j = 0; j < trailNum; j++)
            {
                initABNode();
                sSum = sSum + calGeoEncounter();
                biSum = biSum + calBiGeoEncounter();
                maxSinPara = x;
                maxBiPara = x;
            }
            x = x + 0.05;
            continue;
        }

        if (x > 1)
        {
            break;
        }

        for (double j = 0; j < trailNum; j++)
        {
            initABNode();
            sSum = sSum + calGeoEncounter();
            biSum = biSum + calBiGeoEncounter();
            maxSinPara = x;
            maxBiPara = x;
        }

        if (sSum > sTem)
        {
            sSum = sTem;
            maxSinPara = x;
        }

        if (biSum > biTem)
        {
            biSum = biTem;
            maxBiPara = x;
        }
        x = x + 0.05;
    }
}
