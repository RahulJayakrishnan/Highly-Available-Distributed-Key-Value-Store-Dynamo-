# Highly-Available-Distributed-Key-Value-Store-based-on-Dynamo

In this project we have implemented a distributed key-value store (GTStore). We have designed
and evaluated GT store on (i) scalability of overall system, (ii) availability of the system, and
(iii) system resilience to temporary node failures (reliability). (iv) eventual consistency.

## Design Overview:

We have three components in the design: GT Store, Driver Application, and Client Library.
GT Store consists of N number of storage nodes and K number of managers(coordinators).
The storage nodes are processes that perform key-value storage in memory. Coordinators
perform membership management, liveness check and load balancing at the granularity of a
session.
Driver application is a series of read modify write operations.
Client library offers driver applications to initialize a session with the coordinator, put a
value to a storage node for a certain key, and retrieve the value using the key.
APIs
`init(&env)`
Client uses this API to initialize the session and obtain the preference list of storage nodes
based on the client ID.
Elements of env Type Description
Client ID Integer ID of the client
Storage Node 1 Integer Port for priority 1
Storage Node 2 Integer Port for priority 2
Storage Node 3 Integer Port for priority 3
Cart String Items in cart
`put(&env, key, value)`
Insert/Update cart value with a key. Put function returns when two nodes have been
successfully written to. If any node in preference list is inactive, the function re-initializes the
preference list.
`get(&env, key)`
Retrieve value stored in a node for the key. Functions returns a value when read from two
nodes are successful.
`finalize(&env)`
Frees environment variable and terminates the session.

## Design Principles:
### Data Partitioning
In order to balance the keys across available node space, our design employs a hashing
mechanism to map a client to nodes associated with it. On initialization, the client contacts a
manager(coordinator). The manager performs a hashing operation (client id % num of
storage nodes), and returns the preference list of storage nodes for the client ID. It also
maintains a vector that monitors the load on each node in the system and orders the
preference list accordingly.
Once the preference list is obtained, the client no longer needs to communicate with the
coordinator. The consistent hashing method chosen distributes clients evenly on the node
ring. The consistent hashing method intrinsically provides fault tolerance which will be
discussed in further sections.
Cons:
Every client must contact a coordinator in order to initialize the session. Even though this is
a lightweight operation, it could become a bottleneck. We have provided multiple
coordinator nodes for this reason. If a client finds that a coordinator is unreachable, it tries
to connect to another coordinator. This can further be improved by making each
coordinator responsible for a set of client/storage nodes based on the client id.
### Data Replication
Each read and write operation is carried out on a quorum of storage nodes. In our design, a
read/write is deemed successful if it is performed on two nodes. Since there would be an
overlap between the read and write quorums, we have opted for the eventual consistency
model, where reconciliation happens on reads.
During writes, we attempt to connect to two of the nodes from the preference list and
perform a write operation. The message is sent via socket in the following format.
Flag is used to signal the storage nodes the type of operation to be carried out.
p -> Write, g -> Read
On read request, the storage node indexes into a vector of carts using the key, and returns
the cart data structure to the client.
On a write request, the storage node indexes into a vector of carts and updates the cart and
increments the local timestamp(version number). If the cart for the client doesn’t exist, it
will be created with a timestamp(version number) of zero.
Pros:
This replication scheme ensures that as long as write quorum is active in the network, write
is always guaranteed. This is crucial for certain business models like ecommerce shopping
carts, where customers should always be able to add items to carts, failing which it results in
poor customer experience and loss of revenue.
Cons:
This design can tolerate only one failure in the read quorum. Two simultanious failures do
not guarantee the most up to date cart. Failure of all three replicas results in a failed read.
Data Consistency:
Our system uses a sloppy quorum for writes: we write to N healthy nodes in the preference
list for the client. If N healthy nodes are not available, the client initializes with the
coordinator to obtain an updated preference list with healthy nodes. In our design,
preference list consists of three healthy nodes. The client chooses two nodes out of them to
write to. The sloppy quorum ensures the writes are always guaranteed as long as two
healthy nodes are available in the entire ring of nodes. Each write also updates the local
timestamp(version number) on the nodes being written to, and this information is stored
along with the cart.
Client initiates two out of three nodes from the preference list and compares the versions
values of carts from the two servers.
a) If both versions are the same, then no reconciliation is required. Any of the obtained
cart can be returned.
b) If the versions differ, the cart corresponding to higher version is returned and the
contents of the two carts are compared. If the contents differ, an internal put
request is made to the storage node with the lower version to add the cart from the
higher version number, thus reconciling on read.
### Handling Temporary Failures
Our design tasks the coordinator with liveness check of the storage nodes. It repeatedly
polls the storage nodes and keeps track of their liveness in an array. This array is consulted
when an initialization request if received from a client. If a node is found to be down, the
successor node is returned for that particular slot in the client’s preference list.
Node failures can happen post initialization. If the client is unable to connect to any nodes in
the preference list during the write, the client re-initializes to update its preference list of
active nodes, thus guaranteeing that the writes and future reads are successful. When the a
node joins/re-joins, read reconciliation method ensures that the node is eventually made up
to date with all the key values.
## Design Tradeoffs:
### Scalability
Access times after initialization is O(1) regardless of number of storage nodes on the ring.
The tradeoff is that the coordinator has to do with all the heavy lifting associated with load
balancing and client initialization.
Consistency
The sloppy quorum and read reconciliation techniques allow for guaranteed as well as quick
writes, as we need to write only to the quorum associated with the client(2 servers in our
case). The downside is, reads might be slower due to potential reconciliation, and reads
cannot tolerate more than two simultaneous node failures from the preference list.
### Availability:
The storage nodes are always available for the writes. But reads cannot tolerate more than
two simultaneous node failures from the preference list.
Fault Tolerance:
The system tolerates failures even after client initialization. The downside is that the writes
occurred during failures take longer due to re-initialization.
Results/Test Cases:
Our design is stable, scalable and fault tolerant. The following experiments are used to verify
these properties.
### Node failure/Rejoin Detection:
At the coordinator end, our system detects node failure with the worst case time of
[Number of Storage Nodes] * [Time to connect to a single node] + [Time between successive
heartbeat checks

For 200 nodes, a node failure will be detected in 200 * 30 us + 10000 us = 10600 us
Time to initialize

For 20 nodes, average time to initialize the client is 237us

For 200 nodes, average time to initialize the client is 229us

Time for put operation with no node failures

For 20 storage nodes, average time for put operation is 82 us

For 200 storage nodes, average time for put operation is 98 us

Time for get operation with no node failures

For 20 storage nodes, average time for get operation is 482 us.

For 200 storage nodes, average time for get operation is 517 us

Time for put operation with nodes failing post initialization

For 20 storage nodes, average time for put operation is 1166 us

For 200 storage nodes, average time for put operation is 1219 us
