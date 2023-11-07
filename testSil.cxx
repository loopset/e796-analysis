#include "ActInputParser.h"

#include "Math/Point3D.h"
#include "Math/Point3Dfwd.h"
#include "Math/Vector3D.h"

#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <vector>
namespace ActCore
{
    class SilUnit
    {
    private:
        double fWidth {};
        double fHeight {};
        double fThick {};

    public:
        SilUnit() = default;
        SilUnit(double height, double width, double thick) : fHeight(height), fWidth(width), fThick(thick) {}
        inline void Print() const
        {
            std::cout << "---- SilUnit ----" << '\n';
            std::cout << "-> Height : " << fHeight << " mm" << '\n';
            std::cout << "-> Width  : " << fWidth << " mm" << '\n';
            std::cout << "-> Thick. : " << fThick << " mm" << '\n';
            std::cout << "--------------------" << '\n';
        }
    };

    class SilLayer
    {
    private:
        std::map<int, std::pair<double, double>> fCenters;
        std::map<int, double> fThresholds;
        SilUnit fUnit;

    public:
        SilLayer() = default;
        void ReadFile(const std::string& file);
    };
} // namespace ActCore

void ActCore::SilLayer::ReadFile(const std::string& file)
{
    ActRoot::InputParser parser {file};
    auto block {parser.GetBlock("f0")};
    std::vector<std::string> unitKeys {"Height", "Width", "Thickness"};
    std::vector<double> unitVals(unitKeys.size());
    for(int i = 0; i < unitKeys.size(); i++)
    {
        block->CheckTokenExists(unitKeys[i]);
        unitVals[i] = block->GetDouble(unitKeys[i]);
    }
    fUnit = SilUnit {unitVals[0], unitVals[1], unitVals[2]};
    fUnit.Print();
}

void testSil()
{
    ActCore::SilLayer layer {};
    layer.ReadFile("./configs/silicons.detector");
}
