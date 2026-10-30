

        
        take two laptop  A and B
       
       

       at A 
        cd /src/signaller_websocket# 
                
                

        ./reliableSctp 
                
        
        at B run 
        
         iptables -A OUTPUT -p udp -m statistic --mode random --probability 0.10 -j DROP
         iptables -A INPUT -p udp -m statistic --mode random --probability 0.10 -j DROP

        ./reliableSctp 
         
         with 
         dcInit.reliability.unordered = true; // by default datachannlel is reliable
         dcInit.reliability.maxRetransmits = 1;


         you will see lot of pakcet loss


         then do 

         // dcInit.reliability.unordered = true; // by default datachannlel is reliable
        // dcInit.reliability.maxRetransmits = 1;

        by default sctp is reliable 