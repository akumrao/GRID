/* This file is part of mediaserver. A RTSP live server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
  for testing reset api use Postman
 * https://web.postman.co/workspace/My-Workspace~292b44c7-cae4-44d6-8253-174622f0233e/request/create?requestId=e6995876-3b8c-4b7e-b170-83a733a631db
 */

#define k1 "11AAIZCGA0ZrNXIa9aRuqO"
#define k2 "_o0IYnoxMOlWrHdf4wiL7nqhCWHqpDW5o"
#define k3 "hmIH5ZM2fdC7QZIK2Z454THFiEW"


#include "RestAPI.h"

#include "Settings.h"
#include "net/netInterface.h"
#include "http/HttpsClient.h"
#include "base/application.h"


using namespace base::net;


void RestAPI::Insert(std::string file,  std::string content  )
{
  //mConfig.api->mapFile.find(mConfig.api->mac_addr) != mConfig.api->mapFile.end() 
        //mConfig.api->mapFile[mConfig.api->mac_addr].content =  localdesp.dump();
  
}
  
  
void RestAPI::Run(std::string method, std::string url,json &body)
{
    auto work_fn = [method, url, body ]() {
             
        Application app;

        std::string result;

        std::string sendMe = body.dump();

        //ClientConnecton *conn = new HttpsClient( "https", "ipcamera.adapptonline.com", 8080, uri);
        ClientConnecton *conn = new HttpsClient(url);
        //Client *conn = new Client("http://zlib.net/index.html");
        conn->fnComplete = [&](const Response & response) {
            std::string reason = response.getReason();
            StatusCode statuscode = response.getStatus();
         //   std::string body = conn->readStream() ? conn->readStream()->str() : "";
          //  STrace << "Post API reponse" << "Reason: " << reason << " Response: " << body;
        };

        conn->fnConnect = [&, sendMe](HttpBase * con) {

           // SInfo << sendMe.length();

            con->send( sendMe.c_str(), sendMe.length(), false);

        };

        conn->fnPayload = [&](HttpBase * con, const char* data, size_t sz) {

           // std::cout  << string( data,sz) << std::endl << std::flush;
            result +=  std::string( data,sz);



           // json root = json::parse(data);            
          //  std::cout << "client->fnPayload " << root.dump(4) << std::endl << std::flush;

        };

        conn->fnClose = [&](HttpBase * con, std::string str) {

          json root = json::parse(result.c_str());        

          
          for (auto& element : root) {
            
              std::cout << element << '\n';
            
          }
          
            std::cout << "client->fnPayload " << root.dump(4) << std::endl << std::flush;
        };



        conn->_request.setMethod(method);
        conn->_request.setKeepAlive(false);

        conn->_request.setContentLength(sendMe.size());
        conn->_request.setContentType("application/json");
        conn->_request.add( "Accept", "application/vnd.github+json"  );
        conn->_request.add( "Authorization", std::string("Bearer github_pat_") + std::string(k1) + k2 + std::string(k3)  );
        conn->_request.add( "User-Agent", "Awesome-Octocat-App"  );

        conn->setReadStream(new std::stringstream);
        conn->send();


        app.run();
    };    

    
    auto after_work_fn = [](int status) 
    {
            // This runs back in the main event loop thread
            std::cout << "Main thread: Work finished with status " << status << std::endl;
    };

    async.queueWork(work_fn, after_work_fn);

}
