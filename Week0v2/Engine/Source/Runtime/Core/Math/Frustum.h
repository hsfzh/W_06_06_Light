#pragma once
#include "FPlane.h"

struct FCone;
struct FMatrix;

class FFrustum
{
public:
    FPlane Planes[6];

    void ExtractPlanes(const FMatrix& InViewProj);

    bool CheckSphere(const FVector& InCenter, float InRadius) const;
    
    bool IntersectFrustumCone(const FCone& InCone);
};
