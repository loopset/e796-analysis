#include "ActMergerData.h"

#include "ROOT/RDF/RInterface.hxx"
#include "ROOT/RDataFrame.hxx"

#include <cstdlib>
#include <fstream>
#include <utility>

#include "../../Selector/Selector.h"
void get_events()
{
    // Read processes dataset
    ROOT::RDataFrame df {"Sel_Tree", gSelector->GetAnaFile(3, "20O", "2H", "3H")};

    // Plot phi of particle
    auto hPhi {df.Histo1D("fPhiLight")};

    // And write entries
    std::pair<double, double> phiCut {80, 110};
    std::ofstream streamer {"./events.dat"};
    df.Foreach(
        [&](ActRoot::MergerData& d)
        {
            auto phi {std::abs(d.fPhiLight)};
            auto rpx {d.fRP.X()};
            if((phiCut.first <= phi && phi <= phiCut.second) && (100 <= rpx && rpx <= 130))
                d.Stream(streamer);
        },
        {"MergerData"});
    streamer.close();

    hPhi->DrawClone();
}
