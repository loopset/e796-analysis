#include "ActSilMatrix.h"

#include "TAttLine.h"
#include "TCanvas.h"
#include "TLine.h"
void CompareSilMatrix()
{
    ActPhysics::SilMatrix sma;// using all data
    sma.Read("./silmatrix.root");
    sma.SetSyle(true, kSolid, 2, 0);

    ActPhysics::SilMatrix sm0;// using data RP.X() < 256. / 3
    sm0.Read("./silmatrix_part0.root");
    sm0.SetSyle(false, kDashed, 4, 0);

    ActPhysics::SilMatrix sm1;// using data RP.X() > 256. * 2. / 3
    sm1.Read("./silmatrix_part1.root");
    sm1.SetSyle(false, kDotted, 4, 0);

    // plot
    auto* cm {new TCanvas("cm", "Sil matrix canvas")};
    sma.Draw();
    sm0.Draw(true);
    sm1.Draw(true);
}
