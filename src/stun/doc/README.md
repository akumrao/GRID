# STUN

When using TCP for STUN, the STUN messages (binding request and response) are sent over a TCP connection, following specific framing methods as described in RFC 4571. 
Message Structure:
The structure of STUN messages remains consistent with the UDP-based version, including attributes like MAPPED-ADDRESS (for the reflexive address) and XOR-MAPPED-ADDRESS (for the server reflexive address). 

 
n ICE (Interactive Connectivity Establishment), "nomination" refers to the process of selecting and confirming a specific ICE candidate pair for establishing the media connection. After candidates are gathered and connectivity checks are performed, the devices agree on the best candidate pair to use for data transmission. This nomination process ensures both endpoints are synchronized and using the same candidate pair. 
Here's a more detailed explanation:
1. Gathering Candidates: ICE agents gather potential network addresses (candidates) that can be used to establish a connection. These include host, server-reflexive, and relayed candidates. 
2. Prioritizing and Checking Candidates: ICE prioritizes candidates based on their type and connectivity. The agents then perform connectivity checks using STUN binding requests to determine which candidate pairs are viable. 
3. Nomination: After the connectivity checks, the agents nominate the candidate pair that offers the best connectivity and performance. This involves agreeing on which local and remote candidate addresses to use. 
4. Signaling: A flag in the STUN binding request can be used to indicate which candidate pair has been nominated. 
5. Regular vs. Aggressive Nomination: 
Regular Nomination:
The controlling agent allows connectivity checks to continue until at least one valid candidate pair is found. Then, it sends another STUN binding request for the nominated candidate pair with a flag.
Aggressive Nomination:
The controlling agent sends a STUN binding request with the flag set on every candidate pair as it's tested.
6. Best Candidate Pair Selection: The chosen candidate pair is the one with the lowest latency and best overall connection quality. 
7. Data Transmission: Once a candidate pair is nominated and confirmed, media transmission can proceed over that path. 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * Host Candidates:
Server Reflexive Candidates
Relayed Candidates
The gethering process involves an ICE agent that queries both STUN and TURN servers

Step 2 Prioritizing Candidates
Once all the candidates are collected, they are prioritized. ICE has a priority formula that is used to assign a numeric priority to each candidate.

This priority number is then used to determine the order in which the candidates are tested. The formula has various considerations such as

Is the candidate a host
server reflexive or
relayed type
This formula assigns a higher value to the candidate that has a higher possibility to successfully establish and maintain the connection.

Generally, host candidates gets that top priority then server reflexive candidates and lastly relayed candidates.

Step 3 Checking Connectivity
Once the prioritizing is done then the ICE agent begins connectivity checks

STUN binding requests
ICE Continuations
Step 4 Selecting the Best Candidates
Once the connectivity checks are completed ICE needs to determine which candidate pair provides the best performance

Nomination of Pairs
Final Selection 
 
 * 
 * 
 * https://medium.com/@jamesbordane57/stun-server-what-is-session-traversal-utilities-for-nat-8a82d561533a
 * 
 * https://subspace.com/resources/stun-101-subspace
 
 * https://github.com/diederickh/WebRTC/blob/master/src/test_webrtc_ice_agent.cpp
 */
