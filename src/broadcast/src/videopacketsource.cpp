

#include "webrtc/videopacketsource.h"
//#include "VideoEncoder.h"

#include "webrtc/peermanager.h"

#include "webrtc/rawVideoFrame.h"

//#include "muxer.h"
#include "tools.h"

#include "Settings.h"

using std::endl;

namespace base {
    
   
namespace web_rtc {
    


VideoPacketSource::VideoPacketSource( const char *name, LiveConnectionContext  *ctx, st_track *trackInfo, bool recording, web_rtc::FrameFilter *next):ctx(ctx),trackInfo(trackInfo), web_rtc::FrameFilter(name, next), recording(recording)
    , _rotation(webrtc::kVideoRotation_0)
    , _timestampOffset(0)

//    , _source(nullptr)
{
   
    SInfo << " VideoPacketSource " << this;
    
    #if BYPASSGAME
    StartParser();
    #else
//     StartParser(); 
//     StartLive(); 
    #endif
    
    

    //ctx = new web_rtc::LiveConnectionContext(LiveConnectionType::rtsp, "address", 1, cam, cam, Settings::configuration.tcpRtsp, this ) ; // Request livethread to write into filter info
    
   
    liveThread = new LiveThread("live", ctx,  trackInfo, recording, false);
    if(recording)
    {
        this->ctx->recFrame= this;
        //this->ctx->liveFrame = nullptr; 
    }
    else
    {
        this->ctx->liveThread = liveThread;
        this->ctx->liveFrame = this;
    }    
   
    liveThread->start();
   
  
}












VideoPacketSource::~VideoPacketSource()
{
    
    SInfo <<  "~VideoPacketSource() ";
    if(liveThread)
    liveThread->stop();
    
     delete liveThread;
    liveThread =nullptr;
                   
           
//    if(ctx->fragmp4_filter)
//    {
//
//        delete ctx->fragmp4_filter;
//        ctx->fragmp4_filter = nullptr;
//    }
////            
//    if(ctx->fragmp4_muxer)
//    delete ctx->fragmp4_muxer;
//    ctx->fragmp4_muxer = nullptr;
//           
           
            

    
     SInfo <<  "delete nullDecoder";

    if(nullDecoder)
    {
        delete nullDecoder;
        nullDecoder = nullptr;
    }
    
    
     SInfo <<  "~VideoPacketSource() over ";


}

 


void VideoPacketSource::oncommand( std::string & cmd , int first,  int second)
{
    if(cmd == "apply")
    {
        reset_sws_cts= true;  // arvind
///        ffparser->reverse();  
    }
    else if( cmd == "mute")
    {
       // ffparser->paused(first);  // arvind
    }

    
    if(nullDecoder)
    nullDecoder->resetTimer();
    
    
}



void VideoPacketSource::run(web_rtc::Frame *frame)
{
//    if(trackInfo.ai &&  frame->type() ==  "txt") // this code is for video analysis
//    {
//        web_rtc::TextFrame *txt_frame = static_cast<web_rtc::TextFrame *> (frame);
//        
//        mutextxt.lock();
//        
//        metaData = txt_frame->txt;
//                
//        mutextxt.unlock();
//        
//        return;
//    }
    
    #if LOCALTEST
    if(cam.ai)
    {
         mutextxt.lock();
    //metaData = "<Frame Time='1678121202.56409'><Objects InputStream='2'><Object><BoundingBox X0='0.681' Y0='0.019' X1='1.000' Y1='0.816'/><Classification Label='person' Confidence='0.629999995'/><Attributes><Attribute Name='LowerClothingColor' Value='Black' Confidence='0.340000004'/><Attribute Name='UpperClothingColor' Value='Black' Confidence='0.230000004'/></Attributes><Tracking Id='20'/></Object></Objects></Frame>";

         metaData =  "<Frame Time='1678354650.48074'><Objects InputStream='2'><Object><BoundingBox X0='0.714' Y0='0.085' X1='0.998' Y1='0.960'/><Classification Label='person' Confidence='0.819999993'/><Tracking Id='1539'/></Object><Object><BoundingBox X0='0.411' Y0='0.208' X1='0.729' Y1='0.988'/><Classification Label='person' Confidence='0.74000001'/><Tracking Id='1542'/></Object></Objects></Frame>";
    //metaData =  "<Frame Time='1678441838.53494'><Alarms><ObjectAlert><Rule Name='AI' Type='Detection'/><BoundingBox X0='0.496' Y0='0.084' X1='0.806' Y1='0.767'/><HitTime Time='1678441838.53223'/></ObjectAlert></Alarms><Objects InputStream='2'><Object><BoundingBox X0='0.496' Y0='0.084' X1='0.806' Y1='0.767'/><Classification Label='person' Confidence='0.400000006'/><Attributes><Attribute Name='LowerClothingColor' Value='Blue' Confidence='0.460000008'/><Attribute Name='UpperClothingColor' Value='Blue' Confidence='0.819999993'/></Attributes><Tracking Id='26'/></Object></Objects></Frame>";

         mutextxt.unlock();
    }
    #endif
    
    web_rtc::BasicFrame *basic_frame = static_cast<web_rtc::BasicFrame *> (frame);
 

    uint8_t* data = NULL;
    int size = 0;

 //   std::copy(basic_frame->data, basic_frame->data + basic_frame->sz, std::back_inserter(buffer));


    //while (buffer.size() > 0)
    {
       // int len = av_parser_parse2(parser, cdc_ctx, &data, &size, &buffer[0], buffer.size(), 0, 0, AV_NOPTS_VALUE);

       // if (size == 0 && len >= 0) {
         //   return;
      //  }

  // Iterate through the map and print the elements
       // if (len )
        {   
          //  if(trackInfo.encType == EN_NATIVE)
            {
                ///runNULLEnc(frame);


                if(!nullDecoder)
                {
                    nullDecoder = new NULLDecoder( recording );

                    nullDecoder->cb_frame = [&](stFrame* frame) {

                   
//                    mutextxt.lock();
//        
//                    txtCpy = metaData;
//		      metaData.clear();
//                    mutextxt.unlock();
//        
                                
                    rtc::scoped_refptr<NULLEncBuffer> Buffer = new rtc::RefCountedObject<NULLEncBuffer>( frame ,nullDecoder->width, nullDecoder->height, nullDecoder->fps , recording);

                    int64_t TimestampUs = rtc::TimeMicros();
                      
                    webrtc::VideoFrame Frame = webrtc::VideoFrame::Builder().
                             set_video_frame_buffer(Buffer).
                             set_rotation(webrtc::kVideoRotation_0).
                             set_timestamp_us(TimestampUs).
                             build();

                     // SDebug << "ideoPacketSource::OnFrame";

                     OnFrame(Frame); //arvind

                      // std::this_thread::sleep_for(std::chrono::microseconds(40000));



                    };
         
               }




              // web_rtc::BasicFrame *basic_frame = static_cast<web_rtc::BasicFrame *>(frame);
                
               
                    
//               
//                if(recording &&  ++recording > Settings::configuration.Mp4Size_Key*25)
//                {
//                     recording= 0;
//                }
                 
                if( !recording && liveThread->t31rgba->record )
                {
                    ((LiveThread*)liveThread)->t31rgba->record = false;
                    
                    if(frameCount < 0)
                    frameCount= 0;
                }
                

               nullDecoder->runNULLEnc( (uint8_t*) basic_frame->data,  basic_frame->sz,  frameCount , ctx, liveThread->t31rgba->m_date );

               // runNative(frame);
               // return;
            }





       
            
        
           
             
        } // len
       // if(len)
       // buffer.erase(buffer.begin(), buffer.begin() + len);
        
    }// while buffer
    
   
    
    return ;
}





void VideoPacketSource::myAddRef(  std::string peerid)  {
   
    mutexVideoSoure.lock();
    
    setPeerid.insert(peerid);
    
    mutexVideoSoure.unlock();
 // const int count =   rtc::AtomicOps::Increment(&ref_count_);  //arvind
 // SInfo << "VideoPacketSource::AddRef()" << count;
  
}

rtc::RefCountReleaseStatus VideoPacketSource::myRelease(  std::string peerid )  {
    
    std::set< std::string> ::iterator itr;
    int count =1;
    
    mutexVideoSoure.lock();
    itr = setPeerid.find(peerid);
    
    if( itr != setPeerid.end())
    {
        setPeerid.erase(itr);
    }
    
    count = setPeerid.size();
    mutexVideoSoure.unlock();
    
    
  
  
    SInfo << "VideoPacketSource::Release()" << count;
    
   if (count == 0) {
     
     return rtc::RefCountReleaseStatus::kDroppedLastRef;
   }
  return rtc::RefCountReleaseStatus::kOtherRefsRemained;
}




 void VideoPacketSource::reset(  std::set< std::string> & peeerids )  {
    
    std::set< std::string> tmp;
    mutexVideoSoure.lock();
   
    peeerids =    setPeerid;
    
    setPeerid.clear();
    
    mutexVideoSoure.unlock();
    
}



// 
////////////
webrtc::MediaSourceInterface::SourceState VideoPacketSource::state() const {
  return kLive;
}

bool VideoPacketSource::remote() const {
  return false;
}

bool VideoPacketSource::is_screencast() const {
  return false;
}

absl::optional<bool> VideoPacketSource::needs_denoising() const {
  return false;
}

} } // namespace web_rtc





/*
#define h24SoftEnc 1
void VideoPacketSource::run(fmp4::Frame *frame)
{
    fmp4::BasicFrame *basic_frame = static_cast<fmp4::BasicFrame *>(frame);
  
    
#if h24SoftEnc
    
    uint8_t* data = NULL;
    int size = 0;

    std::copy(basic_frame->payload.data(), basic_frame->payload.data() +  basic_frame->payload.size(), std::back_inserter(buffer));

       // int len = av_parser_parse2(parser, cdc_ctx, &data, &size, 
        //                           basic_frame->payload.data(),  basic_frame->payload.size(), 0, 0, AV_NOPTS_VALUE);

    int len = av_parser_parse2(parser, cdc_ctx, &data, &size,  &buffer[0], buffer.size(), 0, 0, AV_NOPTS_VALUE);

    if(size == 0 && len >= 0) {
      return ;
    }

    if(len) 
    {
        decodeFrame(&buffer[0], size);
        buffer.erase(buffer.begin(), buffer.begin() + len);
        return ;
    }
#else
    

            int64_t TimestampUs = rtc::TimeMicros();
          
            unsigned char* buffer = basic_frame->payload.data();
            int frameSize =  basic_frame->payload.size();

             bool idr = false;

             bool slice = false;

             int cfg = 0;
              std::vector<webrtc::H264::NaluIndex> naluIndexes = webrtc::H264::FindNaluIndices((uint8_t*) buffer, frameSize);

             for (webrtc::H264::NaluIndex index : naluIndexes) {
                 webrtc::H264::NaluType nalu_type = webrtc::H264::ParseNaluType(buffer[index.payload_start_offset]);
                // SInfo << __FUNCTION__ << " nalu:" << nalu_type << " payload_start_offset:" << index.payload_start_offset << " start_offset:" << index.start_offset << " size:" << index.payload_size;
                 if (nalu_type == webrtc::H264::NaluType::kSps) {

                     m_sps.resize(index.payload_size + index.payload_start_offset - index.start_offset);

                     memcpy( &m_sps[0],   &buffer[index.start_offset], index.payload_size + index.payload_start_offset - index.start_offset);

                     unsigned char *tmp = m_sps.data() + 4;



                     unsigned num_units_in_tick, time_scale;


                     H264Framer obj;  
                     obj.analyze_seq_parameter_set_data(&buffer[index.start_offset + index.payload_start_offset], index.payload_size, num_units_in_tick, time_scale);


                     //SInfo <<  " Got SPS fps "  << obj.fps << " width "  << obj.width  <<  " height " << obj.height ;

                     absl::optional<webrtc::SpsParser::SpsState> sps_;
                     static_cast<bool>(sps_ = webrtc::SpsParser::ParseSps(&buffer[index.start_offset + index.payload_start_offset +  webrtc::H264::kNaluTypeSize], index.payload_size -  webrtc::H264::kNaluTypeSize));

                     width =  sps_->width;
                     height = sps_->height;

                     width =  obj.width;
                     height = obj.height;

                     fps = obj.fps;

                     cfg++;
                 } else if (nalu_type == webrtc::H264::NaluType::kPps) {

                      m_pps.resize(index.payload_size + index.payload_start_offset - index.start_offset);
                    // m_pps = webrtc::EncodedImageBuffer::Create((uint8_t*) & buffer[index.start_offset], index.payload_size + index.payload_start_offset - index.start_offset);
                      memcpy( &m_pps[0],   &buffer[index.start_offset], index.payload_size + index.payload_start_offset - index.start_offset);
                     cfg++;
                 } else if (nalu_type == webrtc::H264::NaluType::kIdr) {
                     idr = true;
                 }

                 else if (nalu_type == webrtc::H264::NaluType::kSlice) {
                     slice = true;
                 }
             }
            // SInfo << __FUNCTION__ << " idr:" << idr << " cfg:" << cfg << " " << m_sps.size() << " " << m_pps.size() << " " << frameSize;

            //                rtc::scoped_refptr<webrtc::EncodedImageBufferInterface> encodedData = webrtc::EncodedImageBuffer::Create((uint8_t*) buffer, frameSize);
            //                delete [] buffer;
             // add last SPS/PPS if not present before an IDR
             if (idr &&   (m_sps.size() != 0) && (m_pps.size() != 0)) {
                 //char * newBuffer = new char[frameSize + m_sps.size() + m_pps.size()];
                // memcpy(newBuffer, &m_sps[0], m_sps.size());
                // memcpy(newBuffer + m_sps.size(), &m_pps[0], m_pps.size());
                 //memcpy(newBuffer + m_sps.size() + m_pps.size(), buffer, frameSize);
                // encodedData = webrtc::EncodedImageBuffer::Create((uint8_t*) newBuffer, encodedData->size() + m_sps->size() + m_pps->size());
                // delete [] newBuffer;
                // if(!frameNo)
                 {
                     //std::cout << " payload size " <<   basic_frame->payload.size()  << std::endl << std::flush;

                     basic_frame->payload.insert(  basic_frame->payload.begin() , m_pps.begin() , m_pps.end() );

                   //  std::cout << " payload size " <<   basic_frame->payload.size()  << std::endl << std::flush;

                     basic_frame->payload.insert(  basic_frame->payload.begin() , m_sps.begin() , m_sps.end() );

                    frameNo++;

                 }


             }
             else if(slice && frameNo)
             {
                 frameNo++;
             }
             else
             {
                 return ;
                 int y= 0;
             }

            //



        if(!frameNo)
        return ;         

       // SInfo <<  " VideoPacketSource frame no"  <<   frameNo  <<  " frame size "  << basic_frame->payload.size() << " idr " << idr << " width " << width  << " height " << height ;

        rtc::scoped_refptr<FRawFrameBuffer> Buffer = new rtc::RefCountedObject<FRawFrameBuffer>( basic_frame->payload ,width, height, frameNo, idr, fps);
                      


         webrtc::VideoFrame Frame = webrtc::VideoFrame::Builder().
                 set_video_frame_buffer(Buffer).
                 set_rotation(webrtc::kVideoRotation_0).
                 set_timestamp_us(TimestampUs).
                 build();

         // SDebug << "ideoPacketSource::OnFrame";

         OnFrame(Frame); //arvind

    
    return ;
    
    #endif
}
*/
