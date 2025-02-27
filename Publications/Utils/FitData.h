#ifndef FitData_h
#define FitData_h

#include "VPlotData.h"

#include <string>
#include <vector>

// forward declarations
class TH1D;
class TGraph;
class TList;
class TLegend;
class THStack;

namespace PubUtils
{
class FitData : public VPlotData
{
private:
    // Holding pointer to objects read from .root
    TH1D* fHist {};
    TGraph* fGlobal {};
    std::vector<std::string> fPeakNames {};
    TList* fPeaks {};
    THStack* fStack {};
    TLegend* fLegend {};

public:
    FitData() = default;
    FitData(const std::string& file) { Parse(file); }

    void Parse(const std::string& file, const std::string& key = "") override;
    void SetOpts(Opts opts) override;
    void Draw() override;
    void Print() const override;

private:
    void BuildStack();
};
} // namespace PubUtils

#endif // !FitData_h
