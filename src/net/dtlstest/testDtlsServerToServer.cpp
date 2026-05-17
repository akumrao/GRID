/* This file is part of mediaserver. A webrtc sfu server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

#include "base/base.h"
#include "base/logger.h"
#include "base/application.h"
#include "net/TcpServer.h"
// #include "base/test.h"
#include "base/time.h"
#include "net/SslConnection.h"
#include "DtlsTransport.h"
#include "net/certificate.h"
#include "json/confSettings.h"
#include "json/configuration.h"
#include "configuration.h"

#include "Router.h"


using std::endl;
using namespace base;
using namespace net;

extern ConfCert config;




int main(int argc, char** argv) {
    
        Logger::instance().add(new ConsoleChannel("debug", Level::Trace));


 
     //  net::SSLManager::initNoVerifyServer();

        Application app;
        //SecTcpServer *tcpServer = new SecTcpServer(nullptr, "0.0.0.0", 5001, false, true);
        
        rtc::Router router1("1", 1);
        rtc::Router router2("2", 2);
        
        
        base::cnfg::Configuration cnfgSet;
        cnfgSet.load("./config.js");


        try {
            ConfSettings::SetConfiguration(cnfgSet.root);
        } catch (const std::exception& error) {

         //  Settings::exit();
            std::_Exit(-1);
        } 
    


        
        
                
        #if HTTPSSL
        
        config.certificatePemFile = ConfSettings::configuration.certFile;
        config.keyPemFile =ConfSettings::configuration.keyFile  ;

        #else
        SInfo << "http://localhost:8000";
        #endif
        
        rtc::SctpTransport::Init();
        rtc::SctpSettings mCurrentSctpSettings = {};
        rtc::SctpTransport::SetSettings(mCurrentSctpSettings);
        
   
        rtc::DtlsTransport::ClassInit();
      
        rtc::Configuration transportconfig;
        
       rtc::CertificateFingerprint fingerPrint =  config.mCertificate->fingerprint();
        
        router1.HandleRequest(true, transportconfig, 8000, 9000, "127.0.0.1", "127.0.0.1",  fingerPrint);
        router2.HandleRequest(false,transportconfig, 9000, 8000, "127.0.0.1", "127.0.0.1", fingerPrint);

        app.waitForShutdown([&](void*) {

         router1.Close();
         router2.Close();
            
             rtc::DtlsTransport::ClassDestroy();

        });



    return 0;
}