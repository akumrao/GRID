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




USE-CANDIDATE Attribute:
When the controlling agent selects a candidate pair that it wants the other peer to use, it includes the USE-CANDIDATE attribute in a STUN binding request. This tells the other peer that this specific candidate is the one being nominated.

Nomination:
The nomination process involves sending a STUN binding request with the USE-CANDIDATE attribute to the selected candidate pair. The controlling agent then uses this nominated candidate pair for subsequent data transfer.



In the context of WebRTC and the Interactive Connectivity Establishment (ICE) process, a candidate pair represents a potential connection endpoint between two peers. The terms "default", "valid", and "nominated" describe different stages and characteristics of these candidate pairs:
Default:
A default candidate pair is one that is initially considered for use, typically based on the information provided in the Session Description Protocol (SDP) offer and answer. 
Valid:
A candidate pair is considered valid if it passes initial connectivity tests, such as STUN binding requests, indicating that a connection is potentially possible through that pair. 
Nominated:
A candidate pair is nominated when it's selected and marked for use as the final connection endpoint. The controlling agent (the peer that initiates the nomination) sends a STUN request with a specific flag to indicate that this candidate pair should be used for the WebRTC connection, according to a Microsoft document. 


 * https://vocal.com/networking/ice-interactive-connectivity-establishment/
 *
 * https://info.support.huawei.com/hedex/api/pages/EDOC1100331435/AEM10132/04/resources/dc/dc_fd_stun_0005.html
 * 
 * https://github.com/diederickh/WebRTC/blob/master/src/test_webrtc_ice_agent.cpp
 *
 * https://datatracker.ietf.org/doc/html/rfc5245
 *
 * 

 */


In IPv6, while there's no direct equivalent to IPv4's private address ranges, there are "Unique Local Addresses" (ULAs) that serve a similar purpose. ULAs are not publicly routable and are intended for use within private networks. Public IPv6 addresses, on the other hand, are globally unique and routable on the internet. 
Key Differences:
Scope:
Public IPv6 addresses are globally routable, while ULAs are only for use within a local network. 
Use:
Public addresses are used for internet communication, while ULAs are for internal network communication. 
Routability:
Public addresses are routed on the internet, while ULAs are not. 
Example:
A typical public IPv6 address might look like 2001:0db8:0000:0000:0000:0000:0000:0001, while a ULA might start with fd (e.g., fd00:0000:0000:0000:0000:0000:0000:0001). 
NAT:
While IPv4 used NAT to translate private IPs to public IPs, in IPv6, a similar mechanism called "Network Address Translation, Protocol v6" (NPTv6) can be used for prefix translation. 


Why is NAT not Needed in IPv6? — RapidSeedboxNPTv6, short for Network Prefix Translation for IPv6, is a method used to translate IPv6 addresses, specifically the prefix portion of an address, at the edge of a network. It's similar to NAT (Network Address Translation) in IPv4 but for IPv6 and only translates the network prefix, not the host part of the address. 

Imagine your home network has an IPv6 address block (the prefix) of 2001:db8::/48, and your internet provider gives you a different IPv6 address block (the prefix) of 2001:0:0:0::/48. NPTv6 would translate the 2001:db8::/48 prefix to 2001:0:0:0::/48 when communicating with the internet, allowing your devices to use their internal addresses while still being reachable on the internet. 




NAT hole punching is not directly required for IPv6, as IPv6's design eliminates the need for NAT. However, firewalls can still block unsolicited incoming traffic, making hole punching a potential workaround, although it's less necessary in IPv6 than in IPv4. 
