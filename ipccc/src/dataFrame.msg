packet metaFrame {
    int findCount;        //  feedback hop values
    int hop2Sink;        //  feedback hop values
    int actualHop;       // actual hops
    int destinationId;   // sinkNode
    int nextHopId;       // next node
    int sourceId;        // origin node
    int senderId;        // sender node
    int msgType;         // packets or acks
    int msgID;           // unique numbers of packets from source node
    int msgMark;         // the start and end marks of  the packets sequence
    int pairMark;        // indicate if node ia paired
    int roundMark;       // to decide when the simulation ends
    int chargeSlot;      // indicate the senders'charging time 
    int endTrans;        // indicate the senders'charging time 
    simtime_t sendTime;  // recording sending time
}

