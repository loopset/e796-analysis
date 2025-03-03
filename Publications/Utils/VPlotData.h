#ifndef VPlotData_h
#define VPlotData_h

#include "TAttLine.h"

#include <algorithm>
#include <any>
#include <map>
#include <string>
#include <vector>

namespace PubUtils
{
class VPlotData
{
public:
    using Opts = std::map<std::string, std::any>;
    using Labels = std::map<std::string, std::string>;
    using Styles = std::map<std::string, TAttLine>;

public:
    VPlotData() = default;
    virtual ~VPlotData() = default;

    virtual void Parse(const std::string& file, const std::string& key = "") {}
    virtual void SetOpts(Opts opts) {}
    virtual void Draw() {}
    virtual void Print() const {}

    inline static void SortNames(std::vector<std::string>& names)
    {
        std::sort(names.begin(), names.end(),
                  [](const std::string& a, const std::string& b)
                  {
                      auto sortType = [](const std::string& s)
                      {
                          auto it {s.find_first_of("0123456789")};
                          auto type {s.substr(0, it)};
                          if(type == "g")
                              return 0;
                          if(type == "v")
                              return 1;
                          if(type == "ps")
                              return 2;
                          if(type == "cte")
                              return 3;
                          return 4; // default in case unknown type (never happens, guaranteed by CheckInit)
                      };
                      int rankA = sortType(a), rankB = sortType(b);
                      if(rankA != rankB)
                          return rankA < rankB;
                      auto ia {std::stoi(a.substr(a.find_first_of("0123456789")))};
                      auto ib {std::stoi(b.substr(b.find_first_of("0123456789")))};
                      return ia < ib;
                  });
    }
};
} // namespace PubUtils

#endif // !VPlotData_h
