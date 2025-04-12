#pragma once
#include "Vector.h"

class FFrustum;

struct FCone
{
    FVector Apex;        // 원뿔의 꼭짓점
    FVector Direction;   // 원뿔의 축 (정규화되어야 함)
    float HalfAngle;     // 원뿔의 반각 (라디안)
    float TanHalfAngle;  // 반각의 탄젠트 (미리 계산)
    float Range;         // 원뿔의 최대 범위 (0이면 제한 없음)

    // 생성자: 주어진 값으로 초기화 (direction은 정규화되어 있다고 가정)
    FCone(const FVector &InApex, const FVector &InDirection, const float InHalfAngle, const float InRange)
      : Apex(InApex), Direction(InDirection), HalfAngle(InHalfAngle), Range(InRange)
    {
        TanHalfAngle = FMath::Tan(InHalfAngle);
    }

    float MaxXDotN(const FVector N) const
    {
        const float ApexDot = Apex.Dot(N);
        const float DirectionDot = Direction.Dot(N);
        const float R = Range * TanHalfAngle;
        const float BaseValue = ApexDot + Range * DirectionDot + R * FMath::Sqrt(FMath::Max(0.f, 1 - DirectionDot * DirectionDot));
        return FMath::Max(ApexDot, BaseValue);
    }
};
