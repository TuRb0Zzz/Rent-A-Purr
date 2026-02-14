#pragma once
#include <drogon/drogon.h>
#include "DataBase.h"

#include <openssl/sha.h>
#include <iomanip>
#include <sstream>

#include <json/json.h>
using namespace std;
using namespace drogon;

class Handler{
    private:
        static string hashPassword(const string& password);

        static bool verifyPassword(const string& plainPassword, const string& hashedPassword);

    public:

        static void RegisterUser(const HttpRequestPtr& request,std::function<void(const HttpResponsePtr&)>&& callback);
        
        //void AutoriseUser(const HttpRequestPtr& req,function<void(const HttpResponsePtr&)> callback);
};