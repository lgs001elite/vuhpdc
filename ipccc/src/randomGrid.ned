package src;

network RandomGrid
{
    
    parameters:
        int backoffMechanism = default(-1);
        int variousSwitch = default(-1);
        int topologyType = default(-1);
        int relayWidth = default(-1);
        int relayHeight  = default(-1);
        int nodesWide  = default(-1);
        int nodesHigh  = default(-1);
        int chargingTime  = default(-1);
        int maxSimTime   = default(-1);
        int overalStrategy = default(-1); // 0: matrix; 1: geoSin; 2: geoBi
        int initFlag = default(0);
        int hNumber;
        int rNumber;
        int distributedWay  = default(0);
        int multiPath       = default(0);
        bool reverseBiasSwitch = default(false);
        int varyingChargingSwitch = default(0);
    submodules:
        sink: ICNode;
        transceiver[440]: ICNode;       // 50  528 1224 3968
}



