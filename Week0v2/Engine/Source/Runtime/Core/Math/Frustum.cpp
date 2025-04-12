#include "Frustum.h"

#include "Cone.h"
#include "Matrix.h"

void FFrustum::ExtractPlanes(const FMatrix& viewProj)
{
    // Left Plane = row4 + row1
    Planes[0].A = viewProj.M[3][0] + viewProj.M[0][0];
    Planes[0].B = viewProj.M[3][1] + viewProj.M[0][1];
    Planes[0].C = viewProj.M[3][2] + viewProj.M[0][2];
    Planes[0].D = viewProj.M[3][3] + viewProj.M[0][3];
    Planes[0].Normalize();

    // Right Plane = row4 - row1
    Planes[1].A = viewProj.M[3][0] - viewProj.M[0][0];
    Planes[1].B = viewProj.M[3][1] - viewProj.M[0][1];
    Planes[1].C = viewProj.M[3][2] - viewProj.M[0][2];
    Planes[1].D = viewProj.M[3][3] - viewProj.M[0][3];
    Planes[1].Normalize();

    // Top Plane = row4 - row2
    Planes[2].A = viewProj.M[3][0] - viewProj.M[1][0];
    Planes[2].B = viewProj.M[3][1] - viewProj.M[1][1];
    Planes[2].C = viewProj.M[3][2] - viewProj.M[1][2];
    Planes[2].D = viewProj.M[3][3] - viewProj.M[1][3];
    Planes[2].Normalize();

    // Bottom Plane = row4 + row2
    Planes[3].A = viewProj.M[3][0] + viewProj.M[1][0];
    Planes[3].B = viewProj.M[3][1] + viewProj.M[1][1];
    Planes[3].C = viewProj.M[3][2] + viewProj.M[1][2];
    Planes[3].D = viewProj.M[3][3] + viewProj.M[1][3];
    Planes[3].Normalize();

    // Near Plane = row4 + row3
    Planes[4].A = viewProj.M[3][0] + viewProj.M[2][0];
    Planes[4].B = viewProj.M[3][1] + viewProj.M[2][1];
    Planes[4].C = viewProj.M[3][2] + viewProj.M[2][2];
    Planes[4].D = viewProj.M[3][3] + viewProj.M[2][3];
    Planes[4].Normalize();

    // Far Plane = row4 - row3
    Planes[5].A = viewProj.M[3][0] - viewProj.M[2][0];
    Planes[5].B = viewProj.M[3][1] - viewProj.M[2][1];
    Planes[5].C = viewProj.M[3][2] - viewProj.M[2][2];
    Planes[5].D = viewProj.M[3][3] - viewProj.M[2][3];
    Planes[5].Normalize();
}

bool FFrustum::CheckSphere(const FVector& InCenter, const float InRadius) const
{
    for (int i = 0; i < 6; ++i)
    {
        if (Planes[i].Distance(InCenter) < -InRadius)
        {
            return false;
        }
    }
    return true;
}


bool FFrustum::IntersectFrustumCone(const FCone& InCone)
{
    // for each plane in the frustum
    for (int i = 0; i < 6; ++i) {
        // 평면의 법선 (N)와 d
        const FVector N = { Planes[i].A, Planes[i].B, Planes[i].C };
        const float D = Planes[i].D; // 평면 방정식: N dot X + d = 0, 내부면: >=0
        
        // Cone의 support 값을 N 방향으로 계산
        const float support = InCone.MaxXDotN(N);
        
        // 만약 support < -d, 그러면 모든 점 X in Cone에 대해 N dot X < -d 이므로,
        // 프러스텀의 해당 평면 밖에 있음
        if (support < -D)
            return false;
    }
    return true;
}

