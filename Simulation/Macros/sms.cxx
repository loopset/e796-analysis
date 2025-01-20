// Compare Juans and my SM
#include "ActSilMatrix.h"

#include "TAttLine.h"
#include "TCanvas.h"

#include <fstream>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "../../PostAnalysis/Utils.cxx"
void sms()
{
    std::string target {"2H"};
    std::string light {"3H"};

    // Parse Juan file
    std::fstream streamer;
    if(target == "2H" && light == "3H")
        streamer.open("/media/miguel/FICA_4/Juan/Sim_d4He/Contours/Contours_Veto_v3.txt");
    std::map<int, std::vector<double>> map;
    int idx {};
    double a, b, c, d;
    while(streamer >> idx >> a >> b >> c >> d)
        map[idx] = {a, b, c, d};

    auto* juan {new ActPhysics::SilMatrix};
    for(const auto& [idx, vec] : map)
    {
        std::pair<double, double> x {vec[0], vec[1]};
        std::pair<double, double> y {vec[2], vec[3]};
        juan->AddSil(idx, x, y);
    }

    // Mine
    auto* sm {E796Utils::GetEffSilMatrix(target, light)};

    // Centre juans around mine
    juan->MoveZTo(sm->GetMeanZ({3, 4}), {3, 4});

    // Draw
    auto* c0 {new TCanvas {"c0", "SM comparison"}};
    sm->Draw(false);
    juan->SetSyle(false, kDashed);
    juan->Draw();

    // Write juan to test!
    juan->Write("./juan_veto.root");
}
