#ifndef ActGrid_cxx
#define ActGrid_cxx

#include "../inc/ActGrid.h"


ActAlgorithm::Grid::Grid(int nx, float xmin, float xmax, int ny, float ymin, float ymax)
    : fXaxis(nx, xmin, xmax),
      fYaxis(ny, ymin, ymax)
{
    fGrid.resize(boost::extents[nx][ny]);
}

ActAlgorithm::Grid::Grid(AxisModel xaxis, AxisModel yaxis) : fXaxis(xaxis), fYaxis(yaxis)
{
    fGrid.resize(boost::extents[std::get<0>(fXaxis)][std::get<0>(fYaxis)]);
}

int ActAlgorithm::Grid::FindCell(int dim, float val) const
{
    // Get axis info
    int n {};
    float min {};
    float max {};
    if(dim == 0)
        std::tie(n, min, max) = fXaxis;
    if(dim == 1)
        std::tie(n, min, max) = fYaxis;
    // Compute bin size
    auto size {(max - min) / n};
    if(val < min || val >= max)
        return -1; // sentinel of invalid bin
    else
    {
        return static_cast<int>((val - min) / size);
    }
}

float ActAlgorithm::Grid::GetCellCenter(int dim, int idx) const
{
    int n {};
    float min {};
    float max {};
    if(dim == 0)
        std::tie(n, min, max) = fXaxis;
    if(dim == 1)
        std::tie(n, min, max) = fYaxis;
    // Bin size
    auto size {(max - min) / n};
    return min + (idx + 0.5f) * size;
}

float ActAlgorithm::Grid::GetCellWidth(int dim) const
{
    int n {};
    float min {};
    float max {};
    if(dim == 0)
        std::tie(n, min, max) = fXaxis;
    if(dim == 1)
        std::tie(n, min, max) = fYaxis;
    // Bin size
    return (max - min) / n;
}


void ActAlgorithm::Grid::AddVoxel(const ActRoot::Voxel& v)
{
    // X
    auto x {FindCell(0, v.GetPosition().X())};
    // Y
    auto y {FindCell(1, v.GetPosition().Y())};
    if((x != -1) && (y != -1))
        fGrid[x][y].push_back(v);
}

void ActAlgorithm::Grid::Print() const
{
    for(int i = 0; i < fGrid.shape()[0]; i++)
    {
        for(int j = 0; j < fGrid.shape()[1]; j++)
        {
            if(IsCellEmpty(i, j))
                continue;
            std::cout << "[" << i << ", " << j << "] : " << GetCellSize(i, j) << '\n';
        }
    }
}

void ActAlgorithm::Grid::Draw() const
{
    // auto wx {GetCellWidth(0)};
    // auto wy {GetCellWidth(1)};
    //
    // // Multigraph
    // auto* mg {new TMultiGraph};
    // for(int i = 0; i < GetNbinsX(); i++)
    // {
    //     for(int j = 0; j < GetNbinsY(); )
    // }
}

bool ActAlgorithm::Grid::IsInGrid(int dim, unsigned int idx) const
{
    int size {};
    if(dim == 0)
        size = fGrid.shape()[0];
    if(dim == 1)
        size = fGrid.shape()[1];
    return 0 <= idx && idx < size;
}

#endif
