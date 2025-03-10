#include "TGraphErrors.h"
#include "TVirtualPad.h"

#include "AngComparator.h"

#include <map>
#include <string>

void plot()
{
    auto* g {new TGraphErrors {"./elastic.dat", "%lg %lg"}};

    Angular::Comparator comp {"E.Khan g.s", g};
    comp.Add("BG", "../Inputs/g0_Khan/fort.201");
    comp.Fit();
    comp.Draw("", true);

    // Compare theoretical calculations
    Angular::Comparator theo {"Theoretical", nullptr};
    std::map<std::string, std::string> theos {{"g.s", "../Inputs/g0_Khan/fort.201"},
                                              {"g.s from iblock=2", "../Inputs/g1_Khan/fort.201"}};
    for(const auto& [key, file] : theos)
        theo.Add(key, file);
    theo.DrawTheo();
    gPad->SetLogy();
}
