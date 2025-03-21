#ifndef ActGraph_h
#define ActGraph_h

#include "ActGrid.h"
#include "ActVoxel.h"

#include <boost/graph/adjacency_list.hpp>
#include <boost/multi_array.hpp>
#include <string>
#include <vector>

namespace ActAlgorithm
{
class Graph
{
public:
    struct VertexData
    {
        int i {};
        int j {};
        std::string name {};
        ActRoot::Voxel v {};
    };
    using GraphType = boost::adjacency_list<boost::listS, boost::vecS, boost::undirectedS, VertexData>;
    using Vertex = boost::graph_traits<GraphType>::vertex_descriptor;
    using IndexPair = std::pair<unsigned int, unsigned int>;

private:
    const Grid& fGrid {};
    GraphType fGraph {};
    boost::multi_array<std::vector<Vertex>, 2> fVertices {};


public:
    Graph() = default;
    Graph(const Grid& grid) : fGrid(grid)
    {
        fVertices.resize(boost::extents[fGrid.GetNbinsX()][fGrid.GetNbinsY()]);
        AddVertices();
        AddEdges();
        BuildGraph();
    }

    // Build graph
    void AddVertices();
    void AddEdges();
    void BuildGraph();
    int GetNodeIndex(unsigned int i, unsigned int j, unsigned int idx);
    std::vector<IndexPair> GetConnectingCells();
    void FindCenter();

    // Getters
    const GraphType& GetGraph() const { return fGraph; }

    void DrawToDot(const std::string& file);
    void Print() const;


private:
    void AddEdge(unsigned int i, unsigned int j, unsigned int m, unsigned int n);
};
} // namespace ActAlgorithm

#endif // !ActGraph_h
