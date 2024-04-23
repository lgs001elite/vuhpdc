package src;
simple ICNode
{
    parameters:
        @signal[arrivalHop](type=long);
        @statistic[arrivalHop](title="hop count"; source="arrivalHop"; record=vector,histogram,last,min,max; interpolationmode=none);
        
        @signal[arrivalSlot](type=long);
        @statistic[arrivalSlot](title="slot count"; source="arrivalSlot"; record=vector,histogram,last,min,max; interpolationmode=none);
        
        @signal[arrivalSourceID](type=long);
        @statistic[arrivalSourceID](title="source ID"; source="arrivalSourceID"; record=vector,histogram,last,min,max; interpolationmode=none);
        
        @signal[arrivalHop2Sink](type=long);
        @statistic[arrivalHop2Sink](title="Hop2Sink"; source="arrivalHop2Sink"; record=vector,histogram,last,min,max; interpolationmode=none);
        
        @signal[findTime](type=simtime_t);
        @statistic[findTime](title="findTime"; source="findTime"; record=vector,histogram,last,min,max; interpolationmode=none);
        
        @signal[simuTime](type=simtime_t);
        @statistic[simuTime](title="simuTime"; source="simuTime"; record=vector,histogram,last,min,max; interpolationmode=none);

        int     transmissionRange = default(10) ;     
        double  transmissionDuration  = default(0.001);
        int     nodeID                        = default(0);
        bool    sender                        = default(false);
        int     routingTableSize  = default(0);
        int     nodeType          = default(0); // 0: SinkNode; 1: Transceiver
        int     tryTransRound     = default(3);

    gates:
       input   ICNodeRadioIn @directIn;
}
