#pragma once
#include "UBillboardComponent.h"

struct FParticleSubUVCompInfo : public FBillboardComponentInfo
{
    DECLARE_ACTORCOMPONENT_INFO(FParticleSubUVCompInfo);
    float ScaleU = 0;
    float ScaleV = 0;
    float OffsetU = 0;
    float OffsetV = 0;
    int indexU = 0;
    int indexV = 0;
    float Second = 0;
    
    // 프레임 당 지속 시간 (밀리초 단위, 필요에 따라 조정)
    float FrameDuration = 75.0f;

    int CellsPerRow;
    int CellsPerColumn;

    FParticleSubUVCompInfo()
        : FBillboardComponentInfo()
        , CellsPerRow(1)
        , CellsPerColumn(1)
    {
        InfoType = TEXT("FParticleSubUVCompInfo");
        ComponentType = TEXT("UParticleSubUVComp");
    }

    virtual void Copy(FActorComponentInfo& Other) override
    {
        FBillboardComponentInfo::Copy(Other);
        FParticleSubUVCompInfo& OtherSubUV = static_cast<FParticleSubUVCompInfo&>(Other);
        OtherSubUV.ScaleU = ScaleU;
        OtherSubUV.ScaleV = ScaleV;
        OtherSubUV.OffsetU = OffsetU;
        OtherSubUV.OffsetV = OffsetV;
        OtherSubUV.indexU = indexU;
        OtherSubUV.indexV = indexV;
        OtherSubUV.Second = Second;
        OtherSubUV.FrameDuration = FrameDuration;
        OtherSubUV.CellsPerRow = CellsPerRow;
        OtherSubUV.CellsPerColumn = CellsPerColumn;
    }

    virtual void Serialize(FArchive& ar) const override
    {
        FBillboardComponentInfo::Serialize(ar);
        ar << ScaleU << ScaleV << OffsetU << OffsetV << indexU << indexV << Second << FrameDuration << CellsPerRow << CellsPerColumn;
    }

    virtual void Deserialize(FArchive& ar) override
    {
        FBillboardComponentInfo::Deserialize(ar);
        ar >> ScaleU >> ScaleV >> OffsetU >> OffsetV >> indexU >> indexV >> Second >> FrameDuration >> CellsPerRow >> CellsPerColumn;
    }
};

class UParticleSubUVComp : public UBillboardComponent
{
    DECLARE_CLASS(UParticleSubUVComp, UBillboardComponent)

public:
    UParticleSubUVComp();
    UParticleSubUVComp(const UParticleSubUVComp& other);
    virtual ~UParticleSubUVComp() override;

    virtual void InitializeComponent() override;
    virtual void TickComponent(float DeltaTime) override;

    UObject* Duplicate() const override;
    void DuplicateSubObjects(const UObject* Source) override;
    void SetRowColumnCount(int _cellsPerRow, int _cellsPerColumn);

    // ID3D11Buffer* vertexSubUVBuffer;
    //UINT numTextVertices;

    FVector2D GetUVOffset() const { return FVector2D(OffsetU, OffsetV); }
    FVector2D GetUVScale() const { return FVector2D(ScaleU, ScaleV); }
public:
    virtual std::shared_ptr<FActorComponentInfo> GetActorComponentInfo();
    virtual void LoadAndConstruct(const FActorComponentInfo& Info) override;

protected:
    bool bIsLoop = true;

private:
    float ScaleU = 0;
    float ScaleV = 0;
    
    float OffsetU = 0;
    float OffsetV = 0;

    // 현재 애니메이션 프레임 (열, 행 인덱스)
    int indexU = 0;
    int indexV = 0;
    
    float Second = 0;

    // 누적 시간 (프레임 전환을 위한)
    float ElapsedTime = 0.0f;
    // 프레임 당 지속 시간 (밀리초 단위, 필요에 따라 조정)
    float FrameDuration = 75.0f;

    int CellsPerRow = 1;
    int CellsPerColumn = 1;

    void UpdateVertexBuffer(const TArray<FVertexTexture>& vertices);
    void CreateSubUVVertexBuffer();
};
