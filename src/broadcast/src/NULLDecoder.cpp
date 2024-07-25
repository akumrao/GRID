

#include "NULLDecoder.h"
#include "base/logger.h"
#include "base/datetime.h"
#include "tools.h"
#include "base/filesystem.h"
//#include "livethread.h"
#include "webrtc/signaler.h"




//#include "rtc_base/ref_counted_object.h"
//#include "rtc_base/atomic_ops.h"
#include <chrono>
#include "base/platform.h"
#include "tools.h"
//#include "common_video/h264/sps_parser.h"
//#include "common_video/h264/h264_common.h"

#include "H264Framer.h"
#include "Settings.h"


#include <thread>

//////////////////////////////////////////////////////////////////////////

namespace base {
    namespace web_rtc {

        
//  int remove_directory(const char *path) {
//   DIR *d = opendir(path);
//   size_t path_len = strlen(path);
//   int r = -1;
//
//   if (d) {
//      struct dirent *p;
//
//      r = 0;
//      while (!r && (p=readdir(d))) {
//          int r2 = -1;
//          char *buf;
//          size_t len;
//
//          /* Skip the names "." and ".." as we don't want to recurse on them. */
//          if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, ".."))
//             continue;
//
//          len = path_len + strlen(p->d_name) + 2; 
//          buf = malloc(len);
//
//          if (buf) {
//             struct stat statbuf;
//
//             snprintf(buf, len, "%s/%s", path, p->d_name);
//             if (!stat(buf, &statbuf)) {
//                if (S_ISDIR(statbuf.st_mode))
//                   r2 = remove_directory(buf);
//                else
//                   r2 = unlink(buf);
//             }
//             free(buf);
//          }
//          r = r2;
//      }
//      closedir(d);
//   }
//
//   if (!r)
//      r = rmdir(path);
//
//   return r;
//}
 /*
static  unsigned char *find_start_code( unsigned char *h264_data, int h264_data_bytes, int *zcount)
{
    unsigned char  *eof = h264_data + h264_data_bytes;
    unsigned char  *p = h264_data;
    do
    {
        int zero_cnt = 1;
        unsigned char * found = (uint8_t*)memchr(p, 0, eof - p);
        p = found ? found : eof;
        while (p + zero_cnt < eof && !p[zero_cnt]) zero_cnt++;
        if (zero_cnt >= 2 && p[zero_cnt] == 1)
        {
            *zcount = zero_cnt + 1;
            return p + zero_cnt + 1;
        }
        p += zero_cnt;
    } while (p < eof);
    *zcount = 0;
    return eof;
}



    //////////////////////////////////////////////////////////////////////
int parse_nal(  unsigned char **nal, int &length , int & payload_type, int &sizeof_nal, int &frameType )
{

    if( length <  5 )
    return 0;
    

    const unsigned char *eof = *nal + length;
    int zcount;
    unsigned char *start = find_start_code(*nal, length, &zcount);

    *nal = start;
    
    length = length - zcount;

    if( zcount )
    {
        payload_type = start[0] & 31;
        frameType =  (( start[0] & 96) >> 5);

        switch( payload_type)
        {
        case 7:
            // printf("sps\n");
            break;
        case 8:
           // return 1;
            printf("pps\n");
            break;
        case 5:
             printf("idr\n");
             //return 1;
           break;

         case 1:
              printf("nonidr\n");
             //return 1;
           break;

        default:
              //printf("unknown\n");
            break;
        };

    }
    else
        return 0;




    const uint8_t *stop = start;
    if (start)
    {
        stop = find_start_code(start, (int)(eof - start), &zcount);
        while (stop > start && !stop[-1])
        {
            stop--;
        }
    }

    sizeof_nal = (int)(stop - start - zcount);
    
    length = length - sizeof_nal;
            
    return 1;          
    
     
}
   */
        
        
            //////////////////////////////////////////////////////////////////////
        int parse_nal(  unsigned char *nal, int length , int & payload_type,  int &frameType )
        {

            if( length <  5 )
            return 0;

            if( !memcmp(nal, startcode, 4  ))
            {      
                payload_type = nal[4] & 31;
                frameType =  (( nal[4] & 96) >> 5);

                switch( payload_type)
                {
                case 7:
                   //  printf("sps\n");
                    break;
                case 8:
                   // printf("pps\n");
                    break;
                case 5:
                   // printf("idr\n");
         
                   break;

                 case 1:
                  //    printf("nonidr\n");
                    
                   break;

                default:
                      printf("unknown\n");
                      return 0;
                    break;
                };

            }

            return 1;          

        }
        
        NULLDecoder::NULLDecoder( bool &recording ):recording(recording)
        {
            
            SInfo << "NULLDecoder()";
            
            resetTimer();

            qframe = new stFrame();
            

//            basicframe.media_type  = AVMEDIA_TYPE_VIDEO;
//            basicframe.codec_id   = AV_CODEC_ID_H264;
//            basicframe.stream_index  = 0;
            
            std::string tmp = Settings::configuration.storage  ;
                  

            mf.load(tmp + "manifest.js");

            if( mf.root.is_null() )
            {
               mf.root = json::object();
            }
                    
     
        }

        NULLDecoder::~NULLDecoder() {
            
            SInfo << "~NULLDecoder()";
            qframe->clear();
            SInfo << "~NULLDecoder()1";
            delete qframe;
            SInfo << "~NULLDecoder() over";
        }

        
        void NULLDecoder::WriteTofile( unsigned char *buf , int size , int& recframeCount)
        {
          
            char filePath[128];
             
            sprintf(filePath, "%s/frame-%.3d.h264", m_pathDate.c_str() , ++recframeCount);

                            
            in_file = fopen(filePath,"wb");
            if(!in_file){
                   SError << "can't open file! " <<  in_file;
            }

            fwrite(buf, 1, size, in_file);
            
            fclose(in_file);
            
        }
 

        void NULLDecoder::runNULLEnc(unsigned char *buffer, int size,  int & recframeCount , LiveConnectionContext  *ctx , std::time_t &datetm) 
        {

            int cfg{0}; 
            bool idr = false;

            
          //  SInfo << " full buffer size " << size;
            
            bool slice = false;
           
                
            int payload_type; 
            int frameType;
             
            while ( parse_nal(buffer, size,  payload_type,   frameType ))
            {
                

   
                if (payload_type ==  7 && cfg < 2) {

                    m_sps.resize(size);

                    memcpy(&m_sps[0], buffer, size);

//                           unsigned char *tmp = m_sps.data() + 4;
                    qframe->pushsps(m_sps);
                    unsigned num_units_in_tick, time_scale;
                      

                    H264Framer obj;
                    obj.analyze_seq_parameter_set_data(buffer+4, size - 4 , num_units_in_tick, time_scale);

                    STrace  <<  "Got SPS fps "  << obj.fps << " width "  << obj.width  <<  " height " << obj.height ;

                    //absl::optional<webrtc::SpsParser::SpsState> sps_;
                    //static_cast<bool> (sps_ = webrtc::SpsParser::ParseSps(&buffer[index.start_offset + index.payload_start_offset + webrtc::H264::kNaluTypeSize], index.payload_size - webrtc::H264::kNaluTypeSize));

                    //width = sps_->width;
                   // height = sps_->height;

                    width = obj.width;
                    height = obj.height;

                    fps = obj.fps ? obj.fps:30;
                    vdelay = uint64_t(1000000) / uint64_t(fps); // default

                    
//                    basicframe.width = width;
//                    basicframe.height = height;
//                    basicframe.fps = fps;
//                    
//                    
//                    basicframe.payload.resize(m_sps.size());
//                    basicframe.payload.insert(basicframe.payload.begin(), m_sps.begin(), m_sps.end());
//                    basicframe.fillPars();
//                    cb_mp4(&basicframe, false); 
                      cfg++;

                      break;
                    
                   
                } else if (payload_type == 8 && cfg < 2) {

                    m_pps.resize(size);
                    // m_pps = webrtc::EncodedImageBuffer::Create((uint8_t*) & buffer[index.start_offset], index.payload_size + index.payload_start_offset - index.start_offset);
                    memcpy(&m_pps[0],  buffer, size);
                    qframe->pushpps(m_pps);
                    
//                    basicframe.payload.resize(m_pps.size());
//                    basicframe.payload.insert(basicframe.payload.begin(), m_pps.begin(), m_pps.end());
//                    basicframe.fillPars(); 
//                    cb_mp4(&basicframe, false); 
                    
                            
                      cfg++;
                     
                      break;
                } else if (payload_type == 5) {
                    idr = true;
                   //  SInfo << " idr:" << idr << " recording " << recording << " this " << this <<  " cfg:" << cfg << "  sps " << m_sps.size() << " pps  " << m_pps.size() << " vframecount " << vframecount << " width " << width << " height  " << height << " fps " << fps;
                    
//                    basicframe.payload.resize(index.payload_size + index.payload_start_offset - index.start_offset);
//                    basicframe.payload.insert(basicframe.payload.begin(),  &buffer[index.start_offset],   &buffer[index.start_offset] + index.payload_size + index.payload_start_offset - index.start_offset);
//                    basicframe.fillPars();
//                    cb_mp4(&basicframe, true); 
                    
                } else if (payload_type == 1) {
                   
                    
                   // SInfo << "slice";

                    int x = 1;
//                    basicframe.payload.resize(index.payload_size + index.payload_start_offset - index.start_offset);
//                    basicframe.payload.insert(basicframe.payload.begin(),  &buffer[index.start_offset],   &buffer[index.start_offset] + index.payload_size + index.payload_start_offset - index.start_offset);
//                    basicframe.fillPars();
//                    cb_mp4(&basicframe, false); 
          
                }
                else
                {
      
                   // size =  size - sizeof_nal;
                    break;
                }
       

                    
                    
                 
                    
            if( idr  )
            {
               // SInfo << "  AV_PICTURE_TYPE_I " ;
               qframe->clear();
            }
            
            
//             if( width == 0 || height == 0)
//            {
//              exit(0);
//            } 
             
            if(m_sps.size() && m_pps.size() ) 
            {
               // SInfo << " index.start_offset " << index.start_offset  << " size " << index.payload_size + index.payload_start_offset - index.start_offset;
                
                if(recording)
                {
                    int x = 0;
                }
                
                Store *store = new Store(buffer, size, width, height, vframecount, idr);
                qframe->push(store);
                cb_frame(qframe); 
            }
            
            if(  recframeCount > 1  )
            {
                if( recframeCount > Settings::configuration.recordsize + 500)
                {
                     recframeCount = -1;  
                     // SInfo << " recframeCount " << recframeCount;
                    
                }
                else if( recframeCount >=  Settings::configuration.recordsize)
                {
                    
                   if(recframeCount++ ==  Settings::configuration.recordsize)
                   {
                        //  SInfo << " Saved " << recframeCount;
                        mf.save();
                       // recordingTime(ctx);
                   }
                }
                else if(idr )
                {
                    WriteTofile(&buffer[0], size, recframeCount);
                }
                else if ( recframeCount > 2)
                {
                    WriteTofile(&buffer[0], size, recframeCount);
                }
            }
            else if( Settings::configuration.recording && !recframeCount )
            {
                
                struct std::tm* tms = std::localtime(&datetm);

                char datetime[100] = {'\0'}; //"%Y-%m-%d-%H-%M-%S"
                int len = std::strftime(datetime, sizeof (datetime), "%Y-%m-%d-%H-%M-%S", tms);
                
                char dates[100] = {'\0'}; //"%Y-%m-%d-%H-%M-%S"
                len = std::strftime(dates, sizeof (dates), "%Y-%m-%d", tms);
                
                
                if( m_date != dates) // do cleaning
                {
                    m_date = dates;
    
                  //  std::time_t lastweek =  datetm - 604800;  // ;// 7*24*60*60;
                   // struct std::tm* tms = std::localtime(&lastweek);

                    //char lwdates[100] = {'\0'}; //"%Y-%m-%d-%H-%M-%S"
                    //len = std::strftime(lwdates, sizeof (lwdates), "%Y-%m-%d", tms);
           
                    for (json::iterator itDate = mf.root.begin(); itDate != mf.root.end(); ) 
                    {
                        struct tm timeinfo= {};
                        std::string lwdates = itDate.key() ;
                        strptime(lwdates.c_str(), "%Y-%m-%d", &timeinfo);
                        time_t lastweek = mktime(&timeinfo);

                        double diff = difftime(datetm,  lastweek  );

                        //std::cout << "lwdates: " << lwdates << std::endl;
                        // 
                        //std::cout << "Year: " << 1900 + timeinfo.tm_year << std::endl;
                        //std::cout << "Month: " << 1 + timeinfo.tm_mon << std::endl;
                        //std::cout << "day: " << timeinfo.tm_mday << std::endl;
                         
                       //std::cout << "diff " << diff/(24*60*60) <<  " lastweek " << lastweek  <<  std::endl;

                         
                        if( diff > double(604800) )
                        {
                            for (json::iterator itDT =  itDate.value().begin(); itDT != itDate.value().end(); ++itDT) 
                            {
                                SInfo <<  "delete " << *itDT ;
                                std::string tmp = "rm -rf " + Settings::configuration.storage +  itDT->get<std::string>() ;
                                
                                system(tmp.c_str());
                                
                                //remove_directory(tmp.c_str());
                            }

                           
                            itDate = mf.root.erase(itDate);
                        }
                        else
                          ++itDate  ;
                    }
                    
                }
                
                if( m_dateTime != datetime)
                {   m_dateTime = datetime;
                    m_pathDate = Settings::configuration.storage + m_dateTime  ;
                    if (!base::fs::exists(m_pathDate ))
                    {
                       SInfo <<  m_pathDate;
                       mkdir(m_pathDate.c_str(),0777);
                    }
                    
                    if( mf.root.find(dates) ==   mf.root.end())
                    {
                        mf.root[dates] = json::array();
                    }
                    mf.root[dates].push_back(m_dateTime);
                 
                    WriteTofile(&m_sps[0], m_sps.size(), recframeCount);
                    WriteTofile(&m_pps[0], m_pps.size(), recframeCount);
                    if(idr )
                    {
                        WriteTofile(&buffer[0], size, recframeCount);
                    }
                    
                }
            }
            
            
            delayFrame();
             
            break;
             
            
            }// end for
            
            
            

           

            return;
        }
        
        
        
        void NULLDecoder::delayFrame( )
        {
            
           int speed =1; 
           
//           if( trackInfo.speed != 0)
//           {
//               speed = trackInfo.speed;
//           }


//           if(speed > 1 && speed < 4 )
//           {
//              vdelay =  vdelay/speed;
//           }
//           else if( speed <  -1)
//           {
//               vdelay =  vdelay*(-1*speed);
//           }
//        
//             

            ++vframecount;


            uint64_t deltamicro = CurrentTime_microseconds() - startStreaming;

        //    if(  deltamicro >=  vframecount * vdelay)
        //    {
        //         std::this_thread::sleep_for(std::chrono::microseconds(1004));
        //        SError <<  "deltamicro > vframecount * vdelay " << deltamicro - vframecount * vdelay;
        //    }
//#ifdef LOCALTEST
            while(deltamicro < vframecount * vdelay)
            {
                std::this_thread::sleep_for(std::chrono::microseconds(20000));
                deltamicro = CurrentTime_microseconds() - startStreaming;
            }
//#endif
        }
        
        void NULLDecoder::resetTimer()
        {
           startStreaming = CurrentTime_microseconds();
           vframecount =0;
        }
        
        /*
        void NULLDecoder::recordingTime(LiveConnectionContext  *ctx )
        {
            cnfg::Configuration identity;
            identity.load(Settings::configuration.storage + "manifest.js");

            if(identity.loaded())
            {
                json m;
                m["messageType"] = "RECORDING";
                m["messagePayload"] =  identity.root;
                ctx->signaler->postAppMessage( m);

            }
        }
        */    


    }// ns web_rtc
}//ns base
