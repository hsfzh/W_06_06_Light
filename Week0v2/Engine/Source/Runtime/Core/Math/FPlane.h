#pragma once
#include "Vector.h"

struct FPlane
{
    float A, B, C, D;

    // 평면 정규화: (a, b, c) 법선 벡터의 길이로 나눕니다.
    void Normalize()
    {
        const float len = FMath::Sqrt(A * A + B * B + C * C);
        if (len > 0.00001f)
        {
            A /= len;
            B /= len;
            C /= len;
            D /= len;
        }
    }
    
    // 평면과 3D 점 사이의 거리: (ax + by + cz + d)
    float Distance(const FVector& pt) const
    {
        return A * pt.x + B * pt.y + C * pt.z + D;
    }
};

