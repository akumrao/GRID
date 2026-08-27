#include "sdpcommon.h"
#include "base/logger.h"
#include <Agent.h>

//#include "Settings.h"

using namespace base;
using namespace stun;
using namespace std::chrono_literals;
using std::chrono::system_clock;

//#define arvind 1

namespace rtc {

//	enum class Type { Unknown, Host, ServerReflexive, PeerReflexive, Relayed };
    
    
    
     int ice_type_suffix(const Candidate *candidate, const char **type, char **suffix)
     {

	switch (candidate->mType) {
            case Candidate::Type::Host:
		*type = "host";
		break;
	case Candidate::Type::PeerReflexive:
		*type = "prflx";
		break;
	case Candidate::Type::ServerReflexive:
		*type = "srflx";
		*suffix = "raddr 0.0.0.0 rport 0"; // This is needed for compatibility with Firefox
		break;
	case Candidate::Type::Relayed:
		*type = "relay";
		*suffix = "raddr 0.0.0.0 rport 0"; // This is needed for compatibility with Firefox
		break;
	default:
		SError << "Unknown candidate type";
		return -1;
	}

        return 0;
    }

    
   int ice_generate_candidate_sdp( Candidate *candidate, char *buffer, size_t size) {

	const char *type = NULL;
	char *suffix = NULL;
       
       int ret = ice_type_suffix(candidate, &type, &suffix );
       if(ret < 0)
       {
           exit(0);
       }
       
       char *add =  (char *)candidate->address().c_str();
               
	return snprintf(buffer, size, "a=candidate:%s %u UDP %u %s %u typ %s%s%s",    candidate->mFoundation.c_str(), candidate->mComponent, candidate->mPriority, 
	                add , candidate->port(), type, suffix ? " " : "",
	                suffix ? suffix : "");
   }


       
    int ice_generate_sdp(const ice_description_t *description, char *buffer, size_t size) {
	if (!*description->ice_ufrag || !*description->ice_pwd)
		return -1;

	int len = 0;
	char *begin = buffer;
	char *end = begin + size;

	// Round 0: description
	// Round i with i>0 and i=count+1: candidate i-1
	// Round count + 1: end-of-candidates and ice-options lines
	for (int i = 0; i < description->candidates_count + 2; ++i) {
		int ret;
		if (i == 0) {
			ret = snprintf(begin, end - begin, "a=ice-ufrag:%s\r\na=ice-pwd:%s\r\n",
			               description->ice_ufrag, description->ice_pwd);
			if (description->ice_lite)
				ret = snprintf(begin, end - begin, "a=ice-lite\r\n");

		} else if (i < description->candidates_count + 1) {
			const Candidate *candidate = description->candidates + i - 1;
			if (candidate->mType == Candidate::Type::ServerReflexive ||
			    candidate->mType == Candidate::Type::ServerReflexive)
				continue;
			char tmp[4096];
			if (ice_generate_candidate_sdp((Candidate*)candidate, tmp, 4096) < 0)
				continue;
			ret = snprintf(begin, end - begin, "%s\r\n", tmp);
		} else { // i == description->candidates_count + 1
			// RFC 8445 10. ICE Option: An agent compliant to this specification MUST inform the
			// peer about the compliance using the 'ice2' option.
			if (description->finished)
				ret = snprintf(begin, end - begin, "a=end-of-candidates\r\na=ice-options:ice2\r\n");
			else
				ret = snprintf(begin, end - begin, "a=ice-options:ice2,trickle\r\n");
		}
		if (ret < 0)
			return -1;

		len += ret;

		if (begin < end)
			begin += ret >= end - begin ? end - begin - 1 : ret;
	}
	return len;
    }
  

    const char *skip_prefix(const char *str, const char *prefix) {
	size_t len = strlen(prefix);
	return strncmp(str, prefix, len) == 0 ? str + len : str;
    }
    
    bool match_prefix1(const char *str, const char *prefix, const char **end)
    {
	*end = skip_prefix(str, prefix);
	return *end != str || !*prefix;
    }
    
  
    bool comp(Candidate a, Candidate b)
    {
        return a.priority() > b.priority();
    }
        
    
    
    
    
    
    
    
socklen_t addr_get_len(const struct sockaddr *sa) {
	switch (sa->sa_family) {
	case AF_INET:
		return sizeof(struct sockaddr_in);
	case AF_INET6:
		return sizeof(struct sockaddr_in6);
	default:
		SInfo << "Unknown address family " <<  sa->sa_family;
		return 0;
	}
}  
    
    
    
    
    
bool addr_is_any(const struct sockaddr *sa) {
	switch (sa->sa_family) {
	case AF_INET: {
		const struct sockaddr_in *sin = (const struct sockaddr_in *)sa;
		const uint8_t *b = (const uint8_t *)&sin->sin_addr;
		for (int i = 0; i < 4; ++i)
			if (b[i] != 0)
				return false;

		return true;
	}
	case AF_INET6: {
		const struct sockaddr_in6 *sin6 = (const struct sockaddr_in6 *)sa;
		if (IN6_IS_ADDR_V4MAPPED(&sin6->sin6_addr)) {
			const uint8_t *b = (const uint8_t *)&sin6->sin6_addr + 12;
			for (int i = 0; i < 4; ++i)
				if (b[i] != 0)
					return false;
		} else {
			const uint8_t *b = (const uint8_t *)&sin6->sin6_addr;
			for (int i = 0; i < 16; ++i)
				if (b[i] != 0)
					return false;
		}
		return true;
	}
	default:
		return false;
	}
}
    

bool addr_is_local(const struct sockaddr *sa) {
	switch (sa->sa_family) {
	case AF_INET: {
		const struct sockaddr_in *sin = (const struct sockaddr_in *)sa;
		const uint8_t *b = (const uint8_t *)&sin->sin_addr;
		if (b[0] == 127) // loopback
			return true;
		if (b[0] == 169 && b[1] == 254) // link-local
			return true;
		return false;
	}
	case AF_INET6: {
		const struct sockaddr_in6 *sin6 = (const struct sockaddr_in6 *)sa;
		if (IN6_IS_ADDR_LOOPBACK(&sin6->sin6_addr)) {
			return true;
		}
		if (IN6_IS_ADDR_LINKLOCAL(&sin6->sin6_addr)) {
			return true;
		}
		if (IN6_IS_ADDR_V4MAPPED(&sin6->sin6_addr)) {
			const uint8_t *b = (const uint8_t *)&sin6->sin6_addr + 12;
			if (b[0] == 127) // loopback
				return true;
			if (b[0] == 169 && b[1] == 254) // link-local
				return true;
			return false;
		}
		return false;
	}
	default:
		return false;
	}
}

static int has_duplicate_addr(struct sockaddr *addr, const addr_record_t *records, size_t count) {
	for (size_t i = 0; i < count; ++i) {
		const addr_record_t *record = records + i;
		if (record->addr.ss_family == addr->sa_family) {
			switch (addr->sa_family) {
			case AF_INET: {
				// For IPv4, compare the whole address
				const struct sockaddr_in *rsin = (const struct sockaddr_in *)&record->addr;
				const struct sockaddr_in *asin = (const struct sockaddr_in *)addr;
				if (memcmp(&rsin->sin_addr, &asin->sin_addr, 4) == 0)
					return true;
				break;
			}
			case AF_INET6: {
				// For IPv6, compare the network part only
				const struct sockaddr_in6 *rsin6 = (const struct sockaddr_in6 *)&record->addr;
				const struct sockaddr_in6 *asin6 = (const struct sockaddr_in6 *)addr;
				if (memcmp(&rsin6->sin6_addr, &asin6->sin6_addr, 8) == 0) // compare first 64 bits
					return true;
				break;
			}
			}
		}
	}
	return false;
}

uint16_t addr_get_port(const struct sockaddr *sa) {
	switch (sa->sa_family) {
	case AF_INET:
		return ntohs(((struct sockaddr_in *)sa)->sin_port);
	case AF_INET6:
		return ntohs(((struct sockaddr_in6 *)sa)->sin6_port);
	default:
		SWarn <<  "Unknown address family " <<   sa->sa_family;
		return 0;
	}
}

int addr_set_port(struct sockaddr *sa, uint16_t port) {
	switch (sa->sa_family) {
	case AF_INET:
		((struct sockaddr_in *)sa)->sin_port = htons(port);
		return 0;
	case AF_INET6:
		((struct sockaddr_in6 *)sa)->sin6_port = htons(port);
		return 0;
	default:
		SWarn <<  "Unknown address family "<<   sa->sa_family;
		return -1;
	}
}

int udp_get_addrs(addr_record_t &bound, addr_record_t *records, size_t count, char *mac_addr)
{
    
    if (!addr_is_any((struct sockaddr *)&bound.addr)) {
            if (count > 0)
                    records[0] = bound;

            return 1;
    }


    uint16_t port = addr_get_port((struct sockaddr *)&bound.addr);
    
    addr_record_t *current = records;
    addr_record_t *end = records + count;
    int ret = 0;

    
    uv_interface_address_t *info;
        int icount, i;

        uv_interface_addresses(&info, &icount);
        i = icount;

        STrace <<  " Number of interfaces: " <<  icount;
        while (i--) 
        {
            uv_interface_address_t interface_a = info[i];
        

            Candidate candidate;
            candidate.mType = Candidate::Type::Host;
            
            SInfo  <<  " Name: " <<  interface_a.name;
          
                       
            if(!interface_a.is_internal)
            {
 
                struct sockaddr* sa = (struct sockaddr*)&interface_a.address;
                #if arvind
                
                if(Settings::configuration.is_ipv4 && sa->sa_family == AF_INET6 )
                {
                  continue;    
                }else if(!Settings::configuration.is_ipv4 && sa->sa_family == AF_INET )
                {
                  continue;  
                }
                #endif

           	socklen_t len;
		if (sa &&
		    (sa->sa_family == AF_INET
                    ||  (sa->sa_family == AF_INET6 && bound.addr.ss_family == AF_INET6)   // arvind to disable ipv6
                    ) &&
		    !addr_is_local(sa) && (len = addr_get_len(sa)) > 0) {
			if (!has_duplicate_addr(sa, records, current - records)) {
				++ret;
				if (current != end) {
					memcpy(&current->addr, sa, len);
					current->len = len;
					addr_set_port((struct sockaddr *)&current->addr, port);
					++current;
                                        
                                        if(ret == 1 && mac_addr)
                                        {
                                            snprintf(mac_addr,18, "%02x%02x%02x%02x%02x%02x",
                                            (unsigned char)interface_a.phys_addr[0], (unsigned char)interface_a.phys_addr[1],
                                            (unsigned char)interface_a.phys_addr[2], (unsigned char)interface_a.phys_addr[3],
                                            (unsigned char)interface_a.phys_addr[4], (unsigned char)interface_a.phys_addr[5]);
                                                SInfo  <<  " mac_addr: " <<  mac_addr;
                                        }
				}
			}
		}
               
            }

        }
 	 if (info != nullptr) {
            uv_free_interface_addresses(info, icount);
        }


    return ret;
}
    
    

    
    
}
