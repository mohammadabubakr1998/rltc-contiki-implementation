# rltc-contiki-implementation
Implementation of "A Reliable and Lightweight Trust Computing Mechanism for IoT Edge Devices Based on Multi-Source Feedback Information Fusion" (https://ieeexplore.ieee.org/document/8352868)
On contiki / cooja simulator.
Complie Broker.c as node 1 and Node.c as other nodes.
When button on Node.c will be clicked it will request for trust to node 1 (Broker Node). Then broker will send request to other nodes in the network for trust values.
Other nodes will then generate trust value (till now based on randomly generated values). And send the trust value to broker node.
Broker node will then send this trust value to the requestor node.

