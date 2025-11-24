/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Api.h
 * Author: Arvind and Herman
 *
 * Created on Nov 25, 2025
 */

#ifndef RESTAPI_H
#define RESTAPI_H


#include "base/async.h"
#include "nlohmann/json.hpp"

using namespace base;



using json = nlohmann::json;
 
class RestAPI 
{
public:

    RestAPI(Async &async):async(async)
    {
        
    }
    
    ~RestAPI()
    {
        
    }
            
    void Run(std::string method, std::string url,json &body);
    
    void Insert(std::string file,  std::string content  );

    Async &async;
    char mac_addr[18];
    
    struct stMap{
//        stMap(std::string sha,  std::string content):sha(sha),content(content)
//        { }
        std::string sha;
        std::string content;
    };
    std::map<std::string,  stMap> mapFile;
             
};



#endif /* RESTAPI_H */

