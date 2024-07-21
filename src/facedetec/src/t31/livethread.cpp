#include "livethread.h"
#include "base/platform.h"
// #include "base/base64.hpp"

#include "base/logger.h"
#include <sys/reboot.h>

        
#include "net/netInterface.h"        
#include "http/url.h"
#include "base/filesystem.h"
#include "http/HttpClient.h"
#include "http/HttpsClient.h"

#include <imp/imp_log.h>
#include <imp/imp_common.h>
#include <imp/imp_system.h>
#include <imp/imp_framesource.h>
#include <imp/imp_encoder.h>
#include "sample-common.h"


#include "base/platform.h"
#include "base/rgba_bitmap.h"
#include "base/base64.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "base/stb_image.h"

#if(DUMPFILE)
#include "Settings.h"
struct Settings::Configuration Settings::configuration;

#else
#include "webrtc/signaler.h"
#include "Settings.h"
#endif

#include <xailient-fi/sdk_json_interface.h>


#include "base/logger.h"
using namespace base;
using namespace base::net;
using namespace base::cnfg;



extern struct chn_conf chn[];

#define LOW_BITSTREAM
//#define SHOW_FRM_BITRATE
#ifdef SHOW_FRM_BITRATE
#define FRM_BIT_RATE_TIME 2
#define STREAM_TYPE_NUM 3
static int frmrate_sp[STREAM_TYPE_NUM] = { 0 };
static int statime_sp[STREAM_TYPE_NUM] = { 0 };
static int bitrate_sp[STREAM_TYPE_NUM] = { 0 };
#endif

#include <zbar.h>
using namespace zbar;




std::atomic<int>  HDVideo {2};

                                                
namespace base {
namespace web_rtc {
    




//static int notfound = 0, exit_code = 0;
//static int num_images = 0, num_symbols = 0;
static int xmllvl  = 0;
//static int polygon = 0;
//static int oneshot = 0;
//static int binary  = 0;

//char *xmlbuf     = NULL;
unsigned xmlbuflen = 0;

static zbar_processor_t *processor = NULL;



void saveFile( const char* filename,  std::string str)
{
   FILE *fp=NULL;
   
   fp = fopen( filename , "wb" );
   if(fp)
   fwrite(str.c_str() , 1 ,str.size() , fp );

   fclose(fp);
    
}

/*
// blue
echo 0 > /sys/class/gpio/gpio39/value  
echo 1 > /sys/class/gpio/gpio38/value

// red
echo 1 > /sys/class/gpio/gpio39/value  
echo 0 > /sys/class/gpio/gpio38/value



// no color 
echo 1 > /sys/class/gpio/gpio39/value  
echo 1 > /sys/class/gpio/gpio38/value

// orange
echo 0 > /sys/class/gpio/gpio39/value  
echo 0 > /sys/class/gpio/gpio38/value
*/

void blueLed( )
{
   int f39 = open("/sys/class/gpio/gpio39/value", O_RDWR);
   
    if( f39 >= 0 )
    {
      write(f39, "0", 1);
      close(f39);
    }
   
   
    int f38 = open("/sys/class/gpio/gpio38/value", O_RDWR);
    if( f38 >= 0 )
    {
      write(f38, "1", 1);
      close(f38);
    }
   
    
}


void noLed( )
{
   int f39 = open("/sys/class/gpio/gpio39/value", O_RDWR);
   
    if( f39 >= 0 )
    {
      write(f39, "1", 1);
      close(f39);
    }
   
   
    int f38 = open("/sys/class/gpio/gpio38/value", O_RDWR);
    if( f38 >= 0 )
    {
      write(f38, "1", 1);
      close(f38);
    }
   
    
}

void redLed( )
{
   int f39 = open("/sys/class/gpio/gpio39/value", O_RDWR);
   
    if( f39 >= 0 )
    {
      write(f39, "1", 1);
      close(f39);
    }
   
   
    int f38 = open("/sys/class/gpio/gpio38/value", O_RDWR);
    if( f38 >= 0 )
    {
      write(f38, "0", 1);
      close(f38);
    }
   
    
}

        

int readFile(const char* filename,  char *buf, int len)
{
   FILE *fp=NULL;
   
   fp = fopen( filename , "rb" );
   
   if(fp)
   fread(buf , 1 ,len , fp );
   else 
       return -1;

   fclose(fp);
   
   return 0;
    
}


static int scan_image(unsigned char *blob, int width, int height)
{

    int found        = 0;
    unsigned seq = 0;

  //if (!MagickSetImageIndex(images, seq) && dump_error(images))
   //   return (-1);

  zbar_image_t *zimage = zbar_image_create();
  assert(zimage);
  zbar_image_set_format(zimage, zbar_fourcc('R', 'G', 'B', '3'));


  zbar_image_set_size(zimage, width, height);

  // extract grayscale image pixels
  // FIXME color!! ...preserve most color w/422P
  // (but only if it's a color image)
  size_t bloblen      = width * height*3;
        
  //unsigned char *blob = malloc(bloblen);
  zbar_image_set_data(zimage, blob, bloblen, zbar_image_free_data);

  //if (!MagickGetImagePixels(images, 0, 0, width, height, "RGB", CharPixel,  blob))
            
       
        
         //saveFile(blob, bloblen );

  if (xmllvl == 1) {
      xmllvl++;
     // printf("<source href='%s'>\n", filename);
  }

  int retResult =0 ;
  zbar_process_image(processor, zimage, &retResult);
  
  noLed();
  
  if(retResult < 0)
      redLed();

  // output result data
  const zbar_symbol_t *sym = zbar_image_first_symbol(zimage);
  for (; sym; sym = zbar_symbol_next(sym)) {
      zbar_symbol_type_t typ = zbar_symbol_get_type(sym);
      unsigned len     = zbar_symbol_get_data_length(sym);
      if (typ == ZBAR_PARTIAL)
    continue;
      else if (xmllvl <= 0)
    {
      if (!xmllvl)
        printf("%s:", zbar_get_symbol_name(typ));

      if (len &&
          fwrite(zbar_symbol_get_data(sym), len, 1, stdout) != 1) 
      {
          return (-1);
      }
    } 
//            else
//            {
//    if (xmllvl < 3) {
//        xmllvl++;
//        printf("<index num='%u'>\n", seq);
//    }
//    zbar_symbol_xml(sym, &xmlbuf, &xmlbuflen);
//    if (fwrite(xmlbuf, xmlbuflen, 1, stdout) != 1) {
//        exit_code = 1;
//        return (-1);
//    }
//      }


     json root;
     try
     { 
        root = json::parse(zbar_symbol_get_data(sym));

        if(  root.find("s") != root.end()) 
          saveFile("/configs/.wifissid", root["s"].get<std::string>());
        if( root.find("p") != root.end()) 
         saveFile("/configs/.wifipasswd", root["p"].get<std::string>() );


        if( root.find("i") != root.end()) 
        {     
            json m;

            m["dtlsCertificateFile"] = "/mnt/key/certificate.crt";
            m["dtlsPrivateKeyFile"] =  "/mnt/key/private_key.pem";
            m["storage"]= "/mnt/pvi-storage/";
            m["qrcode"] = root["i"].get<std::string>();
            m["server"] =  "ipcamera.adapptonline.com";
            m["port"]  =  443;
            m["recording"] = true;
            m["cloud"] =  false;
            m["facedetect"]  =  true;
            m["motionevent"] =  true;
            m["cam_reconnect"] =  0;
            m["authtimeout"] =  3600;
            m["Mp4Size_Key"] =  40;
            m["SegSize_key"] =  5;
            m["logLevel"] = "info";

            base::cnfg::saveFile("/mnt/config.js", m );
        
            blueLed();

            reboot(RB_AUTOBOOT);
        }
        
      }
      catch(...)
      {
          SError << "wrong json format";
      }



    found++;
     // num_symbols++;

     
      printf("\n");
     
  }

  fflush(stdout);

  zbar_image_destroy(zimage);

//  num_images++;
//  if (zbar_processor_is_visible(processor)) {
//      int rc = zbar_processor_user_wait(processor, -1);
//      if (rc < 0 || rc == 'q' || rc == 'Q')
//    exit_code = 3;
//  }

    if (xmllvl > 1) {
  xmllvl--;
  printf("</source>\n");
    }



    //DestroyMagickWand(images);
    return found;
}


/*
int qrcode_scan()
{

  int width  = 100;
  int height =100;

 
  size_t bloblen      = width * height*3;
  unsigned char *blob = malloc(bloblen);
      
  if(readFile("./test.rgb", blob, bloblen ) )                        
   return (-1);
        
        
  scan_image( blob, width, height);
      
}
*/

int qrcode_init()
{
    // option pre-scan

    int i=0;
    int  display = 0;


    processor = zbar_processor_create(0);
    assert(processor);



    if (zbar_processor_init(processor, NULL, display)) {
        zbar_processor_error_spew(processor, 0);
       return ( -11);
    }

       
    zbar_increase_verbosity();
  

    return (0);
}


int qrcode_exit()
{

 zbar_processor_destroy(processor);
}





void RestAPI(std::string method, std::string ip, std::string uri,json &m)
{
    Application app;

    std::string sendMe = m.dump();
    
    //ClientConnecton *conn = new HttpsClient( "https", "ipcamera.adapptonline.com", 8080, uri);
    ClientConnecton *conn = new HttpsClient("https", ip, 443, uri);
    //Client *conn = new Client("http://zlib.net/index.html");
    conn->fnComplete = [&](const Response & response) {
        std::string reason = response.getReason();
        StatusCode statuscode = response.getStatus();
     //   std::string body = conn->readStream() ? conn->readStream()->str() : "";
      //  STrace << "Post API reponse" << "Reason: " << reason << " Response: " << body;
    };

    conn->fnConnect = [&, sendMe](HttpBase * con) {
        
        SInfo << sendMe.length();
        
        //SDebug << " db envent "  <<  sendMe;

        con->send( sendMe.c_str(), sendMe.length(), false);
        
    };

    conn->fnPayload = [&](HttpBase * con, const char* data, size_t sz) {

        std::cout << "client->fnPayload " << data << std::endl << std::flush;
    };

    conn->fnUpdateProgess = [&](const std::string str) {
        std::cout << "final test " << str << std::endl << std::flush;
    };

    conn->_request.setMethod(method);
    conn->_request.setKeepAlive(false);
    
    conn->_request.setContentLength(sendMe.size());
    conn->_request.setContentType("application/json");
    
    
    conn->setReadStream(new std::stringstream);
    conn->send();
    
    
    app.run();
       
    
}

void T31RGBA::run() {
    


    SInfo << "T31RGBA::run()";
    int ret = 0;
    int i = 0;
   
    IMPFrameInfo *frame;

    if(!QRCode)
    {

      
      XA_addGallery();
    
    

     #if(DUMPFILE)

      FILE *fp;

      fp = fopen("/tmp/snap.rgba", "wb");
      if(fp == NULL) {
          SError<<"file open error ";
          return ;
      }

      #endif

    
    
      Timestamp ts;
      //Timestamp::TimeVal time = ts.epochMicroseconds();
    //  int milli = int(time % 1000000) / 1000;

      std::time_t time1 = ts.epochTime();
      struct std::tm* tms = std::localtime(&time1);

      char date[100] = {'\0'}; //"%Y-%m-%d-%H-%M-%S"
      int len = std::strftime(date, sizeof (date), "%Y-%m-%d-%H-%M-%S", tms);

      json m;

      m["messageType"] = "INIT";
      m["messagePayload"] =  "INIT";
      m["camid"] = ctx->cam;
      m["ts"] =  date;
       RestAPI("POST",  "backend.adapptonline.com", "/eventsToCloudX", m);  
    }


   


    while (!stopped()) {
        /* Snap RGBA */
        ret = IMP_FrameSource_SetFrameDepth(3, 1);
        if (ret < 0) {
            SError<<"IMP_FrameSource_SetFrameDepth failed";
            return;
        }

        ret = IMP_FrameSource_GetFrame(3, &frame);
        if (ret < 0) {
            SError<<"IMP_FrameSource_GetFrame failed";
            return ;
        }



        STrace << "Frame size "  <<  frame->size << " width"  << frame->width << " height "  <<   frame->height <<  " format "  << frame->pixfmt;
        
       
        if ( ready_flag ) 
        {
            size_t  p_output_size1 = 0;

            unsigned char *  rgbBuf = rgba_to_rgb_brg( (const unsigned char*)frame->virAddr , frame->size,  bitmap_buffer_format_RGB , 0, frame->width , frame->height , &p_output_size1 );


           // for( int x =0; x < 40 ; ++x)
            {

              if(!QRCode)
              {
                 XAProcess( rgbBuf, frame->width , frame->height) ;
                free(rgbBuf) ;
               }
              else
               scan_image( rgbBuf, frame->width , frame->height);




               // base::sleep(100);
             }

             

            // fwrite((void *)frame->virAddr, frame->size, 1, fp);
            // fclose(fp);

            
            // unsigned char *  bgrBuf = rgba_to_rgb_brg( (const unsigned char*) frame->virAddr , frame->size, bitmap_buffer_format_BGR , 0, frame->width , frame->height , &p_output_size1 );
            // write_bmp(bgrBuf, frame->width , frame->height, "/tmp/snap.bmp"  );
            // free(bgrBuf) ;


             // STrace << " new  try end";
        

             //            //  #if(DUMPFILE)

             //  std::ifstream f("./arvind.rgba"); //taking file as inputstream
             //  std::string str;
             //  if(f) {
             //     std::stringstream ss;
             //     ss << f.rdbuf(); // reading data
             //     str = ss.str();
             //  }
             //  else
             //  {
             //      SError << " rgba file does not exist ";
             //      return -1;
             //  }

                
                      
            //   unsigned long  width = 640;
            //   unsigned long  height = 360;
            //   size_t  p_output_size = 0;
              
                
            //   //unsigned char *  bgrBuf = rgba_to_rgb_brg( (const unsigned char*) str.c_str() , str.length(), bitmap_buffer_format_BGR , 0, width , height , &p_output_size );
            //   //write_bmp(bgrBuf, width, height, "arvind.bmp"  );
            //   //free(bgrBuf) ;
              
            //   unsigned char *  rgbBuf = rgba_to_rgb_brg( (const unsigned char*) str.c_str() , str.length(), bitmap_buffer_format_RGB , 0, width , height , &p_output_size );

  
            //  for( int x = 0; x < 100; ++x)
            //  {
            //      XAProcess( rgbBuf , width, height );

            //      base::sleep(100);

            //      STrace << " new  try " << x ; 
            //  }
              

            
            // free(rgbBuf) ;
          
            
        }

      //  ++i;


         //XAProcess( frame->virAddr, frame->width , frame->height) ;

       //  base::sleep(700);

      

        IMP_FrameSource_ReleaseFrame(3, frame);
        if (ret < 0) {
            SError<<"IMP_FrameSource_ReleaseFrame failed";
            return;
        }
        ret = IMP_FrameSource_SetFrameDepth(3, 0);
        if (ret < 0) {
            SError<<"IMP_FrameSource_SetFrameDepth failed";
            return ;
        }

         if(!QRCode)
         base::sleep(700);
    }

    #if(DUMPFILE)

    fclose(fp);

    #endif


    SInfo << "31RGBA::run()" ;

    /* end */
    return ;
}



void T31RGBA::onMessage(json &jsonMsg )
{

   json & rjson  = jsonMsg["messagePayload"];
 
   SInfo << " onMessage "  << rjson.dump();

   std::string  jpegBuffBase64= jsonMsg["registrationImage"].get<std::string>();

   XA_addGallery(jpegBuffBase64, rjson);

}
         


int T31RGBA::XA_addGallery()
{
    const char * galleryIdentityManifest;
    const xa_sdk_identity_images_t * remaining_identity_image_pairs;
    const char * updated_json_identities;
    xa_fi_error_t returnValue;


    cnfg::Configuration identity;

    identity.load("./updated_json_identities.json");


    //if (< a new gallery manifest exists >) 
    if (identity.loaded()) 
    {
          std::string xaidentity = identity.root.dump();

          galleryIdentityManifest = xaidentity.c_str();

          // Step 1
          returnValue = xa_sdk_update_identities(galleryIdentityManifest,
          &remaining_identity_image_pairs,
          &updated_json_identities);
          if (returnValue == XA_ERR_NONE) {
              //< persist updated_json_identities >
              SInfo << "xa_sdk_update_identities passed";      
           }
           else {
              SError << "xa_sdk_update_identities fails";

              return -1;
         }
    

     int totalIdentity = remaining_identity_image_pairs->number_of_remaining_images;
    // Step 2 / Step 4
      if ((returnValue == XA_ERR_NONE) && (remaining_identity_image_pairs->number_of_remaining_images > 0)) 
      {
         SError << "xa_sdk_update_identities fails";
      }

    }

}
    

int T31RGBA::XA_addGallery(std::string jpegBuffBase64 , json & newidentity)
{

    ready_flag = 0;

    
    cnfg::Configuration identity;

    identity.load("./updated_json_identities.json");

    std::string sGal;

    if (identity.loaded()) 
    {
        //std::string xaidentity = identity.root.dump();
      
        if( identity.root.find("configuredGalleryIdentities") != identity.root.end())
        {
        
            if( newidentity.find("configuredGalleryIdentities") != newidentity.end())
            {


                for (json::iterator it = newidentity["configuredGalleryIdentities"].begin(); it != newidentity["configuredGalleryIdentities"] .end(); ++it) {


                    std::cout << it.key() << " : " << it.value() << "\n";

                    if(  (identity.root.find("sequenceNum") != identity.root.end()) && ( identity.root["configuredGalleryIdentities"].find(it.key()) ==  identity.root["configuredGalleryIdentities"].end()       ))
                    {
                        identity.root["sequenceNum"] = identity.root["sequenceNum"].get<int>() + 1;
                    }    

                    identity.root["configuredGalleryIdentities"][it.key()] = it.value();

                }

            }
                
        }
        
          sGal=  identity.root.dump();
    }
    else
    {
         sGal=  newidentity.dump();
    }
    
    
    const char * galleryIdentityManifest = sGal.c_str();
    const xa_sdk_identity_images_t * remaining_identity_image_pairs;
    const char * updated_json_identities;
    xa_fi_error_t returnValue;
    
    SInfo << "jpegBuffBase64 " << jpegBuffBase64.size() << " identity " <<  sGal ;
    
    std::string out;
    //  base64::Decoder dec;
    //  dec.decode(jpegBuffBase64, out);

    out = base64_decode(jpegBuffBase64);

    SInfo << "base64 decoded " << out.size() ;

    int width, height , channels;

    //  if(!stbi_info_from_memory(out, jpegBuffBase64.size(), &width, &height, &channels)) return -1;
    //

    //  /* exit if the image is larger than ~80MB */
    //  if(width && height > (80000000 / 4) / height) return -1;

    unsigned char *img = stbi_load_from_memory(out.c_str(), out.size(), &width, &height, &channels, 3);

    SInfo << "wx: " << width  << " he: " << height <<  " ch: " << channels;




    //if (< a new gallery manifest exists >) 
    {    
   
        // Step 1
        returnValue = xa_sdk_update_identities(galleryIdentityManifest,
        &remaining_identity_image_pairs,
        &updated_json_identities);
        if (returnValue == XA_ERR_NONE) {
            //< persist updated_json_identities >
            SInfo << "xa_sdk_update_identities passed";      

         }
         else {
            SError << "xa_sdk_update_identities fails";
            free(img);
            return -1;
       }
    }

    int totalIdentity = remaining_identity_image_pairs->number_of_remaining_images;
    // Step 2 / Step 4
    while ((returnValue == XA_ERR_NONE) && (remaining_identity_image_pairs->number_of_remaining_images > 0)) 
    {

        int index = totalIdentity - remaining_identity_image_pairs->number_of_remaining_images;

        SInfo << "IdentityIndex:" << index  << " totalIndenty:" << totalIdentity;

        xa_fi_image_t image;
        image.width = width;
        image.height = height;
        image.pixel_format =  XA_FI_COLOR_RGB888;  // signifies the buffer data format
        image.buff = img;


        //< acquire the image for remaining_identity_image_pairs->identity_images[0] >
       // xa_fi_image_t image = < convert the acquired image to xa_fi_image_t - see fi_image.h >

        // Step 3
        returnValue = xa_sdk_add_identity_image(remaining_identity_image_pairs->identity_images[index].identity_id,
        remaining_identity_image_pairs->identity_images[index].image_id,
        &image,
        &remaining_identity_image_pairs,
        &updated_json_identities);

        // Step 5 - persist after each image to avoid needing to download them again
         // this step could also be placed after the while loop
         if (returnValue == XA_ERR_NONE) {
          //< persist updated_json_identities >

            json up_json_identities = json::parse(updated_json_identities); 

            base::cnfg::saveFile("./updated_json_identities.json", up_json_identities );

            SInfo << "remaining_identity_image_pairs passed: " << remaining_identity_image_pairs->identity_images[0].identity_id;
        }
        else 
        {
            SError << "remaining_identity_image_pairs fails " << remaining_identity_image_pairs->identity_images[0].identity_id;

            free(img);
            return -1;

        }
    }

    const char * deviceCheckinJson = xa_sdk_get_device_checkin_json();

    SInfo << "deviceCheckinJson:" << deviceCheckinJson;

    SInfo << "sleep for 10 secs ";

    base::sleep(10000);
    free(img);



      //XA_addGallery(event["registrationImage"].get<std::string>()) ;
    uint8_t* tmpBuf = new uint8_t [width*3*height];
    memset(tmpBuf, 0, width*3*height);
    XAProcess( tmpBuf , width, height );
    delete [] tmpBuf;

     ready_flag = 1;

    //< perform a deviceCheckin with the deviceCheckinJson >

}

int T31RGBA::XAProcess( uint8_t* buffer_containing_raw_rgb_data , int w, int h  )
{
    
    xa_fi_image_t image;
    image.width = w;
    image.height = h;

    image.pixel_format =   XA_FI_COLOR_RGB888;  // signifies the buffer data format


    image.buff =  buffer_containing_raw_rgb_data;  // note this is in RGB order, otherwise
                                         // colors will be swapped

    xa_fi_error_t returnValue;

    xa_sdk_process_image_outputs* process_image_outputs;

    if (1) {
        returnValue = xa_sdk_process_image(&image, &process_image_outputs);

        if (returnValue == XA_ERR_NONE) {
            for (int index = 0;
                 index < process_image_outputs->number_of_json_blobs; ++index) {
                xa_sdk_json_blob_t blob = process_image_outputs->blobs[index];

                if (blob.blob_descriptor == XA_FACE_TRACK_EVENT) {
                    //<
                    // send blob -> json to Face Track Event endpoint >

                    json event = json::parse(blob.json); 

                    SInfo << "json to Face Track Event endpoint";

                    //std::string path = "./event.json";
                    //base::cnfg::saveFile(path, event );

                    if(  (event.find("eventType") != event.end()) )
                    {

                        if(  event.find("registrationImage") != event.end()) 
                        {

                            if(event["eventType"].get<std::string>() ==  "IDENTITY_NOT_IN_GALLERY")
                            {

                              SInfo << "IDENTITY_NOT_IN_GALLERY";

                          
                              #if DUMPFILE

                                cnfg::Configuration identity;

                                identity.load("./identity.json");
                                std::string xaidentity = identity.root.dump();

                                 if (identity.loaded()) 
                                 {
                                  
                                   //XA_addGallery(event["registrationImage"].get<std::string>() ,xaidentity ) ;
                                   // uint8_t* tmpBuf = new uint8_t [image.width*3*image.height];
                                   // memset(tmpBuf, 0, image.width*3*image.height);
                                   // XAProcess( tmpBuf , image.width, image.height );
                                   // delete [] tmpBuf;
                                   
                                 }
                                 else
                                  SError << "./identity.json missing";

                               #else
                              //cnfg::Configuration identity;

                              //identity.load("./event.json");
                              // std::string xaidentity = identity.root.dump();
                                  record = true;
                              
                                  std::string str = event["registrationImage"].get<std::string>();

                                  Timestamp ts;
                                 // Timestamp::TimeVal time = ts.epochMicroseconds();
                                  //int milli = int(time % 1000000) / 1000;

                                  std::time_t time1 = ts.epochTime();
                                  struct std::tm* tms = std::localtime(&time1);

                                  char date[100] = {'\0'}; //"%Y-%m-%d-%H-%M-%S"
                                  int len = std::strftime(date, sizeof (date), "%Y-%m-%d-%H-%M-%S", tms);
                                 
                                  m_date = time1;

                                  json m;
                                  
                                  m["messageType"] = "IDENTITY_NOT_IN_GALLERY";
                                  m["messagePayload"] =  event;
                                  m["ts"] =  date;
                                  m["camid"] = ctx->cam;
                                  ctx->signaler->postAppMessage( m);
                                  RestAPI("POST",  "backend.adapptonline.com", "/eventsToCloudX", m);  
                              #endif
                            }
                            else if(event["eventType"].get<std::string>() ==  "IDENTITY_RECOGNIZED")
                            {
                                 SInfo << "IDENTITY_RECOGNIZED";

                                 record = true;

                                  #if DUMPFILE

                                  #else  

                                  Timestamp ts;
                                  //Timestamp::TimeVal time = ts.epochMicroseconds();
                                  //int milli = int(time % 1000000) / 1000;

                                  std::time_t time1 = ts.epochTime();
                                  struct std::tm* tms = std::localtime(&time1);

                                  char date[100] = {'\0'}; //"%Y-%m-%d-%H-%M-%S"
                                  int len = std::strftime(date, sizeof (date), "%Y-%m-%d-%H-%M-%S", tms);
                                 
                                  m_date = time1;

                                  json m;
                                 
                                  m["messageType"] = "IDENTITY_RECOGNIZED";
                                  m["messagePayload"] =  event;
                                  m["camid"] = ctx->cam;
                                  m["ts"] =  date;
                                  ctx->signaler->postAppMessage( m);

                                  RestAPI("POST",  "backend.adapptonline.com", "/eventsToCloudX", m);  

                                  #endif  
                            }


                        }
                        else
                         SError << "no registrationImage " <<  event.dump(4);  

                     }
                     else
                     {
                       SError << "no eventType " << event.dump(4);
                     }



                } else if (blob.blob_descriptor == XA_ACCURACY_MONITOR) {
                    //<
                    // send blob -> json to Accuracy Monitor endpoint >
                    STrace << "send blob -> json to Accuracy Monitor endpoint: " <<  blob.json;
                } else {
                    SError << "Not a possible state";
                }
            }
        } else {
            SError << "Error at process_image_outputs";
        }

        //std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

}


/*******************************************************************************************
  T31RGBA end
*******************************************************************************************/

int T31RGBA::T31RGBAInit()
{
    
    if(ctx )
    {
        blueLed();
    
       
    }
    else
    qrcode_init();
    return 0;
}
int T31RGBA::T31RGBAExit()
{
    if(!ctx )
    {
      qrcode_exit();
    }

    return 0;
}


T31RGBA::~T31RGBA() 
{
  stop();
  join();
  T31RGBAExit();

  SInfo << "~t31rgba->()";

}


/*******************************************************************************************
  T31H264 start
*******************************************************************************************/


void T31H264::play( unsigned char *str, int len)
{


#if(DUMPFILE)

#else

  basicframe.data = str ;
  basicframe.sz = len;
            

  if(ctx->liveFrame)
  ctx->liveFrame->run(&basicframe); // starts the frame filter chain
  //ctx->muRecFrame.unlock(); 
  basicframe.payload.resize(basicframe.payload.capacity());

 #endif 
}



void T31H264::run()
{
 
  #if(DUMPFILE)
//  IMP_LOG_DBG(TAG, "Video ChnNum=%d Open Stream file %s ", chnNum, stream_path);
   FILE *stream_fd = fopen("/tmp/test.264", "wb"); 
   if (stream_fd < 0) {
      SError <<  "could not open file /tmp/test.264" <<  chnNum;
     return ;
   }
   #endif

   //IMP_LOG_DBG(TAG, "OK\n");

  T31H264Init(2);
  
  while (!stopped())
  {


   
    if( HDVideo !=  chnNum)
    {
      T31H264Exit();
      T31H264Init(HDVideo);
    }
    

    int ret = IMP_Encoder_PollingStream(chnNum, 1000);
    if (ret < 0) {
      SError <<  "IMP_Encoder_PollingStream timeout" <<  chnNum;
      continue;
    }

    IMPEncoderStream stream;
    /* Get H264 or H265 Stream */
    ret = IMP_Encoder_GetStream(chnNum, &stream, 1);
#ifdef SHOW_FRM_BITRATE
    int i, len = 0;
    for (i = 0; i < stream.packCount; i++) {
      len += stream.pack[i].length;
    }
    bitrate_sp[chnNum] += len;
    frmrate_sp[chnNum]++;

    int64_t now = IMP_System_GetTimeStamp() / 1000;
    if(((int)(now - statime_sp[chnNum]) / 1000) >= FRM_BIT_RATE_TIME){
      double fps = (double)frmrate_sp[chnNum] / ((double)(now - statime_sp[chnNum]) / 1000);
      double kbr = (double)bitrate_sp[chnNum] * 8 / (double)(now - statime_sp[chnNum]);

      printf("HDVideo[%d] streamNum[%d]:FPS: %0.2f,Bitrate: %0.2f(kbps)\n", HDVideo, chnNum, fps, kbr);
      //fflush(stdout);

      frmrate_sp[chnNum] = 0;
      bitrate_sp[chnNum] = 0;
      statime_sp[chnNum] = now;
    }
#endif
    if (ret < 0) {
     SError <<  "IMP_Encoder_GetStream failed " <<  chnNum;
      return ;
    }

//    ret = save_stream(stream_fd, &stream);
    int i, nr_pack = stream.packCount;

//IMP_LOG_DBG(TAG, "----------packCount=%d, stream->seq=%u start----------\n", stream->packCount, stream->seq);
    for (i = 0; i < nr_pack; i++) {
//IMP_LOG_DBG(TAG, "[%d]:%10u,%10lld,%10u,%10u,%10u\n", i, stream->pack[i].length, stream->pack[i].timestamp, stream->pack[i].frameEnd, *((uint32_t *)(&stream->pack[i].nalType)), stream->pack[i].sliceType);
            IMPEncoderPack *pack = &stream.pack[i];
            if(pack->length){
                    uint32_t remSize = stream.streamSize - pack->offset;
                    if(remSize < pack->length){
                            #if(DUMPFILE)
                            fwrite( (void *)(stream.virAddr + pack->offset), remSize,1,stream_fd);
                            fwrite( (void *)stream.virAddr, pack->length - remSize, 1, stream_fd);
                            #else
                            play( (stream.virAddr + pack->offset), remSize );
                            play( stream.virAddr, pack->length - remSize );
                            
                            #endif
                    }else {
                            #if(DUMPFILE)
                            fwrite( (void *)(stream.virAddr + pack->offset), pack->length, 1, stream_fd);
                            #else
                             play( (stream.virAddr + pack->offset), pack->length );
                            #endif
                    }
            }
    }

    
    #if(DUMPFILE)
    if (ret < 0) {
      fclose(stream_fd);
      return ;
    }
    #endif

    IMP_Encoder_ReleaseStream(chnNum, &stream);


  }

  T31H264Exit();

   SInfo << "T31H264::run()" ;


  return ;
    
    
}

int T31H264::T31H264Init( int ch)
{



  if (ch < FS_CHN_NUM && chn[ch].enable)
   {    
       chnNum = chn[ch].index;
  
       int ret = IMP_Encoder_StartRecvPic(chnNum);
        if (ret < 0) {
          SError <<  "IMP_Encoder_StartRecvPic failed\n" <<  chnNum;
          return -1;
        }

      SInfo << "T31H264Init chnNum " << chnNum; 
  }
  else
  {
      SError << "T31H264Init not enabled chnNum " << chnNum; 

  }

  
    return 0;
}

int T31H264::T31H264Exit()
{

  int ret = IMP_Encoder_StopRecvPic(chnNum);
  if (ret < 0) {
    SError << "IMP_Encoder_StopRecvPic failed" <<  chnNum;
    return -1;
  }
      
    return ret;
}

 T31H264::~T31H264() {

  stop();
  join();
  



  SInfo << "~T31H264()";

}



/*******************************************************************************************
  T31H264 end
*******************************************************************************************/





/*******************************************************************************************
  LiveThread start
*******************************************************************************************/


int LiveThread::T31Init()
{
    int i, ret;

    /* Step.1 System init */
    ret = sample_system_init();
    if (ret < 0) {
        SError<<"IMP_System_Init() failed";
        return -1;
    }

    chn[3].enable = 0;
    chn[2].enable = 1;
    ret = sample_framesource_init();
    if (ret < 0) {
        SError<<"FrameSource init failed";
        return -1;
    }

    ret = sample_framesource_ext_rgba_init();
    if (ret < 0) {
        SError<<"FrameSource init ext rgba failed";
        return -1;
    }

    for (i = 0; i < FS_CHN_NUM; i++) {
        if (chn[i].enable) {
            ret = IMP_Encoder_CreateGroup(chn[i].index);
            if (ret < 0) {
                SError<<"IMP_Encoder_CreateGroup(%d) error: " <<  i ;
                return -1;
            }
        }
    }

    /* Step.3 Encoder init */
    ret = sample_encoder_init();
    if (ret < 0) {
        SError<<"Encoder init failed";
        return -1;
    }

    /* Step.4 Bind */
    for (i = 0; i < FS_CHN_NUM; i++) {
        if (chn[i].enable) {
            ret = IMP_System_Bind(&chn[i].framesource_chn, &chn[i].imp_encoder);
            if (ret < 0) {
                SError<<"Bind FrameSource channel%d and Encoder failed: " << i ;
                return -1;
            }
        }
    }

    /* Step.5 Stream On */
    ret = sample_framesource_streamon();
    if (ret < 0) {
        SError<<"ImpStreamOn failed";
        return -1;
    }
    ret = sample_framesource_ext_rgba_streamon();
    if (ret < 0) {
        SError<<"ImpStreamOn ext rgba failed";
        return -1;
    }


    /* Step.6 Get stream */



    // ret = sample_get_video_stream();  // For hh264 stream
    // if (ret < 0) {
    //     SError<<"Get H264 stream failed";
    //     return -1;
    // }


    return 0;
}


int LiveThread::T31Exit()
{
     int ret;
    /* Exit sequence as follow */

    /* Step.a Stream Off */
    ret = sample_framesource_streamoff();
    if (ret < 0) {
        SError<<"FrameSource StreamOff failed";
        return -1;
    }

    ret = sample_framesource_ext_rgba_streamoff();
    if (ret < 0) {
        SError<<"FrameSource StreamOff  ext rgba failed";
        return -1;
    }

    /* Step.b UnBind */
    for (int i = 0; i < FS_CHN_NUM; i++) {
        if (chn[i].enable) {
            ret = IMP_System_UnBind(&chn[i].framesource_chn, &chn[i].imp_encoder);
            if (ret < 0) {
                SError<<"UnBind FrameSource channel%d and Encoder failed: "  << i ;
                return -1;
            }
        }
    }

    /* Step.c Encoder exit */
    ret = sample_encoder_exit();
    if (ret < 0) {
        SError<<"Encoder exit failed";
        return -1;
    }

    /* Step.d FrameSource exit */
    ret = sample_framesource_exit();
    if (ret < 0) {
        SError<<"FrameSource exit failed";
        return -1;
    }
    ret = sample_framesource_ext_rgba_exit();
    if (ret < 0) {
        SError<<"FrameSource exit rgba exit failed";
        return -1;
    }

    /* Step.e System exit */
    ret = sample_system_exit();
    if (ret < 0) {
        SError<<"sample_system_exit() failed";
        return -1;
    }
}





typedef void (*xa_sdk_log_callback_function_t)(xa_fi_log_type_t log_level, const char * log_string);


typedef void(*xa_fi_log_cb_t)(xa_fi_log_type_t, const char*);

void logXaFi(xa_fi_log_type_t, const char*)
{

  //printf("never gets called \n");

}


void logXa(xa_fi_log_type_t log_level, const char * log_string)
{

   //printf("arvind log testing  \n");

}




int LiveThread::XAInit()
{


  //  #if(DUMPFILE)

  //   std::ifstream f("./arvind.rgba"); //taking file as inputstream
  //   std::string str;
  //   if(f) {
  //      std::stringstream ss;
  //      ss << f.rdbuf(); // reading data
  //      str = ss.str();
  //   }
  //   else
  //   {
  //       SError << " rgba file does not exist ";
  //       return -1;
  //   }

    
    
  //  //unsigned long  p_width = 0;
  //  //unsigned long  p_height = 0;
  //  //size_t  p_output_size = 0;
  //  //unsigned char *  tmp = rgbaMagic_decode( (const unsigned char*) str.c_str() , str.length(), bitmap_buffer_format_RGB , 0,  &p_width , &p_height , &p_output_size );
   
  //  //w=640 h=360
           
           
  // unsigned long  width = 640;
  // unsigned long  height = 360;
  // size_t  p_output_size = 0;
  
    
  // //unsigned char *  bgrBuf = rgba_to_rgb_brg( (const unsigned char*) str.c_str() , str.length(), bitmap_buffer_format_BGR , 0, width , height , &p_output_size );
  // //write_bmp(bgrBuf, width, height, "arvind.bmp"  );
  // //free(bgrBuf) ;
  
  // unsigned char *  rgbBuf = rgba_to_rgb_brg( (const unsigned char*) str.c_str() , str.length(), bitmap_buffer_format_RGB , 0, width , height , &p_output_size );

  
  // #endif
  

    xa_sdk_log_callback_function_t log_function = &logXa;

    //xa_fi_log_cb_t cb = &logXaFi;

    xa_sdk_register_log_callback(log_function);

   // xa_fi_set_log_callback(cb);



    cnfg::Configuration config;

    config.load("./configXA.json");

   
    
    if (!config.loaded()) 
    {
        SError << "Could not load config";

        return -1;
    }
     std::string xaconfig = config.root.dump();
     
    
    xa_fi_error_t returnValue;

    const char* path_to_vision_cell =
        "/mnt/libxailient-fi-vcell.so";                    // For shared lib
    returnValue = xa_sdk_initialize(path_to_vision_cell);  // For shared lib

    // returnValue = xa_sdk_initialize(); // For static lib

    if (returnValue != XA_ERR_NONE) {
        SError << "Error at xa_sdk_initialize";

        return -1;
    }

    const char* configuration = xaconfig.c_str();

    STrace << "config json: " << configuration;

    //xa_fi_set_log_callback(cb);
   // xa_sdk_register_log_callback(log_function);
    

    // xa_sdk_update_identities
    // xa_sdk_update_identity_image
    returnValue = xa_sdk_configure(configuration);
    if (returnValue != XA_ERR_NONE) {
        SError << "Error at xa_sdk_configure";

        return -1;
    }

    returnValue = xa_sdk_is_face_recognition_enabled();
    if (returnValue != XA_ERR_NONE) {
        SError << "Error at xa_sdk_configure";

        return -1;
    }


    
    
            
    /*
    cnfg::Configuration event;
    event.load("./event.json");

    if( event.root.find("registrationImage") == event.root.end()) 
    {
       SError  << " no registrationImage found in event" ;

       return -1;
    }

    XA_addGallery(event.root["registrationImage"].get<std::string>()) ;
    */

//    for( int x = 0; x < 100; ++x)
//    {
//        XAProcess( rgbBuf , width, height );
//
//        base::sleep(700);
//    }
    
  // #if(DUMPFILE)
  
  // free(rgbBuf) ;
  
  // #endif


}


int LiveThread::XAExit()
{

  xa_sdk_uninitialize();

}


 LiveThread::~LiveThread() {

  if(!record)
  {

    T31Exit();

    if(!QRCode)
    XAExit();
  }

  SInfo << "~LiveThread";

}


void LiveThread::stop()
{
    SInfo << "LiveThread:: stop" << record  << " recording " << recording;

    
      
    if( record)
    {
         SInfo << "recording:: stop";
        recording->stop();
        recording->join();
        delete recording ;
        recording = nullptr;
        SInfo << "recording:: over";
    }
    else
    {
        SInfo << "t31rgba-:: stop";

        t31rgba->stop();

        t31h264->stop();

        t31rgba->join();

        t31h264->join();


        delete t31h264 ;
        t31h264 = nullptr;
        delete t31rgba ;
        t31rgba = nullptr;

         SInfo << "t31rgba-:: over";
    }

    SInfo << "LiveThread:: stop over";
   
}


 LiveThread::LiveThread(const char* name, LiveConnectionContext *ctx, st_track *trackInfo, bool &record, bool QRCode):ctx(ctx),trackInfo(trackInfo),record(record),QRCode(QRCode)
{

    if(!record)
    {
        t31h264 =  new  T31H264(ctx, trackInfo);
        
        if( Settings::configuration.ai)
        t31rgba =  new  T31RGBA(ctx, trackInfo, QRCode);
        
    }
    else
    {
       recording =  new  Recording(ctx, trackInfo); 
    }
        
    
}


void LiveThread::start()
{
    
    if( record)
    {
        recording->start();
    }
    else
    {
       if(!QRCode)
        XAInit();
        T31Init();


       t31rgba->T31RGBAInit();
        if(!QRCode)
       t31h264->start();
       t31rgba->start();
    }
}
#if(DUMPFILE)

void Recording::run()
{

}


#else

void Recording::run()
{
 

    char outPutNameBuffer[256]={'\0'};

    int ncount = 0;
    
    std::string date = Settings::configuration.storage + trackInfo->start; 
             
    while(!stopped() )
    {

        ncount = ncount%250;

        sprintf(outPutNameBuffer, "%s/frame-%.3d.h264",date.c_str(), ++ncount);

        FILE *fp = fopen(outPutNameBuffer, "rb");
        if(!fp) {
             SError << "Error: cannot open: " <<  outPutNameBuffer;
             return ;
        }


        int bytes_read = (int)fread(inbuf, 1, H264_INBUF_SIZE, fp);

        if(bytes_read) {
           basicframe.data = inbuf ;
           basicframe.sz = bytes_read;
        }


       // ctx.muRecFrame.lock();
        if(ctx->recFrame)
        ctx->recFrame->run(&basicframe); // starts the frame filter chain
        //ctx->muRecFrame.unlock(); 

        //SInfo << "payload " << bytes_read;
        basicframe.payload.resize(basicframe.payload.capacity());
        fclose(fp);

       //  base::sleep(10);   
    }
        
      
    SInfo << "Recording::run() over";
        
}
#endif


}}
