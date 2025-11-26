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

#include <mutex>


using namespace base;



using json = nlohmann::json;
 
class RestAPI 
{
public:

    RestAPI(Async &async, std::string room):async(async), room(room)
    {
        
    }
    
    ~RestAPI()
    {
        
    }
    
    void List();
    
    void Put( std::string file,  std::string content,  std::string sha );
    void Get( std::string file );
            
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
    
    std::mutex mutMapFIle;
    std::string room;
             
};



#endif /* RESTAPI_H */

