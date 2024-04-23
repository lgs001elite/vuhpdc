
package src;
network RandomGrid
{
    parameters:
        int relayWidth = default(-1);
        int relayHeight  = default(-1);
        int chargingTime  = default(-1);
        int relayNum     = default(-1);
        double mapWidth    = default(-1);
        double mapHeight    = default(-1);
        int overalStrategy = default(-1); // 0: matrix; 1: geoSin; 2: geoBi
        int initFlag = default(0);
        int testFlag = default(-1); // for sink node
        int nodeNum         = default(0);
        double nodesWide    = default(0);
        bool backoffMechanism = default(true);
        bool multiPathSwitch = default(false);
    submodules:
        sink: ICNode;
        transceiver[300]: ICNode;       // 50  528 1224 3968
}



