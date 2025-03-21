#include "ActClIMB.h"
#include "ActCluster.h"
#include "ActLine.h"
#include "ActTPCData.h"
#include "ActTPCParameters.h"
#include "ActVoxel.h"

#include "Rtypes.h"

#include "TCanvas.h"
#include "TFile.h"
#include "TH2.h"
#include "TPolyMarker.h"
#include "TTree.h"

#include <boost/graph/biconnected_components.hpp>
#include <memory>
#include <queue>
#include <string>
#include <vector>

// Include grid
#include "./inc/ActGraph.h"
#include "./inc/ActGrid.h"
#include "./src/ActGraph.cxx"
#include "./src/ActGrid.cxx"

ActRoot::TPCData* read(const std::string& file)
{
    auto f {std::make_unique<TFile>(file.c_str())};
    return f->Get<ActRoot::TPCData>("TPCData");
}

void draw_id(TCanvas* c, TH2* h, ActRoot::TPCData* data)
{
    c->cd(1);
    h->Reset();
    h->SetStats(false);
    // Fill
    for(const auto& cl : data->fClusters)
    {
        for(const auto& v : cl.GetVoxels())
        {
            auto x {v.GetPosition().X()};
            auto y {v.GetPosition().Y()};
            h->SetBinContent(x, y, v.GetCharge());
        }
    }
    h->Draw("colz");
}

void draw_grid(TCanvas* c, TH2* h, ActAlgorithm::Grid* g)
{
    c->cd(2);
    h->Reset();
    h->SetStats(false);
    // Fill
    int id {1};
    for(int i = 0; i < g->GetNbinsX(); i++)
    {
        for(int j = 0; j < g->GetNbinsY(); j++)
        {
            if(!g->IsCellEmpty(i, j))
            {
                for(const auto& v : g->GetCellContent(i, j))
                {
                    h->SetBinContent(v.GetPosition().X(), v.GetPosition().Y(), id);
                }
                id++;
            }
        }
    }
    h->Draw("colz");
}

void draw_edges(TCanvas* c, ActAlgorithm::Grid* g)
{
    c->cd(2);
    auto* poly {new TPolyMarker};
    for(int i = 0; i < g->GetNbinsX(); i++)
    {
        for(int j = 0; j < g->GetNbinsY(); j++)
        {
            if(!g->IsCellEmpty(i, j))
            {
                for(const auto& point : g->GetCellContent(i, j))
                {
                    poly->SetNextPoint(point.GetPosition().X(), point.GetPosition().Y());
                }
            }
        }
    }
    poly->SetMarkerColor(kRed);
    poly->SetMarkerStyle(24);
    poly->Draw("same");
}

int main()
{
    // Read data
    auto data {read("./Inputs/3body.root")};

    // TCanvas to draw
    auto* c0 {new TCanvas {"c0", "Factory canvas"}};
    c0->DivideSquare(2);
    // Histograms to draw
    auto* hID {new TH2D {"hID", "Pad canvas", 128, 0, 128, 128, 0, 128}};
    // Draw id
    draw_id(c0, hID, data);

    // Define grid
    ActAlgorithm::Grid g0 {14, 0, 128, 7, 0, 128};
    auto* hGrid {new TH2D {"hGrid", "Grid canvas", 128, 0, 128, 128, 0, 128}};
    for(const auto& cl : data->fClusters)
    {
        for(const auto& v : cl.GetVoxels())
        {
            // Project into 2d
            ActRoot::Voxel point {{v.GetPosition().X(), v.GetPosition().Y(), 0}, v.GetCharge()};
            g0.AddVoxel(point);
        }
    }
    g0.Print();

    // Run cluster in that
    ActAlgorithm::ClIMB cont;
    ActRoot::TPCParameters tpc {"Actar"};
    // Reduce dimensionality in Z
    tpc.SetNPADSZ(1);
    cont.SetTPCParameters(&tpc);
    cont.SetMinPoint(1);
    cont.Print();
    // Cluster and save
    ActAlgorithm::Grid g1 {g0.GetXaxis(), g0.GetYaxis()};
    for(int i = 0; i < g0.GetNbinsX(); i++)
    {
        for(int j = 0; j < g0.GetNbinsY(); j++)
        {
            if(g0.IsCellEmpty(i, j))
                continue;
            // Cluster
            auto [clusters, _] {cont.Run(g0.GetCellContent(i, j))};
            // Fill
            for(const auto& cl : clusters)
            {
                ActRoot::Line line;
                line.FitVoxels(cl.GetVoxels(), true, false);
                ActRoot::Voxel point {line.GetPoint(), 0};
                g1.AddVoxel(point);
            }
        }
    }

    // Draw it
    draw_grid(c0, hGrid, &g0);
    draw_edges(c0, &g1);


    // Build graph
    ActAlgorithm::Graph graph {g1};
    graph.GetConnectingCells();
    // graph.DrawToDot("./test.dot");
    graph.Print();

    // // Find articulation points using Boost's built-in function
    // std::vector<boost::graph_traits<ActAlgorithm::Graph::GraphType>::vertex_descriptor> articulation_points;
    // boost::articulation_points(graph.GetGraph(), std::back_inserter(articulation_points));
    //
    // // Print the articulation points (junctions)
    // std::cout << "Articulation points (junctions):\n";
    // for(auto v : articulation_points)
    // {
    //     std::cout << "Vertex " << v << " (" << graph.GetGraph()[v].name << ")\n";
    // }
    int max_degree = -1;
    boost::graph_traits<ActAlgorithm::Graph::GraphType>::vertex_descriptor vertex_with_max_edges;

    for(auto v : boost::make_iterator_range(boost::vertices(graph.GetGraph())))
    {
        int degree = boost::degree(v, graph.GetGraph()); // Get the degree of the vertex
        if(degree > max_degree)
        {
            max_degree = degree;
            vertex_with_max_edges = v;
        }
    }

    std::cout << "\nVertex with the most edges:\n";
    std::cout << "Vertex " << vertex_with_max_edges << " (" << graph.GetGraph()[vertex_with_max_edges].name
              << "), Degree: " << max_degree << std::endl;
    graph.GetGraph()[vertex_with_max_edges].v.Print();
}
