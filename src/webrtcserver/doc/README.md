
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



udp listen

 nc -6 -ul -p 6111 

 nc  -ul -p 6111 

upd send 

nc -6 -vzu 2401:4900:62a5:400d:a521:aa30:18a6:e80b 6111

nc -vzu 27.63.245.71 6111

Free turn server
https://dev.to/aprogrammer22/list-of-free-stun-and-turn-servers-open-relay-project-3a70

https://gist.github.com/sagivo/3a4b2f2c7ac6e1b5267c2f1f59ac6c6b


Free webpage hosting 

https://cpanel.infinityfree.com/panel/index.php
https://umrao.free.nf

free storage
https://mega.nz/fm/4QtW0SgD

https://app.netlify.com/drop

https://www.techradar.com/web-hosting/best-

https://github.com/themactep/thingino-firmware.git


# List files in folder dd
curl -L \
  -H "Accept: application/vnd.github.object" \
  -H "Authorization: Bearer <yourtocken>" \
  -H "X-GitHub-Api-Version: 2022-11-28" \
  https://api.github.com/repos/akumrao/akumrao.github.io/contents/dd


#add new file 
 curl -X PUT \
    -H "Accept: application/vnd.github+json" \
    -H "Authorization: Bearer  <yourtocken>" \
    https://api.github.com/repos/akumrao/akumrao.github.io/contents/dd/file1.txt \
    -d '{
      "message": "Commit message for the new file",
      "content": "bXkgbmV3IGZpbGUgY29udGVudA=="
    }'
