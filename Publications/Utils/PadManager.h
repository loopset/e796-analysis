#ifndef PadManager_h
#define PadManager_h

// forward declarations
#include <string>
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

    // Setters
    void SetXRight(double xr) { fxright = xr; }
    void SetXLeft(double xl) { fxleft = xl; }
    void SetYUp(double yp) { fyup = yp; }
    void SetYLow(double yl) { fylow = yl; }

    // Getters
    TCanvas* GetCanvas() const { return fCanv; }
    TPad* GetPad(int i, int j) const;
    TPad* GetPad(int idx) const;

    // Utilities
    void CenterXTitle();
    void SetMargins(int row, int col, double l = -1, double r = -1, double b = -1, double t = -1);
    void AddXTitle(double w, double h, const std::string& text, double x = 0.5, double ts = 0.75);
    void AddYTitle(double w, double h, const std::string& text, double y = 0.5, double ts = 0.75);
};
} // namespace PubUtils

#endif // !PadManager_h
