#include "TMatrix.h"
#include "TMatrixD.h"

#include "Math/Point3D.h"
#include "Math/Vector3D.h"

#include <iostream>

using XYZPoint = ROOT::Math::XYZPointF;
using XYZVector = ROOT::Math::XYZVectorF;

void testDecompose()
{
    XYZPoint pa {4.25, 64, 128};
    XYZPoint pb {28, 64, 128};

    XYZVector a {1, 0., 0};
    XYZVector b {1, 0, 0};
    std::cout << "a.b : " << a.Dot(b) << '\n';
    auto c {b.Cross(a)};
    std::cout << "C : " << c << '\n';

    TMatrixD left {3, 3};

    XYZVector vecs[3] {a, -b, c};
    for(int col = 0; col < 3; col++)
    {
        double components[3] {};
        vecs[col].GetCoordinates(components);
        for(int row = 0; row < 3; row++)
            left[row][col] = components[row];
    }
    TMatrixD right {3, 1};
    auto diff {pb - pa};
    double components[3] {};
    diff.GetCoordinates(components);
    right.SetMatrixArray(components);
    left.Print();
    right.Print();
    // 4-> Invert left to solve system
    TMatrixD invLeft {TMatrixD::kInverted, left};
}
