#include <drogon/drogon.h>
#include "Handler.h"
#include "DataBase.h"


using namespace drogon;
using namespace std;


DataBase db;




int main() {

    app().setThreadNum(4);
    app().setDocumentRoot("../images/");
    app().setStaticFileHeaders({
        {"Cache-Control", "public, max-age=86400"},
        {"Access-Control-Allow-Origin", "http://localhost:5173"},
        {"Access-Control-Allow-Credentials", "true"}
    });


    app().registerPreRoutingAdvice([](const drogon::HttpRequestPtr &req, drogon::FilterCallback &&defer, drogon::FilterChainCallback &&chain) {
        if (req->method() == Options) {
            auto resp = drogon::HttpResponse::newHttpResponse();
            resp->setStatusCode(k200OK);
            
            string origin = req->getHeader("Origin");
            if (origin.empty()){
                origin = "http://localhost:5173";
            }
            
            resp->addHeader("Access-Control-Allow-Origin", origin);
            resp->addHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
            resp->addHeader("Access-Control-Allow-Headers", "Content-Type, Accept, Authorization, X-Requested-With, Cookie");
            resp->addHeader("Access-Control-Allow-Credentials", "true");
            resp->addHeader("Access-Control-Max-Age", "3600");
            
            defer(resp);
            return;
        }
        chain();
    });


    app().registerHandler("/register", &Handler::RegisterUser, {Post});

    app().registerHandler("/login", &Handler::AutoriseUser, {Post});

    app().registerHandler("/cats", &Handler::GetCats, {Get});
    app().registerHandler("/cats", &Handler::uploadCatPhoto, {Post});
    app().registerHandler("/cats/{id}",&Handler::updateCatTagsAndMedical,{Put});

    app().registerHandler("/bookings",&Handler::AddToBookings,{Post});

    app().registerHandler("/profile",&Handler::GetUserData,{Get});

    app().registerHandler("/bookings/admin",&Handler::GetAdminBookings,{Get});
    app().registerHandler("/bookings/admin",&Handler::AddAdminBooking,{Post});
    app().registerHandler("/bookings/admin",&Handler::ConfirmAdminBookings,{Put});
    app().registerHandler("/bookings/admin",&Handler::RejectAdminBooking,{Delete});

    app().registerHandler("/logout",&Handler::LogOut,{Post});

    cout<<"server is running"<<endl;

    app().addListener("0.0.0.0", 8000).run();

    return 0;
}