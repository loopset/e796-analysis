#ifndef Runner_cxx
#define Runner_cxx

#include "ActKinematics.h"

#include "TCanvas.h"
#include "TEnv.h"
#include "TString.h"

#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>

#include "../Analyser.cxx"
#include "../Plotter.cxx"

// Alias for std::filesystem
namespace fs = std::filesystem;

void Runner(TString what = "plot")
{
    gEnv->SetValue("IterPath", "./Fractions/mix85_15");

    std::string beam {"16N"};
    std::string target {"d"};
    std::string light {"3He"};
    double ebeam {640.0};
    std::vector<double> exs {};
    if(target == "d")
    {
        if(light == "d")
            exs = {0};
        else if(light == "3He")
            exs = {0, 0.74, 3.10, 11.0};
        else
            throw std::runtime_error("No exs config for this channel");
    }

    if(what.Contains("ana") || what.Contains("+"))
    {
        // Call and save
        std::vector<RetAna> rets;
        for(const auto& ex : exs)
        {
            rets.push_back(Analyse(beam, target, light, ebeam, ex));
        }
        // Simple plot
        for(int i = 0; i < rets.size(); i++)
        {
            auto* c {new TCanvas {TString::Format("c%d", i), TString::Format("Simu canvas Ex = %.2f", exs[i])}};
            c->DivideSquare(6);
            auto& ret {rets[i]};
            c->cd(1);
            ret.hKinSampled->Draw("colz");
            c->cd(2);
            ret.hKin->Draw("colz");
            ActPhysics::Kinematics k {
                TString::Format("%s(%s,%s)@%.2f|%.2f", beam.c_str(), target.c_str(), light.c_str(), ebeam, exs[i])
                    .Data()};
            k.GetKinematicLine3()->Draw("l");
            c->cd(3);
            ret.hPID->Draw("colz");
            c->cd(4);
            ret.hEx->Draw();
            c->cd(5);
            ret.eff->Draw("apl");
            c->cd(6);
            // Divide by sin (thetaCM)
            auto* fsolid {new TF1 {"fsolid", "TMath::Sin(x * TMath::DegToRad())", 0, 180}};
            ret.hCMAll->Divide(fsolid);
            ret.hCMAll->Draw();
        }
    }
    if(what.Contains("plot") || what.Contains("+"))
    {
        // Call plotter function
        Plotter(beam, target, light, ebeam, exs);
    }
    if(what.Contains("comp"))
    {
        // Scan up dir for subdirs
        fs::path iterPath {gEnv->GetValue("IterPath", ".")};
        auto up {iterPath.parent_path()};

        // Get all down directories
        std::vector<std::string> dirs;
        for(const auto& path : fs::directory_iterator(up))
        {
            if(path.is_directory())
                dirs.push_back(path.path().string());
        }
        // And push back final
        dirs.push_back("./final/");

        // Call plot and then build a comparator
        std::vector<RetPlot> rets;
        for(const auto& down : dirs)
        {
            gEnv->SetValue("IterPath", down.c_str());
            rets.push_back(Plotter(beam, target, light, ebeam, exs));
        }
        // Plot
        auto* c {new TCanvas {"cComp", "Comparator of simus"}};
        c->DivideSquare(6);
        for(int i = 0; i < rets.size(); i++)
        {
            auto& ret {rets[i]};
            c->cd(i + 1);
            ret.mEffs->SetTitle(dirs[i].c_str());
            ret.mEffs->Draw("apl plc pmc");
        }
    }
}

#endif