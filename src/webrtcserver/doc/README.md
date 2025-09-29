
https://www.rfc-editor.org/rfc/rfc6627.html#page-9

3.1.  Structure of the DS Field

     Figure 4 shows the structure of the DS and ECN fields.  [RFC0793]
     defined the 8-bit TOS octet and [RFC2474] redefined it as the DS
     field, including the two least significant bits as currently unused
     (CU).  [RFC3168] assigned the two CU bits to ECN and [RFC3260]
     redefined the DS field as only the most significant 6-bits of the
     (former) IPv4 TOS octet, thus separating the two-bit ECN field from
     the DS field.

         0   1   2   3   4   5   6   7
       +---+---+---+---+---+---+---+---+
       |          DS           |  ECN  |
       +---+---+---+---+---+---+---+---+

       DS: Differentiated Services field [RFC2474], [RFC3260]
       ECN: ECN field [RFC3168]

       Figure 4: The Structure of the DS and ECN Fields

3.2.  Constraints from the DS Field

   The Differentiated Services Codepoint (DSCP) set in the DS field
   indicates the per-hop behavior (PHB), i.e., the treatment IP packets
   receive from nodes in a DS domain.  Multiple DSCPs may indicate the
   same PHB.  PCN-traffic is high-priority traffic, which uses a DSCP
   (or DSCPs) that indicates a PHB with preferred treatment.