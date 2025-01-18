#include "TCanvas.h"
#include "TFile.h"
#include "THttpEngine.h"
#include "THttpServer.h"
#include "TSystem.h"
void server()
{
    auto* s {new THttpServer {"http:8080"}};
    s->SetTerminate();
    // Open in browser
    // gSystem->Exec("xdg-open http://localhost:8080/currentdir/index.html &");
    // Register data
    // server->Register("Files/", f);
}
