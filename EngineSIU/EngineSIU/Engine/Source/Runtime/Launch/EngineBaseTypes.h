#pragma once
#include "HAL/PlatformType.h"


enum class EViewModeIndex : uint8
{
    VMI_Lit_Gouraud,
    VMI_Lit_Lambert,
    VMI_Lit_BlinnPhong,
    VMI_Unlit, // Lit 모드는 이 위에 추가해주세요.
    VMI_Wireframe,
    VMI_SceneDepth,
    VMI_WorldNormal,
    VMI_MAX,
};

enum ELevelTick
{
    /** Update the level time only. */
    LEVELTICK_TimeOnly = 0,
    /** Update time and viewports. */
    LEVELTICK_ViewportsOnly = 1,
    /** Update all. */
    LEVELTICK_All = 2,
    /** Delta time is zero, we are paused. Components don't tick. */
    LEVELTICK_PauseTick = 3,
};

enum ELevelViewportType : uint8
{
    LVT_Perspective = 0,
    /** Top */
    LVT_OrthoXY = 1,
    /** Bottom */
    LVT_OrthoNegativeXY,
    /** Left */
    LVT_OrthoYZ,
    /** Right */
    LVT_OrthoNegativeYZ,
    /** Front */
    LVT_OrthoXZ,
    /** Back */
    LVT_OrthoNegativeXZ,

    LVT_MAX,
    LVT_None = 255,
};
