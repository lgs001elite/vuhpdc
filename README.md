# Simulater
   The simalater we use is [Omnetpp](https://omnetpp.org/). A popular network simulater for researchers. 
   The version is [5.6.2](https://github.com/omnetpp/omnetpp/releases/download/omnetpp-5.6.2/omnetpp-5.6.2-src-windows.zip).
   The host system is Windows10 Enterprise version. 

# How to use the code
    The simulator consists of there parts mainly including NED file, INI file, and cpp files.
## NED file
      It is used to define the network componets by the NED language. It is easy to grasp.
      For example, we use ICNODe.ned to define the single node's network properties.
      In addition, we can also define the statistics to collect simulation data from every node.
   ![ICNode](https://github.com/lgs001elite/vuhpdc/assets/65667947/c5e81e47-4963-427c-93b7-56f14a6dbe15)
      we use the randGrid.ned file to define the holistic network's properties.
      For example, the number of nodes in the network.
   ![Network](https://github.com/lgs001elite/vuhpdc/assets/65667947/9dd28ab1-b1c1-4346-9ecb-77507c6ea2c5)
## INI file
       We use INI file to define some global parameters for the simulation. 
       For example, in our setup, we can define the charging time of the node, the size of the topology, etc..
## cpp files
       cpp files are responsible for defining the concrete the actions.
   It does not like other cpp/c projects starting from main functions.
   Instead, it starts with the **initialize()** function.
       The nodes' actions are completed by the message mechanism.
       For example, we fine the msg for every node. Nodes' logical action transfermation by the messager.
       ![Messager](https://github.com/lgs001elite/vuhpdc/assets/65667947/a6741d85-6596-4289-9e5d-42f40903cdf6)

### 
  **msg** file define the packets content. It can be altered according to actual requirements.

# how to import the code to the simulator?
  1. You can download the code. 
  2. From the file ==> import the project **or** open projec from file system.
  3. Then you compile it. 
  4. There maybe some errors. You need to change the path on NED files.
  5. In addition, you need to configure the makefile. The name may need to change to adapt to the new project you side.
  

