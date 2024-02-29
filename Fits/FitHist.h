#ifndef FitHist_h
#define FitHist_h
#include "ROOT/RDF/HistoModels.hxx"

#include "TString.h"
namespace E796Fit
{
// (p,d) settings
const ROOT::RDF::TH1DModel Expd {
    "hEx", TString::Format("^{20}O(p,d);E_{x} [MeV];Counts / %.0f", (6. - (-10.)) / 100 * 1000), 100, -10, 6};
// (d,t) settings
const ROOT::RDF::TH1DModel Exdt {
    "hEx", TString::Format("^{20}O(d,t);E_{x} [MeV];Counts / %.0f", (25. - (-5.)) / 100 * 1000), 100, -5, 25};
} // namespace E796Fit
#endif
