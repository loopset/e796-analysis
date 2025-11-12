#include "ActCrossSection.h"

#include "TCanvas.h"
#include "TGraphErrors.h"
#include "TMultiGraph.h"
#include "TString.h"

#include "FitInterface.h"

#include <iostream>
#include <string>
#include <vector>
void plot_all()
{
    std::string beam {"20O"};
    std::string target {"2H"};
    std::string light {"3H"};

    ActSim::CrossSection* xs {};
    // We use the Fitters interface to get the proper xs file
    Fitters::Interface inter;
    std::string interpath {"Outputs/interface.root"};
    std::string comppath {"comps.conf"};
    std::string xspath {};
    if(target == "1H" && light == "1H")
        xspath = "/media/Data/E796v2/Fits/pp/";
    else if(target == "2H" && light == "2H")
        xspath = "/media/Data/E796v2/Fits/dd/";
    else if(target == "2H" && light == "3H")
        xspath = "/media/Data/E796v2/Fits/dt/";
    else if(target == "2H" && light == "3He")
        xspath = "/media/Data/E796v2/Fits/d3He/";
    else if(target == "2H" && light == "4He")
        xspath = "";
    else if(target == "1H" && light == "2H")
        xspath = "/media/Data/E796v2/Fits/pd/";
    else if(target == "1H" && light == "3H")
        xspath = "";
    else
        throw std::runtime_error("Simulation_E796(): no known xs config");
    // Init interface
    inter.Read(xspath + interpath);
    inter.ReadCompConfig(xspath + "comps.conf");
    // inter.Print();
    std::vector<TGraphErrors*> gs;
    auto* mg {new TMultiGraph};
    mg->SetTitle(";#theta_{CM} [#circ];d#sigma/d#Omega");
    // Exs states
    std::vector<double> Eexs = {0., 1.4, 3.2, 4.6, 6.7, 7.9, 8.9, 10.7, 12.3, 13.9, 14.9, 16.2};
    for(const auto& ex : Eexs)
    {
        auto key {inter.GetKeyOfGuess(ex)};
        std::cout << "Ex : " << ex << " key : " << key << '\n';
        if(!key.size())
            continue;
        auto xsfile {inter.GetTheoCrossSection(key)};
        auto* g {new TGraphErrors {TString::Format("%s%s", xspath.c_str(), xsfile.c_str()), "%lg %lg"}};
        g->SetTitle(key.c_str());
        mg->Add(g);
        gs.push_back(g);
    }

    // Draw
    auto* c0 {new TCanvas {"c0", "Simu xs"}};
    c0->DivideSquare(4);
    c0->cd(1);
    mg->Draw("al plc");
}
