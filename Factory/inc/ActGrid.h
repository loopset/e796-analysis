#ifndef ActGrid_h
#define ActGrid_h

#include "ActVoxel.h"

#include <boost/multi_array.hpp>
#include <tuple>
#include <vector>

namespace ActAlgorithm
{
class Grid
{
public:
    using AxisModel = std::tuple<int, float, float>;
    using CellContent = std::vector<ActRoot::Voxel>;

private:
    boost::multi_array<CellContent, 2> fGrid {};
    AxisModel fXaxis {};
    AxisModel fYaxis {};

public:
    Grid() = default;
    Grid(int nx, float xmin, float xmax, int ny, float ymin, float ymax);
    Grid(AxisModel xaxis, AxisModel yaxis);

    // Add point functions
    void AddVoxel(const ActRoot::Voxel& v);

    // Getter functions
    const CellContent& GetCellContent(unsigned int i, unsigned int j) const { return fGrid[i][j]; }
    int GetCellSize(unsigned int i, unsigned int j) const { return fGrid[i][j].size(); }
    bool IsCellEmpty(unsigned int i, unsigned int j) const { return fGrid[i][j].size() == 0; }
    AxisModel GetXaxis() const { return fXaxis; }
    int GetNbinsX() const { return std::get<0>(fXaxis); }
    AxisModel GetYaxis() const { return fYaxis; }
    int GetNbinsY() const { return std::get<0>(fYaxis); }
    bool IsInGrid(int dim, unsigned int idx) const;

    void Print() const;
    void Draw() const;

private:
    int FindCell(int dim, float val) const;
    float GetCellCenter(int dim, int idx) const;
    float GetCellWidth(int dim) const;
};
} // namespace ActAlgorithm

#endif // !ActGrid_h
