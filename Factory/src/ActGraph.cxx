#ifndef ActGraph_cxx
#define ActGraph_cxx

#include "../inc/ActGraph.h"

#include "Math/Point3D.h"
#include "Math/Vector3D.h"

#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/subgraph.hpp>
#include <boost/range/iterator_range_core.hpp>
#include <fstream>
#include <string>
#include <vector>

void ActAlgorithm::Graph::AddVertices()
{
    for(int i = 0; i < fGrid.GetNbinsX(); i++)
    {
        for(int j = 0; j < fGrid.GetNbinsY(); j++)
        {
            // If this is empty, continue
            if(fGrid.IsCellEmpty(i, j))
                continue;
            // Add vertices
            for(int p = 0; p < fGrid.GetCellSize(i, j); p++)
            {
                auto v {boost::add_vertex(fGraph)};
                fGraph[v].i = i;
                fGraph[v].j = j;
                fGraph[v].name = ("(" + std::to_string(i) + "," + std::to_string(j) + ")" + std::to_string(p));
                fGraph[v].v = fGrid.GetCellContent(i, j)[p];
                fVertices[i][j].push_back(v);
            }
        }
    }
}

void ActAlgorithm::Graph::AddEdges()
{
    auto range {boost::vertices(fGraph)};
    auto begin {range.first};
    auto end {range.second};

    for(auto i = begin; i != end; i++)
    {
        auto& vi {fGraph[*i]};
        for(auto j = i + 1; j != end; j++)
        {
            auto& vj {fGraph[*j]};
            auto dist {(vi.v.GetPosition() - vj.v.GetPosition()).R()};
            if(dist < 10)
            {
                if(!boost::edge(*i, *j, fGraph).second && !boost::edge(*j, *i, fGraph).second)
                    boost::add_edge(*i, *j, fGraph);
            }
        }
    }
}

int ActAlgorithm::Graph::GetNodeIndex(unsigned int i, unsigned int j, unsigned int idx)
{
    return i * 100000 + j * 100 + idx;
}

void ActAlgorithm::Graph::AddEdge(unsigned int i, unsigned int j, unsigned int m, unsigned int n)
{
    // Check neighour is in grid!
    auto isInGridX {fGrid.IsInGrid(0, m)};
    auto isInGridY {fGrid.IsInGrid(1, n)};
    if(!(isInGridX && isInGridY))
        return;
    // Check whether the neighbour is empty
    if(fGrid.GetCellContent(m, n).size() == 0)
        return;

    // Retrieve vertices
    const auto& current {fVertices[i][j]};
    const auto& neighour {fVertices[m][n]};
    for(int a = 0; a < current.size(); a++)
    {
        auto& vA {current[a]};
        for(int b = 0; b < neighour.size(); b++)
        {
            auto& vB {neighour[b]};
            if(!boost::edge(vA, vB, fGraph).second && !boost::edge(vB, vA, fGraph).second)
                boost::add_edge(vA, vB, fGraph);
        }
    }
}

void ActAlgorithm::Graph::BuildGraph()
{
    // for(int i = 0; i < fGrid.GetNbinsX(); i++)
    // {
    //     for(int j = 0; j < fGrid.GetNbinsY(); j++)
    //     {
    //         // If this is empty, continue
    //         if(fGrid.IsCellEmpty(i, j))
    //             continue;
    //         // Else, check neighours
    //         for(int ix = -1; ix <= 1; ix++) // x positions
    //         {
    //             for(int iy = -1; iy <= 1; iy++) // y positions
    //             {
    //                 if(ix == 0 && iy == 0) // skip self point
    //                     continue;
    //                 AddEdge(i, j, i + ix, j + iy); // top
    //             }
    //         }
    //     }
    // }
}

std::vector<ActAlgorithm::Graph::IndexPair> ActAlgorithm::Graph::GetConnectingCells()
{
    // Iterate over edges and return those that have
    std::vector<IndexPair> ret;
    for(auto vertex : boost::make_iterator_range(boost::vertices(fGraph)))
    {
        auto count {boost::out_degree(vertex, fGraph)};
        if(count >= 3)
        {
            auto& v {fGraph[vertex]};
            std::cout << "Junction : " << v.name << '\n';
            ret.push_back({v.i, v.j});
        }
    }
    return ret;
}

void ActAlgorithm::Graph::DrawToDot(const std::string& file)
{
    std::ofstream streamer {file};
    boost::write_graphviz(streamer, fGraph);
    streamer.close();
}

void ActAlgorithm::Graph::Print() const
{
    std::cout << "---- Graph ----" << '\n';
    for(auto vertex : boost::make_iterator_range(boost::vertices(fGraph)))
    {
        std::cout << "  Vertex : " << vertex << " has " << boost::out_degree(vertex, fGraph) << " edges" << '\n';
    }
    std::cout << "------------------" << '\n';
    for(auto edge : boost::make_iterator_range(boost::edges(fGraph)))
    {
        auto a {boost::source(edge, fGraph)};
        auto b {boost::target(edge, fGraph)};
        std::cout << "  Edge : " << a << " <--> " << b << '\n';
    }
}

void ActAlgorithm::Graph::FindCenter() {}

#endif // !ActGraph_cxx
