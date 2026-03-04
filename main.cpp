#include <drogon/drogon.h>
#include "Handler.h"
#include "DataBase.h"

using namespace drogon;
using namespace std;

DataBase db;





int main() {

    app().setDocumentRoot("../images");
    app().setStaticFileHeaders({{"Cache-Control", "public, max-age=86400"}});


    app().registerHandler("/register", &Handler::RegisterUser, {Post});
    app().registerHandler("/register", &Handler::handleOptions, {Options});

    app().registerHandler("/login", &Handler::AutoriseUser, {Get});
    app().registerHandler("/login", &Handler::handleOptions, {Options});

    app().registerHandler("/cats", &Handler::GetCats, {Get});
    app().registerHandler("/cats", &Handler::handleOptions, {Options});
    app().registerHandler("/cats", &Handler::uploadCatPhoto, {Post});
    app().registerHandler("/cats/{id}",&Handler::updateCatTagsAndMedical,{Put});

    app().registerHandler("/bookings",&Handler::AddToBookings,{Post});
    app().registerHandler("/bookings", &Handler::handleOptions, {Options});

    app().registerHandler("/bookings/admin",&Handler::GetAdminBookings,{Get});
    app().registerHandler("/bookings/admin",&Handler::ConfirmAdminBookings,{Put});
    app().registerHandler("/bookings/admin",&Handler::RejectAdminBooking,{Delete});
    app().registerHandler("/bookings/admin", &Handler::handleOptions, {Options});

    cout<<"server is running"<<endl;
    app().addListener("0.0.0.0", 8000).run();

    return 0;
}