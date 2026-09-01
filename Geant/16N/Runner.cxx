#ifndef Runner_cxx
#define Runner_cxx

#include "ActKinematics.h"

#include "TCanvas.h"
#include "TEnv.h"
#include "TString.h"
#include "TVirtualPad.h"

#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>

#include "../Analyser.cxx"
#include "../Plotter.cxx"
#include "../Yield.cxx"
#include "yaml-cpp/yaml.h"

// Alias for std::filesystem
namespace fs = std::filesystem;

void Comparator(const std::vector<RetPlot>& ret);

std::vector<double> GetExsFromYAML(const std::string& file)
{
    YAML::Node config = YAML::LoadFile(file);
    auto exs {config["exs"].as<std::vector<double>>()};
    return exs;
}

void Runner(TString what = "plot")
{
    gEnv->SetValue("IterPath", "./Pressure/p200");

    std::string beam {"16N"};
    std::string target {"d"};
    std::string light {"d"};
    double ebeam {640.0};
    // Parameters of exp
    double intensity {5e3};
    double duration {8 * 3600 * 3 * 5}; // X days (1 day = 3 UTs; 1 UT = 8h)
    double Nb {intensity * duration};
    double Nt {(90. * 2) / (90 * 2 + 10 * 4 + 10 * 10) * 1.92e21}; // LISE++ calculation
    // Parameters of simu
    double Nit {5e5};

    // Set config file
    std::string yaml {};
    if(target == "d")
    {
        if(light == "d")
            yaml = "./dd.yaml";
        else if(light == "3He")
            yaml = "d3He.yaml";
        else
            throw std::runtime_error("No YAML config for this channel");
    }
    auto exs {GetExsFromYAML(yaml)};

    if(what.Contains("ana") || what.Contains("+"))
    {
        // Call ana and plot
        for(const auto& ex : exs)
            Analyse(beam, target, light, ebeam, ex, true);
    }
    if(what.Contains("plot") || what.Contains("+"))
    {
        // Call plotter function
        Plotter(beam, target, light, ebeam, exs);
    }
    if(what.Contains("comp"))
    {
        // Scan up dir for subdirs
        // std::vector<std::string> ups {"./Fractions", "./Pressure"};
        std::vector<std::string> ups {"./Pressure"};
        // std::vector<std::string> ups {"./Fractions"};

        // Get all subdirs directories
        std::vector<std::string> dirs;
        for(const auto& up : ups)
        {
            for(const auto& path : fs::directory_iterator(up))
            {
                if(path.is_directory())
                {
                    dirs.push_back(path.path().string());
                    std::cout << "Found subdir : " << path.path().string() << '\n';
                }
            }
        }
        // And push back final
        // dirs.push_back("./final/");

        // Call plot and store results for each subdir
        std::vector<RetPlot> rets;
        for(int i = 0; i < dirs.size(); i++)
        {
            auto dir {dirs[i]};
            std::cout << "Processing dir : " << dir << '\n';
            gEnv->SetValue("IterPath", dir.c_str());
            auto ret {Plotter(beam, target, light, ebeam, exs)};
            // Set title
            ret.mEffs->SetTitle(dir.c_str());
            ret.hKins->SetTitle(dir.c_str());
            ret.hExs->SetTitle(dir.c_str());
            ret.gsigmas->SetTitle(dir.c_str());
            // And push back
            rets.push_back(ret);
        }
        // Build a comparator (visual comparison of results)
        Comparator(rets);
    }
    if(what.Contains("y"))
    {
        Yield(beam, target, light, ebeam, yaml, Nb, Nt, Nit);
    }
}

void Comparator(const std::vector<RetPlot>& rets)
{
    // Kin + Efficiencies
    auto* c {new TCanvas {"cComp0", "Compare kin + effs"}};
    c->DivideSquare(rets.size() * 2);
    int p {1};
    for(int i = 0; i < rets.size(); i++)
    {
        auto& ret {rets[i]};
        c->cd(p);
        ret.hKins->Draw("colz");
        c->cd(p + rets.size());
        ret.mEffs->Draw("apl plc pmc");
        gPad->BuildLegend();
        p++;
    }
    // Ex + res
    c = new TCanvas {"cComp1", "Compare Ex + res"};
    c->DivideSquare(rets.size() * 2);
    p = 1;
    for(int i = 0; i < rets.size(); i++)
    {
        auto& ret {rets[i]};
        c->cd(p);
        ret.hExs->Draw();
        c->cd(p + rets.size());
        ret.gsigmas->Draw("apl");
        p++;
    }
}

#endif
