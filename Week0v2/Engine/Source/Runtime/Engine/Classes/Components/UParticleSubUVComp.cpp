#include "UParticleSubUVComp.h"

#include "EditorEngine.h"
#include "UnrealEd/EditorViewportClient.h"
#include "Engine/World.h"

UParticleSubUVComp::UParticleSubUVComp()
    : CellsPerRow(0)
    , CellsPerColumn(0)
{
    bIsLoop = true;
}

UParticleSubUVComp::UParticleSubUVComp(const UParticleSubUVComp& other)
    : UBillboardComponent(other)
    , bIsLoop(other.bIsLoop)
    , indexU(other.indexU)
    , indexV(other.indexV)
    , ScaleU(other.ScaleU)
    , ScaleV(other.ScaleV)
    , Second(other.Second)
    , ElapsedTime(other.ElapsedTime)
    , FrameDuration(other.FrameDuration)
    , CellsPerColumn(other.CellsPerColumn)
    , CellsPerRow(other.CellsPerRow)
{
}

UParticleSubUVComp::~UParticleSubUVComp()
= default;

void UParticleSubUVComp::InitializeComponent()
{
	Super::InitializeComponent();
	// UEditorEngine::renderer.GetConstantBufferUpdater().UpdateSubUVConstant(UEditorEngine::renderer.SubUVConstantBuffer, 0, 0);
	// UEditorEngine::renderer.PrepareSubUVConstant();
}

void UParticleSubUVComp::TickComponent(const float DeltaTime)
{
    Super::TickComponent(DeltaTime);
    if (!IsActive() || !Texture)
        return;

    // 텍스처 크기를 기반으로 셀 크기 계산 (행, 열)
    uint32 CellWidth = Texture->width / CellsPerColumn;
    uint32 CellHeight = Texture->height / CellsPerRow;

    float UVSaleX = 1.0f / static_cast<float>(CellsPerColumn);
    float UVSaleY = 1.0f / static_cast<float>(CellsPerRow);

    // 시간 누적 후 프레임 전환
    ElapsedTime += (DeltaTime * 1000);
    if (ElapsedTime >= FrameDuration)
    {
        indexU++;
        ElapsedTime = 0.0f;
    }
    if (indexU >= CellsPerColumn)
    {
        indexU = 0;
        indexV++;
    }
    if (indexV >= CellsPerRow)
    {
        indexU = 0;
        indexV = 0;
        if (!bIsLoop)
        {
            Deactivate();
        }
    }

    float UVOffsetX = static_cast<float>(indexU) * UVSaleX;
    float UVOffsetY = static_cast<float>(indexV) * UVSaleY;

    ScaleU = UVSaleX;
    ScaleV = UVSaleY;

    indexU = UVOffsetX;
    indexV = UVOffsetY;
}

UObject* UParticleSubUVComp::Duplicate() const
{
    UParticleSubUVComp* Cloned = FObjectFactory::ConstructObjectFrom(this);
    Cloned->DuplicateSubObjects(this);
    return Cloned;
}

void UParticleSubUVComp::DuplicateSubObjects(const UObject* Source)
{
    UBillboardComponent::DuplicateSubObjects(Source);
}

void UParticleSubUVComp::SetRowColumnCount(int _cellsPerRow, int _cellsPerColumn)
{
	CellsPerRow = _cellsPerRow;
	CellsPerColumn = _cellsPerColumn;

	CreateSubUVVertexBuffer();
}

std::shared_ptr<FActorComponentInfo> UParticleSubUVComp::GetActorComponentInfo()
{
    std::shared_ptr<FParticleSubUVCompInfo> Info = std::make_shared<FParticleSubUVCompInfo>();
    Super::GetActorComponentInfo()->Copy(*Info);

    Info->CellsPerRow = CellsPerRow;
    Info->CellsPerColumn = CellsPerColumn;

    return Info;
}

void UParticleSubUVComp::LoadAndConstruct(const FActorComponentInfo& Info)
{
    Super::LoadAndConstruct(Info);
    const FParticleSubUVCompInfo& SubUVInfo = static_cast<const FParticleSubUVCompInfo&>(Info);
    CellsPerRow = SubUVInfo.CellsPerRow;
    CellsPerColumn = SubUVInfo.CellsPerColumn;
}

void UParticleSubUVComp::UpdateVertexBuffer(const TArray<FVertexTexture>& vertices)
{

}

void UParticleSubUVComp::CreateSubUVVertexBuffer()
{
	uint32 CellWidth = Texture->width/CellsPerColumn;
	uint32 CellHeight = Texture->height/ CellsPerColumn;
	float normalWidthOffset = float(CellWidth) / float(Texture->width);
	float normalHeightOffset = float(CellHeight) / float(Texture->height);

	TArray<FVertexTexture> vertices =
	{
		{-1.0f,1.0f,0.0f,0,0},
		{ 1.0f,1.0f,0.0f,1,0},
		{-1.0f,-1.0f,0.0f,0,1},
		{ 1.0f,-1.0f,0.0f,1,1}
	};
	vertices[1].u = normalWidthOffset;
	vertices[2].v = normalHeightOffset;
	vertices[3].u = normalWidthOffset;
	vertices[3].v = normalHeightOffset;

	ID3D11Buffer* VB = UEditorEngine::renderer.GetResourceManager()->CreateImmutableVertexBuffer<FVertexTexture>(vertices);
    UEditorEngine::renderer.GetResourceManager()->AddOrSetVertexBuffer(TEXT("QuadVB"), VB);
    UEditorEngine::renderer.MappingVBTopology(TEXT("Quad"), TEXT("QuadVB"), sizeof(FVertexTexture), 4);

    VBIBTopologyMappingName = TEXT("Quad");
}
