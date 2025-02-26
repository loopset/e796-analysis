#ifndef PadManager_h
#define PadManager_h

// forward declarations
#include <vector>
class TCanvas;
class TPad;

namespace PubUtils
{
class PadManager
{
private:
    TCanvas* fCanv {};
    std::vector<TPad*> fPads {};
    // Gaps between pads
    double fxright {0.005};
    double fxleft {0.005};
    double fyup {0.005};
    double fylow {0.005};

public:
    PadManager() = default;
    PadManager(int npads) { Init(npads); }

    void Init(int npads); //!< Main method
    TCanvas* GetCanvas() const { return fCanv; }
    TPad* GetPad(int i, int j) const;
    TPad* GetPad(int idx) const;
    void CenterXTitle();
};
} // namespace PubUtils

#endif // !PadManager_h
