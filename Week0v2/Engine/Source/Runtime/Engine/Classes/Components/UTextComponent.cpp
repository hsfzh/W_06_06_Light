#include "UTextComponent.h"

#include "EditorEngine.h"
#include "Engine/World.h"
#include "Engine/Source/Editor/PropertyEditor/ShowFlags.h"
#include "UnrealEd/EditorViewportClient.h"
#include "LevelEditor/SLevelEditor.h"

UTextComponent::UTextComponent()
{
}

UTextComponent::~UTextComponent()
{

}

UTextComponent::UTextComponent(const UTextComponent& Other)
    : Super(Other)
    , VertexTextureArr(Other.VertexTextureArr)
    , bIsBillboard(Other.bIsBillboard)
    , FinalIndexU(Other.FinalIndexU)
    , FinalIndexV(Other.FinalIndexV)
    , Texture(Other.Texture)
    , Text(Other.Text)
    , Quad(Other.Quad)
    , RowCount(Other.RowCount)
    , ColumnCount(Other.ColumnCount)
    , QuadWidth(Other.QuadWidth)
    , QuadHeight(Other.QuadHeight)
{
}

void UTextComponent::InitializeComponent()
{
    Super::InitializeComponent();
}

void UTextComponent::TickComponent(float DeltaTime)
{
	Super::TickComponent(DeltaTime);
    
}

void UTextComponent::ClearText()
{
    VertexTextureArr.Empty();
}

void UTextComponent::SetRowColumnCount(const int InCellsPerRow, const int InCellsPerColumn) 
{
    RowCount = InCellsPerRow;
    ColumnCount = InCellsPerColumn;
}

int UTextComponent::CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance)
{
	if (!(ShowFlags::GetInstance().currentFlags & static_cast<uint64>(EEngineShowFlags::SF_BillboardText))) {
		return 0;
	}
	for (int i = 0; i < VertexTextureArr.Num(); i++)
	{
		Quad.Add(FVector(VertexTextureArr[i].x,
			VertexTextureArr[i].y, VertexTextureArr[i].z));
	}

	return CheckPickingOnNDC(Quad, pfNearHitDistance);
}

UObject* UTextComponent::Duplicate() const
{
    UTextComponent* ClonedActor = FObjectFactory::ConstructObjectFrom<UTextComponent>(this);
    ClonedActor->DuplicateSubObjects(this);
    ClonedActor->PostDuplicate();
    return ClonedActor;
}

void UTextComponent::DuplicateSubObjects(const UObject* Source)
{
    Super::DuplicateSubObjects(Source);
}

void UTextComponent::PostDuplicate()
{
    Super::PostDuplicate();
}

void UTextComponent::SetTexture(const FWString& InFileName)
{
    Texture = UEditorEngine::resourceMgr.GetTexture(InFileName);
}

FMatrix UTextComponent::CreateBillboardMatrix()
{
    FMatrix CameraView = GetEngine()->GetLevelEditor()->GetActiveViewportClient()->GetViewMatrix();

    CameraView.M[0][3] = 0.0f;
    CameraView.M[1][3] = 0.0f;
    CameraView.M[2][3] = 0.0f;


    CameraView.M[3][0] = 0.0f;
    CameraView.M[3][1] = 0.0f;
    CameraView.M[3][2] = 0.0f;
    CameraView.M[3][3] = 1.0f;


    CameraView.M[0][2] = -CameraView.M[0][2];
    CameraView.M[1][2] = -CameraView.M[1][2];
    CameraView.M[2][2] = -CameraView.M[2][2];
    const FMatrix LookAtCamera = FMatrix::Transpose(CameraView);

    const FVector worldLocation = GetComponentLocation();
    const FVector worldScale = RelativeScale3D;
    const FMatrix S = FMatrix::CreateScale(worldScale.x, worldScale.y, worldScale.z);
    const FMatrix R = LookAtCamera;
    const FMatrix T = FMatrix::CreateTranslationMatrix(worldLocation);
    const FMatrix M = S * R * T;

    return M;
}

std::shared_ptr<FActorComponentInfo> UTextComponent::GetActorComponentInfo()
{
    std::shared_ptr<FTextComponentInfo> Info = std::make_shared<FTextComponentInfo>();
    Super::GetActorComponentInfo()->Copy(*Info);

    Info->Text = Text;

    return Info;
}

void UTextComponent::LoadAndConstruct(const FActorComponentInfo& Info)
{
    Super::LoadAndConstruct(Info);

    // cast
    const FTextComponentInfo& TextInfo = static_cast<const FTextComponentInfo&>(Info);
    SetText(TextInfo.Text);
}

void UTextComponent::SetText(const FWString& InText)
{
	Text = InText;
	if (InText.empty())
	{
		Console::GetInstance().AddLog(LogLevel::Warning, "Text is empty");

		VertexTextureArr.Empty();
		Quad.Empty();
	    
		return;
	}
	int textSize = static_cast<int>(InText.size());

	const uint32 BitmapWidth = Texture->width;
	const uint32 BitmapHeight = Texture->height;

	const float CellWidth =  static_cast<float>(BitmapWidth) / ColumnCount;
	const float CellHeight = static_cast<float>(BitmapHeight) / RowCount;

	const float nTexelUOffset = CellWidth / BitmapWidth;
	const float nTexelVOffset = CellHeight/ BitmapHeight;

	for (int i = 0; i < InText.size(); i++)
	{
		FVertexTexture leftUP = { -1.0f,1.0f,0.0f,0.0f,0.0f };
		FVertexTexture rightUP = { 1.0f,1.0f,0.0f,1.0f,0.0f };
		FVertexTexture leftDown = { -1.0f,-1.0f,0.0f,0.0f,1.0f };
		FVertexTexture rightDown = { 1.0f,-1.0f,0.0f,1.0f,1.0f };
		rightUP.u *= nTexelUOffset;
		leftDown.v *= nTexelVOffset;
		rightDown.u *= nTexelUOffset;
		rightDown.v *= nTexelVOffset;

		leftUP.x += QuadWidth * i;
		rightUP.x += QuadWidth * i;
		leftDown.x += QuadWidth * i;
		rightDown.x += QuadWidth * i;

		float startU = 0.0f;
		float startV = 0.0f;

		SetKorStartUV(InText[i], startU, startV);
		leftUP.u += (nTexelUOffset * startU);
		leftUP.v += (nTexelVOffset * startV);
		rightUP.u += (nTexelUOffset * startU);
		rightUP.v += (nTexelVOffset * startV);
		leftDown.u += (nTexelUOffset * startU);
		leftDown.v += (nTexelVOffset * startV);
		rightDown.u += (nTexelUOffset * startU);
		rightDown.v += (nTexelVOffset * startV);

		VertexTextureArr.Add(leftUP);
		VertexTextureArr.Add(rightUP);
		VertexTextureArr.Add(leftDown);
		VertexTextureArr.Add(rightUP);
		VertexTextureArr.Add(rightDown);
		VertexTextureArr.Add(leftDown);
	}

	const float lastX = -1.0f + QuadSize * InText.size();
	Quad.Add(FVector(-1.0f,1.0f,0.0f));
	Quad.Add(FVector(-1.0f,-1.0f,0.0f));
	Quad.Add(FVector(lastX,1.0f,0.0f));
	Quad.Add(FVector(lastX,-1.0f,0.0f));
    
    ID3D11Buffer* VB = nullptr;
    
    VB = GetEngine()->renderer.GetResourceManager()->CreateImmutableVertexBuffer<FVertexTexture>(VertexTextureArr);

    int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, 
                                     Text.c_str(), -1, 
                                     nullptr, 0, 
                                     nullptr, nullptr);
    std::string result(sizeNeeded, 0);
    WideCharToMultiByte(CP_UTF8, 0, 
                        Text.c_str(), -1, 
                        &result[0], sizeNeeded, 
                        nullptr, nullptr);

    const FString textName = result;

    result.pop_back(); // 널 문자 제거
    GetEngine()->renderer.GetResourceManager()->AddOrSetVertexBuffer(textName, VB);
    GetEngine()->renderer.MappingVBTopology(textName, textName, sizeof(FVertexSimple), VertexTextureArr.Num());
    VBIBTopologyMappingName = textName;
}

void UTextComponent::SetKorStartUV(const wchar_t hangul, float& outStartU, float& outStartV) const
{
    //대문자만 받는중
    int StartU = 0;
    int StartV = 0;
    int offset = -1;

    if (hangul == L' ') {
        outStartU = 0;  // Space는 특별히 UV 좌표를 (0,0)으로 설정
        outStartV = 0;
        offset = 0;
        return;
    }
    else if (hangul >= L'A' && hangul <= L'Z') {

        StartU = 11;
        StartV = 0;
        offset = hangul - L'A'; // 대문자 위치
    }
    else if (hangul >= L'a' && hangul <= L'z') {
        StartU = 37;
        StartV = 0;
        offset = (hangul - L'a'); // 소문자는 대문자 다음 위치
    }
    else if (hangul >= L'0' && hangul <= L'9') {
        StartU = 1;
        StartV = 0;
        offset = (hangul - L'0'); // 숫자는 소문자 다음 위치
    }
    else if (hangul >= L'가' && hangul <= L'힣')
    {
        StartU = 63;
        StartV = 0;
        offset = hangul - L'가'; // 대문자 위치
    }

    if (offset == -1)
    {
        Console::GetInstance().AddLog(LogLevel::Warning, "Text Error");
    }

    int offsetV = (offset + StartU) / ColumnCount;
    int offsetU = (offset + StartU) % ColumnCount;

    outStartU = static_cast<float>(offsetU);
    outStartV = static_cast<float>(StartV + offsetV);
}

void UTextComponent::SetEngStartUV(const char alphabet, float& outStartU, float& outStartV) const
{
    //대문자만 받는중
    int StartU=0;
    int StartV=0;
    int offset = -1;


    if (alphabet == ' ') {
        outStartU = 0;  // Space는 특별히 UV 좌표를 (0,0)으로 설정
        outStartV = 0;
        offset = 0;
        return;
    }
    else if (alphabet >= 'A' && alphabet <= 'Z') {

        StartU = 1;
        StartV = 4;
        offset = alphabet - 'A'; // 대문자 위치
    }
    else if (alphabet >= 'a' && alphabet <= 'z') {
        StartU = 1;
        StartV = 6;
        offset = (alphabet - 'a'); // 소문자는 대문자 다음 위치
    }
    else if (alphabet >= '0' && alphabet <= '9') {
        StartU = 0;
        StartV = 3;
        offset = (alphabet - '0'); // 숫자는 소문자 다음 위치
    }

    if (offset == -1)
    {
        Console::GetInstance().AddLog(LogLevel::Warning, "Text Error");
    }

    int offsetV = (offset + StartU) / ColumnCount;
    int offsetU = (offset + StartU) % ColumnCount;

    outStartU = static_cast<float>(offsetU);
    outStartV = static_cast<float>(StartV + offsetV);

}

bool UTextComponent::CheckPickingOnNDC(const TArray<FVector>& InCheckQuad, float& InHitDistance)
{
    bool result = false;
    POINT mousePos;
    GetCursorPos(&mousePos);
    ScreenToClient(GEngine->hWnd, &mousePos);

    D3D11_VIEWPORT viewport;
    UINT numViewports = 1;
    UEditorEngine::graphicDevice.DeviceContext->RSGetViewports(&numViewports, &viewport);
    float screenWidth = viewport.Width;
    float screenHeight = viewport.Height;

    FVector pickPosition;
    const int screenX = mousePos.x;
    const int screenY = mousePos.y;
    const FMatrix projectionMatrix = GetEngine()->GetLevelEditor()->GetActiveViewportClient()->GetProjectionMatrix();
    pickPosition.x = ((2.0f * screenX / viewport.Width) - 1);
    pickPosition.y = -((2.0f * screenY / viewport.Height) - 1);
    pickPosition.z = 1.0f; // Near Plane

    const FMatrix M = CreateBillboardMatrix();
    const FMatrix V = GEngine->GetLevelEditor()->GetActiveViewportClient()->GetViewMatrix();;
    const FMatrix P = projectionMatrix;
    const FMatrix MVP = M * V * P;

    float minX = FLT_MAX;
    float maxX = FLT_MIN;
    float minY = FLT_MAX;
    float maxY = FLT_MIN;
    float avgZ = 0.0f;
    for (int i = 0; i < InCheckQuad.Num(); i++)
    {
        FVector4 v = FVector4(InCheckQuad[i].x, InCheckQuad[i].y, InCheckQuad[i].z, 1.0f);
        FVector4 clipPos = FMatrix::TransformVector(v, MVP);
		
        if (clipPos.w != 0)	clipPos = clipPos/clipPos.w;

        minX = FMath::Min(minX, clipPos.x);
        maxX = FMath::Max(maxX, clipPos.x);
        minY = FMath::Min(minY, clipPos.y);
        maxY = FMath::Max(maxY, clipPos.y);
        avgZ += clipPos.z;
    }

    avgZ /= InCheckQuad.Num();

    if (pickPosition.x >= minX && pickPosition.x <= maxX &&
        pickPosition.y >= minY && pickPosition.y <= maxY)
    {
        float A = P.M[2][2];  // Projection Matrix의 A값 (Z 변환 계수)
        float B = P.M[3][2];  // Projection Matrix의 B값 (Z 변환 계수)

        float z_view_pick = (pickPosition.z - B) / A; // 마우스 클릭 View 공간 Z
        float z_view_billboard = (avgZ - B) / A; // Billboard View 공간 Z

        InHitDistance = 1000.0f;
        result = true;
    }

    return result;    
}

void UTextComponent::TextMVPRendering()
{
    // UEditorEngine::renderer.PrepareTextureShader();
    // //FEngineLoop::renderer.UpdateSubUVConstant(0, 0);
    // //FEngineLoop::renderer.PrepareSubUVConstant();
    // FMatrix Model = CreateBillboardMatrix();
    //
    // FMatrix ViewProj = GetEngine()->GetLevelEditor()->GetActiveViewportClient()->GetViewMatrix() * GetEngine()->GetLevelEditor()->GetActiveViewportClient()->GetProjectionMatrix();
    //
    // FMatrix NormalMatrix = FMatrix::Transpose(FMatrix::Inverse(Model));
    // FVector4 UUIDColor = EncodeUUID() / 255.0f;
    // if (this == GetWorld()->GetPickingGizmo()) {
    //     UEditorEngine::renderer.GetConstantBufferUpdater().UpdateConstant(UEditorEngine::renderer.ConstantBuffer, Model, ViewProj, NormalMatrix, UUIDColor, true);
    // }
    // else
    //     UEditorEngine::renderer.GetConstantBufferUpdater().UpdateConstant(UEditorEngine::renderer.ConstantBuffer, Model, ViewProj, NormalMatrix, UUIDColor, false);
    //
    // if (ShowFlags::GetInstance().currentFlags & static_cast<uint64>(EEngineShowFlags::SF_BillboardText)) {
    //     UEditorEngine::renderer.RenderTextPrimitive(vertexTextBuffer, numTextVertices,
    //         Texture->TextureSRV, Texture->SamplerState);
    // }
    // //Super::Render();
    //
    // UEditorEngine::renderer.PrepareShader();
}
