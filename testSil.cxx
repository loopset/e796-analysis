#include "ActColors.h"
#include "ActInputParser.h"

#include "Math/Point3D.h"
#include "Math/Point3Dfwd.h"
#include "Math/Vector3D.h"
#include "Math/Vector3Dfwd.h"

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
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
            std::cout << "...................." << '\n';
            std::cout << "-> Height : " << fHeight << " mm" << '\n';
            std::cout << "-> Width  : " << fWidth << " mm" << '\n';
            std::cout << "-> Thick. : " << fThick << " mm" << '\n';
            std::cout << "...................." << '\n';
        }
    };

    class SilLayer
    {
    public:
        using XYZPointD = ROOT::Math::XYZPointD;
        using XYZVectorD = ROOT::Math::XYZVectorD;

    private:
        std::map<int, std::pair<double, double>> fPlacements;
        std::map<int, double> fThresholds;
        SilUnit fUnit;
        XYZPointD fPoint;   //!< Point of layer: basically, contains offset
        XYZVectorD fNormal; //!< Normal vector of silicon plane

    public:
        SilLayer() = default;
        void ReadConfiguration(std::shared_ptr<ActRoot::InputBlock> block);
        inline void Print() const
        {
            std::cout << "--------------------" << '\n';
            std::cout << "-> idx : <X || Y, Z>" << '\n';
            for(const auto& [idx, pair] : fPlacements)
                std::cout << "   " << idx << " : <" << pair.first << ", " << pair.second << ">" << '\n';
            std::cout << "-> Point  : " << fPoint << " [pads]" << '\n';
            std::cout << "-> Normal : " << fNormal << '\n';
            std::cout << "-> Unit   : " << '\n';
            fUnit.Print();
            std::cout << "--------------------" << '\n';
        }

        // Getters and setters
        const std::map<int, std::pair<double, double>>& GetPlacements() const { return fPlacements; }
        const std::map<int, double>& GetThresholds() const { return fThresholds; }
        const SilUnit& GetUnit() const { return fUnit; }
        const XYZPointD& GetPoint() const { return fPoint; }
        const XYZVectorD& GetNormal() const { return fNormal; }
    };

    class SilGeo
    {
    private:
        std::unordered_map<std::string, SilLayer> fLayers;

    public:
        void ReadFile(const std::string& file);
        void Print() const;
        const SilLayer& GetLayer(const std::string& name) {return fLayers[name];}
    };
} // namespace ActCore

void ActCore::SilGeo::ReadFile(const std::string& file)
{
    ActRoot::InputParser parser {file};
    for(const auto& name : parser.GetBlockHeaders())
    {
        SilLayer layer;
        layer.ReadConfiguration(parser.GetBlock(name));
        fLayers[name] = layer;
    }
}

void ActCore::SilLayer::ReadConfiguration(std::shared_ptr<ActRoot::InputBlock> block)
{
    std::vector<std::string> unitKeys {"Height", "Width", "Thickness"};
    std::vector<double> unitVals(unitKeys.size());
    for(int i = 0; i < unitKeys.size(); i++)
    {
        block->CheckTokenExists(unitKeys[i]);
        unitVals[i] = block->GetDouble(unitKeys[i]);
    }
    fUnit = SilUnit {unitVals[0], unitVals[1], unitVals[2]};
    // Get placements
    auto allKeys {block->GetAllReadValues()};
    for(const auto& [key, vals] : allKeys)
    {
        if(!(key.rfind("i", 0) == 0)) // check this is an i-like command to add placements
            continue;
        auto idx {std::stoi(key.substr(1))};
        auto pair {block->GetDoubleVector(key)};
        fPlacements[idx] = {pair[0], pair[1]};
    }
    // Add offset and normal vector
    // 1-> Point aka offset
    if(block->CheckTokenExists("Point"))
    {
        auto aux {block->GetDoubleVector("Point")};
        fPoint = {aux[0], aux[1], aux[2]};
    }
    // 2-> Plane normal
    if(block->CheckTokenExists("Normal"))
    {
        auto aux {block->GetDoubleVector("Normal")};
        fNormal = {aux[0], aux[1], aux[2]};
    }
}

void ActCore::SilGeo::Print() const
{
    std::cout << BOLDYELLOW << "++++++++ SilGeo ++++++++" << '\n';
    for(const auto& [name, layer] : fLayers)
    {
        std::cout << "-> Layer : " << name << '\n';
        layer.Print();
        std::cout << std::endl;
    }
}

void testSil()
{
    ActCore::SilGeo silSpecs {};
    silSpecs.ReadFile("./configs/silicons.detector");
    silSpecs.Print();
}
