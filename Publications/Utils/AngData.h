#ifndef AngData_h
#define AngData_h

#include "VPlotData.h"

#include <string>
#include <vector>

// forward declarations
class TGraphErrors;
class TMultiGraph;
class TLegend;
namespace PhysUtils
{
class SFCollection;
}

namespace PubUtils
{
class AngData : public VPlotData
{
private:
    TMultiGraph* fMulti {};
    std::vector<TGraphErrors*> fGraphs {};
    PhysUtils::SFCollection* fSFs {};


public:
    AngData() = default;
    AngData(const std::string& file, const std::string& key) { Parse(file, key); }

    void Parse(const std::string& file, const std::string& key) override;
    void SetText(double x, double y, const std::string& text, double w = 0.4, double h = 0.2);
    void DisableLabel(const std::string& axis, int idx);
    void SetNDiv(const std::string& axis, int ndiv);
    void SetGraphsStyle(const VPlotData::Styles& styles);
    void SetRangeY(double ymin = -11, double ymax = -11);
    void CenterY(double factor = 1.2);
    void SetOpts(Opts opts) override;
    void Draw() override;
    void Print() const override;

    // Some static utility functions
    static void GetLegendGraph(TLegend* leg, TAttLine st, const std::string& name);

private:
    void SetMulti(TMultiGraph* mg);
};
} // namespace PubUtils

#endif // !AngData_h
