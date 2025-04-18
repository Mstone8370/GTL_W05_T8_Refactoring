#include "OutlinerEditorPanel.h"
#include "World/World.h"
#include "GameFramework/Actor.h"
#include "Engine/EditorEngine.h"
#include <functional>

void OutlinerEditorPanel::Render()
{
    /* Pre Setup */
    // ImGuiIO& io = ImGui::GetIO();
    //
    // float PanelWidth = (Width) * 0.2f - 6.0f;
    // float PanelHeight = (Height) * 0.3f;
    //
    // float PanelPosX = (Width) * 0.8f + 5.0f;
    // float PanelPosY = 5.0f;
    //
    // ImVec2 MinSize(140, 100);
    // ImVec2 MaxSize(FLT_MAX, 500);
    //
    // /* Min, Max Size */
    // ImGui::SetNextWindowSizeConstraints(MinSize, MaxSize);
    //
    // /* Panel Position */
    // ImGui::SetNextWindowPos(ImVec2(PanelPosX, PanelPosY), ImGuiCond_Always);
    //
    // /* Panel Size */
    // ImGui::SetNextWindowSize(ImVec2(PanelWidth, PanelHeight), ImGuiCond_Always);
    //
    // /* Panel Flags */
    // ImGuiWindowFlags PanelFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;
    //
    // /* Render Start */
    // ImGui::Begin("Outliner", nullptr, PanelFlags);
    //
    //
    // ImGui::BeginChild("Objects");
    // UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);
    // if (!Engine)
    //     return;
    //
    // std::function<void(USceneComponent*)> CreateNode = [&CreateNode, &Engine](USceneComponent* InComp)->void
    //     {
    //         FString Name;
    //
    //         if (InComp == InComp->GetOwner()->GetRootComponent())
    //         {
    //             Name = InComp->GetOwner()->GetActorLabel();
    //         }
    //         else
    //         {
    //             Name = InComp->GetName();
    //         }
    //
    //         ImGuiTreeNodeFlags Flags = ImGuiTreeNodeFlags_None;
    //         if (InComp->GetAttachChildren().Num() == 0)
    //             Flags |= ImGuiTreeNodeFlags_Leaf;
    //
    //         bool NodeOpen = ImGui::TreeNodeEx(*Name, Flags);
    //
    //         if (ImGui::IsItemClicked())
    //         {
    //             Engine->SelectActor(InComp->GetOwner());
    //             Engine->SelectComponent(InComp);
    //         }
    //
    //         if (NodeOpen)
    //         {
    //             for (USceneComponent* Child : InComp->GetAttachChildren())
    //             {
    //                 CreateNode(Child);
    //             }
    //             ImGui::TreePop(); // 트리 닫기
    //         }
    //     };
    //
    // for (AActor* Actor : Engine->ActiveWorld->GetActiveLevel()->Actors)
    // {
    //     CreateNode(Actor->GetRootComponent());
    // }
    //
    // ImGui::EndChild();
    //
    // ImGui::End();
    /* Pre Setup */
    ImGuiIO& io = ImGui::GetIO();
    
    float PanelWidth = (Width) * 0.2f - 6.0f;
    float PanelHeight = (Height) * 0.3f;

    float PanelPosX = (Width) * 0.8f + 5.0f;
    float PanelPosY = 5.0f;

    ImVec2 MinSize(140, 100);
    ImVec2 MaxSize(FLT_MAX, 500);
    
    /* Min, Max Size */
    ImGui::SetNextWindowSizeConstraints(MinSize, MaxSize);
    
    /* Panel Position */
    ImGui::SetNextWindowPos(ImVec2(PanelPosX, PanelPosY), ImGuiCond_Always);

    /* Panel Size */
    ImGui::SetNextWindowSize(ImVec2(PanelWidth, PanelHeight), ImGuiCond_Always);

    /* Panel Flags */
    ImGuiWindowFlags PanelFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;
    
    /* Render Start */
    ImGui::Begin("Outliner", nullptr, PanelFlags);

    UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);
    if (!Engine)
        return;
    if (ImGui::TreeNode("Actors")) // 트리 노드 생성
    {
        UWorld* World = GEngine->GetWorld();
        for (AActor* Actor : Engine->ActiveWorld->GetActiveLevel()->Actors)
        {
            bool bSelected = false; 
            if (ImGui::Selectable(*Actor->GetActorLabel(), bSelected))
            {
                Engine->SelectActor(Actor);
            }
        }
        ImGui::TreePop(); // 트리 닫기
    }
    ImGui::End();
}
    
void OutlinerEditorPanel::OnResize(HWND hWnd)
{
    RECT clientRect;
    GetClientRect(hWnd, &clientRect);
    Width = clientRect.right - clientRect.left;
    Height = clientRect.bottom - clientRect.top;
}
