# Design
This document discusses our design particularly in the perspective of how does our submission fulfill the requirements of the assignment as described in the Project expectations pdf.

**NOTE** PARTS TO BE COMPLETED ARE LABELED WITH DOUBLE CARETS <<>>

## Expectations

In the following section conditions and clauses from the expectations document are listed. The subcomponents of each clause/condition are numbered to the right and then listed directly below. 

If the box is checked then that condition is fulfilled by our application details are listed as subpoints.

**I. Component clause.** (2)

- [x] Application must have "a client with a User Interface (GUI, web-based, but NOT just a Command Line Interface) in any programming language"
    > - We have a web client see `Ledger`. It is written in TypeScript using Svelte.

- [x] Application must have "A server, implemented in C++, which must be deployed and accessible over a network."
    > - We have a server implemented in C++, see `Folium`. It is deployed to <<AWS/GCP/AZURE>>. It's API is publicly accessible on the internet over HTTP.

**II. App Communication (Network Protocol) clause.** (1)

- [x] "The client must communicate with the server over a network, following an appropriate model for the application type. Examples include HTTP..."
    > - Our app communicates over HTTP between the client and user. The client is also distributed over HTTP from a CDN ([Vercel](http://www.vercel.com)).

**III. Non-trivial Deployment**

- [x] "...additional points if your server is deployed in a non-trivial manner. This means avoiding a setup where both the client and server run on the same local machine... your deployment should enable external users to connect. You can achieve this by hosting the server on a free cloud service..."
    > - The client is deployed on vercel and publicly accessible to anyone at the URL <<URL>>. The server is deplyed on <<AWS/GCP/AZURE>> and the API is also publicly accessible.

**IV. Transactions and ACID properties**

- [x] "Your application must include the concept of transactions, meaning a sequence of operations that follow ACID properties"
    > - Our application uses Postgres transactions on all reads and writes.

**V. Multiple Concurrent Transactions**

- [x] "The system should support multiple concurrent transactions efficiently."
    > - Our server assigns each request a thread keeping IO-heavy tasks like reading from DB from blocking the server.
    > - Our server creates additional processes to manage note processing to keep the server from blocking during CPU-intensive tasks like note analysis or merging.
    > - We have tested the software up to <<n>> concurrent users without significantly increased times.

**VI. Access control & Session Management**

- [x] "Your application should include a notion of users, with appropriate access or session management."
    > - Our app has users that can only access certain data through access control and on the server side. 
    > - User data is encrypted on the database.
    > - Session management is handled with state information server-side and JSON web tokens.

**VII. Multi-user & Transaction Concurrency + Scalability**

- [x] "The server should handle multiple users and transactions concurrently..."
    > - The server has a queue for handling an overflow of connections, multithreading and multiprocessing to reduce downtime during IO-blocks and processing tasks.
    - See Criteria V.

- [x] "... and be designed for scalability"
    > - Vertically, the server is built to:
    >   - acquire additional processes as processing needs increase and,
    >   - multi-thread IO and connection tasks.

**VIII. Learned Concepts**

- [x] Process/Thread scheduling.
    > - The server's connection manager & IO uses a thread pool with a priority queue based on the request to handle connections. The priority system is described below in [1]. This is thread scheduling.

- [x] Process/Thread synchronization.
    > - Note analysis processes are blocked from writing to the database during server interactions using a mutex.

- [x] Multithreading and Multiprocessing
    > - We have a thread pool for connection management and seperate processes that handle note analysis.

- [x] Deadlock Handling
    > - PostgreSQL has deadlock handling with transactions (+ isolation levels).

## Footnotes
[1] Priority Queue  
The requests recieved by the IO manager from the connection manager or the note analysis processes are preemptively scheduled using a priority system based on the request type. The hierarchy is described below:
1. Sign in
2. Download Note
3. Upload Note
4. Note change (post note-analysis)