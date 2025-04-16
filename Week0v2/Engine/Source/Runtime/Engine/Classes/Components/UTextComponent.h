#pragma once
#include "PrimitiveComponent.h"

struct FTexture;

struct FTextComponentInfo : public FPrimitiveComponentInfo
{
    DECLARE_ACTORCOMPONENT_INFO(FTextComponentInfo);

    bool bIsBillboard;
    float finalIndexU;
    float finalIndexV;
    FWString TexturePath; 
    FWString Text;
    TArray<FVector> Quad;
    int RowCount;
    int ColumnCount;
    float QuadWidth;
    float QuadHeight;

    
    FTextComponentInfo()
        : FPrimitiveComponentInfo()
        , Text(L"")
    {
        InfoType = TEXT("FTextComponentInfo");
        ComponentType = TEXT("UText");
    }
    virtual void Copy(FActorComponentInfo& Other) override
    {
        FPrimitiveComponentInfo::Copy(Other);
        FTextComponentInfo& TextInfo = static_cast<FTextComponentInfo&>(Other);
        TextInfo.bIsBillboard = bIsBillboard;
        TextInfo.finalIndexU = finalIndexU;
        TextInfo.finalIndexV = finalIndexV;
        TextInfo.TexturePath = TexturePath;
        TextInfo.Text = Text;
        TextInfo.Quad = Quad;
        TextInfo.RowCount = RowCount;
        TextInfo.ColumnCount = ColumnCount;
        TextInfo.Quad = Quad;
        TextInfo.QuadWidth = QuadWidth;
        TextInfo.QuadHeight = QuadHeight;
    }

    virtual void Serialize(FArchive& ar) const override
    {
        FPrimitiveComponentInfo::Serialize(ar);
        ar << bIsBillboard << finalIndexU << finalIndexV << TexturePath << Text << Quad << RowCount << ColumnCount << Quad << QuadWidth << QuadHeight;
    }

    virtual void Deserialize(FArchive& ar) override
    {
        FPrimitiveComponentInfo::Deserialize(ar);
        ar >> bIsBillboard >> finalIndexU >> finalIndexV >> TexturePath >> Text >> Quad >> RowCount >> ColumnCount >> Quad >> QuadWidth >> QuadHeight;
    }
};

class UTextComponent : public UPrimitiveComponent
{
    DECLARE_CLASS(UTextComponent, UPrimitiveComponent)

public:
    UTextComponent();
    virtual ~UTextComponent() override;
    UTextComponent(const UTextComponent& Other);
    virtual void InitializeComponent() override;
    virtual void TickComponent(float DeltaTime) override;
    void ClearText();
    void SetText(const FWString& InText);
    FWString GetText() { return Text; }
    void SetRowColumnCount(int InCellsPerRow, int InCellsPerColumn);
    virtual int CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance) override;
    virtual UObject* Duplicate() const override;
    virtual void DuplicateSubObjects(const UObject* Source) override;
    virtual void PostDuplicate() override;

    void SetTexture(const FWString& InFileName);
    FMatrix CreateBillboardMatrix();
    
    TArray<FVertexTexture> VertexTextureArr;

public:    
    virtual std::shared_ptr<FActorComponentInfo> GetActorComponentInfo() override;
    void LoadAndConstruct(const FActorComponentInfo& Info) override;

    bool CheckPickingOnNDC(const TArray<FVector>& InCheckQuad, float& InHitDistance);
    
    bool IsBillboard() const { return bIsBillboard; }
    void SetIsBillboard(const bool InbIsBillboard) { bIsBillboard = InbIsBillboard; }

    float GetFinalIndexU() const { return FinalIndexU; }
    float GetFinalIndexV() const { return FinalIndexV; }

    std::shared_ptr<FTexture> GetTexture() const { return Texture; }
    
private:
    bool bIsBillboard = false;
    
    float FinalIndexU = 0.0f;
    float FinalIndexV = 0.0f;
    std::shared_ptr<FTexture> Texture;
    
    FWString Text;

    TArray<FVector> Quad;

    const int QuadSize = 2;

    int RowCount;
    int ColumnCount;

    float QuadWidth = 2.0f;
    float QuadHeight = 2.0f;

    void SetEngStartUV(char alphabet, float& outStartU, float& outStartV) const;
    void SetKorStartUV(wchar_t hangul, float& outStartU, float& outStartV) const;

    void TextMVPRendering();
};