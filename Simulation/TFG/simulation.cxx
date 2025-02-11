#include "ActKinematics.h"
#include "ActSRIM.h"
#include "ActSilSpecs.h"
#include "ActTPCParameters.h"

#include "TH1.h"
#include "TRandom.h"
#include "TString.h"

#include "Math/Point3D.h"
#include "Math/Vector3D.h"

using XYZPoint = ROOT::Math::XYZPoint;
using XYZVector = ROOT::Math::XYZVector;

// Define funcións para facer o que queres
// E ponlles nomes claros para entender ben o código
XYZPoint SampleVertex()
{
    // Implementa ti a función
    XYZPoint vertex {};
    return vertex;
}

double ApplySilResolution(double deltaE)
{
    // Toma o mesmo valor de sigma que das prácticas
    // (probablemente depende de E)
    return gRandom->Gaus(deltaE, 0.5);
}

double ApplyAngleResolution(double thetaLab)
{
    // Define a sigma apropiada: 1 deg FWHM
    return gRandom->Gaus(thetaLab, 0.5);
}

// Esta é a función principal. recorda que se executa dende unha terminal:
// $ root -l simulation.cxx (.q para saír)
// Vamos facer o algoritmo versátil por se no futuro tes que recorrer a el:
// beam = str que identifica o beam, 11Li
// target = str que identifica o target, 2H neste caso
// light = str para a partícula lixeira
// Ex = double que especifica o valor de enerxía de excitación
void simulation(std::string beam = "11Li", std::string target = "2H", std::string = "3H", double Ex = 0)
{
    // Define o número de iteracións
    // (aumenta unha vez esteas seguro de que funciona)
    int niter {static_cast<int>(1e5)};

    // Define os detectores
    // TPC
    ActRoot::TPCParameters tpc {"Actar"};
    // Tamanhos en mm:
    auto x {tpc.X()}; // o mesmo para Y,Z
    // Silicios
    auto* sils {new ActPhysics::SilSpecs};
    // Están especificados nun ficheiro de configuración
    sils->ReadFile("./Inputs/silicons.conf");
    // Podes comprobar que funciona
    sils->DrawGeo();

    // Le as táboas de SRIM, adecuadas para cada isótopo
    auto* srim {new ActPhysics::SRIM};
    srim->ReadTable("beam", TString::Format("path/to/%s_mixture.txt", beam.c_str()).Data());
    // igual para light (en gas) e lighInSil (no silicio)

    // Cinemática
    auto* kin {new ActPhysics::Kinematics {"11Li(d,t)@82.5"}};

    // Define aqui os histogramas que consideres necesarios
    auto* hEx {new TH1D {"hEx", "Titulo;Titulo X;Titulo Y", 200, -5, 15}};

    // E iteramos
    for(int it = 0; it < niter; it++)
    {
        /*
        1- Samplea vertice
        2- Reduce enerxía beam
        3- Chama á Cinemática
        4- Comproba se a partícula chega a un silicio:
        */
        auto vertex {SampleVertex()};
        // Para calcular a cinemática. Samplea thetaCM e phiCM
        double TBeamVertex {}; // o beam perde enerxia ata o vértice. aplica SRIM
        double thetaCM {};     // uniforme de momento
        double phiCM {};       // uniforme sempre
        // Configura a Tbeam da iteración
        kin->SetBeamEnergy(TBeamVertex);
        // E calcula a cinemática á saída
        kin->ComputeRecoilKinematics(thetaCM, phiCM);
        // E obtén as variables no LAB
        kin->GetT3Lab();
        kin->GetTheta3Lab();
        kin->GetPhi3Lab();
        // Xa podes definir a dirección (recorda implementar a incerteza en theta3lab)
        XYZVector direction {};

        // Para comprobar que a partícula chega a un silicio:
        // 1o argumento: nome da layer á que queres ver se chegas. a primeira layer é "f0"
        // 2o vértice
        // 3o dirección
        // Para a dirección ten en conta que en ACTAR TPC traballamos con X <--> Z.
        // O eixo de propagación do beam é X
        // Lembra que a dirección é o vector unitario r en coordenadas ESFÉRICAS
        auto [silIndex0, silPoint0] {sils->FindSPInLayer("f0", vertex, direction)};
        // Se silIndex == -1 -> NON HOUBO IMPACTO
        // Se silIndex != 1 -> SI houbo impacto, no punto silPoint0

        // E o resto é igual ao das prácticas, só que coa partícula lixeira (light)
        // Usa a táboa "light" para o gas e "lightInSil" para o silicio
    }

    // Aqui crea o(s) TCanvas e debuxa os histogramas en diferentes pads.
}
