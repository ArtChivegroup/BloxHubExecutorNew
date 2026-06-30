/*
 * Dumped With: roblox-dumper 2.7
 * Created by: Jonah (jonahw on Discord)
 * Github: https://github.com/nopjo/roblox-dumper
 * Roblox Version: version-1a951716f19e4638
 * Time Taken: 3499 ms (3.499000 seconds)
 * Total Offsets: 278
 */

#pragma once
#include <cstdint>

// clang-format off
namespace offsets {
    inline constexpr const char* roblox_version = "version-1a951716f19e4638";

    namespace Atmosphere {
        inline constexpr uintptr_t Color = 0xD0;
        inline constexpr uintptr_t Decay = 0xDC;
        inline constexpr uintptr_t Density = 0xE8;
        inline constexpr uintptr_t Glare = 0xEC;
        inline constexpr uintptr_t Haze = 0xF0;
        inline constexpr uintptr_t Offset = 0xF4;
    }

    namespace Attribute {
        inline constexpr uintptr_t Key = 0x0;
        inline constexpr uintptr_t Size = 0x58;
        inline constexpr uintptr_t Value = 0x18;
    }

    namespace BasePart {
        inline constexpr uintptr_t CastShadow = 0xF5;
        inline constexpr uintptr_t Color3 = 0x194;
        inline constexpr uintptr_t Locked = 0xF6;
        inline constexpr uintptr_t Massless = 0xF7;
        inline constexpr uintptr_t Primitive = 0x148;
        inline constexpr uintptr_t Reflectance = 0xEC;
        inline constexpr uintptr_t Shape = 0x1B1;
        inline constexpr uintptr_t Transparency = 0xF0;
    }

    namespace BloomEffect {
        inline constexpr uintptr_t Intensity = 0xD0;
        inline constexpr uintptr_t Size = 0xD4;
        inline constexpr uintptr_t Threshold = 0xD8;
    }

    namespace ByteCode {
        inline constexpr uintptr_t Pointer = 0x10;
        inline constexpr uintptr_t Size = 0x20;
    }

    namespace Camera {
        inline constexpr uintptr_t CFrame = 0xF8;
        inline constexpr uintptr_t FieldOfView = 0x160;
        inline constexpr uintptr_t Position = 0x11C;
        inline constexpr uintptr_t Rotation = 0xF8;
        inline constexpr uintptr_t ViewportInt16 = 0x2AC;
        inline constexpr uintptr_t ViewportSize = 0x2E8;
    }

    namespace CharacterMesh {
        inline constexpr uintptr_t BaseTextureId = 0xE0;
        inline constexpr uintptr_t BodyPart = 0x160;
        inline constexpr uintptr_t MeshId = 0x110;
        inline constexpr uintptr_t OverlayTextureId = 0x140;
    }

    namespace DataModel {
        inline constexpr uintptr_t CreatorId = 0x198;
        inline constexpr uintptr_t GameId = 0x1A0;
        inline constexpr uintptr_t GameLoaded = 0x670;
        inline constexpr uintptr_t JobId = 0x138;
        inline constexpr uintptr_t PlaceId = 0x1A8;
        inline constexpr uintptr_t ServerIP = 0x658;
        inline constexpr uintptr_t Workspace = 0x178;
    }

    namespace FakeDataModel {
        inline constexpr uintptr_t Pointer = 0x7BE9278;
        inline constexpr uintptr_t RealDataModel = 0x1D0;
    }

    namespace GuiBase2D {
        inline constexpr uintptr_t AbsolutePosition = 0x10C;
        inline constexpr uintptr_t AbsoluteRotation = 0x188;
        inline constexpr uintptr_t AbsoluteSize = 0x118;
    }

    namespace GuiObject {
        inline constexpr uintptr_t Active = 0x5B0;
        inline constexpr uintptr_t AnchorPoint = 0x560;
        inline constexpr uintptr_t AutomaticSize = 0x568;
        inline constexpr uintptr_t BackgroundColor3 = 0x548;
        inline constexpr uintptr_t BackgroundTransparency = 0x56C;
        inline constexpr uintptr_t BorderColor3 = 0x554;
        inline constexpr uintptr_t BorderMode = 0x570;
        inline constexpr uintptr_t BorderSizePixel = 0x574;
        inline constexpr uintptr_t ClipsDescendants = 0x5B1;
        inline constexpr uintptr_t GuiState = 0x580;
        inline constexpr uintptr_t Interactable = 0x5B3;
        inline constexpr uintptr_t LayoutOrder = 0x588;
        inline constexpr uintptr_t Position = 0x518;
        inline constexpr uintptr_t Rotation = 0x188;
        inline constexpr uintptr_t Selectable = 0x5B4;
        inline constexpr uintptr_t SelectionOrder = 0x5A4;
        inline constexpr uintptr_t Size = 0x538;
        inline constexpr uintptr_t SizeConstraint = 0x5A8;
        inline constexpr uintptr_t Visible = 0x5B5;
        inline constexpr uintptr_t ZIndex = 0x5AC;
    }

    namespace Humanoid {
        inline constexpr uintptr_t AutoJumpEnabled = 0x1E0;
        inline constexpr uintptr_t AutoRotate = 0x1E1;
        inline constexpr uintptr_t AutomaticScalingEnabled = 0x1E2;
        inline constexpr uintptr_t BreakJointsOnDeath = 0x1E3;
        inline constexpr uintptr_t CameraOffset = 0x140;
        inline constexpr uintptr_t DisplayDistanceType = 0x18C;
        inline constexpr uintptr_t EvaluateStateMachine = 0x1E4;
        inline constexpr uintptr_t Health = 0x194;
        inline constexpr uintptr_t HealthDisplayDistance = 0x198;
        inline constexpr uintptr_t HealthDisplayType = 0x19C;
        inline constexpr uintptr_t HipHeight = 0x1A0;
        inline constexpr uintptr_t JumpHeight = 0x1AC;
        inline constexpr uintptr_t JumpPower = 0x1B0;
        inline constexpr uintptr_t MaxHealth = 0x1B4;
        inline constexpr uintptr_t MaxSlopeAngle = 0x1B8;
        inline constexpr uintptr_t NameDisplayDistance = 0x1BC;
        inline constexpr uintptr_t NameOcclusion = 0x1C0;
        inline constexpr uintptr_t RequiresNeck = 0x1E9;
        inline constexpr uintptr_t RigType = 0x1CC;
        inline constexpr uintptr_t SeatPart = 0x120;
        inline constexpr uintptr_t Sit = 0x1EA;
        inline constexpr uintptr_t TargetPoint = 0x164;
        inline constexpr uintptr_t UseJumpPower = 0x1EC;
        inline constexpr uintptr_t WalkSpeed = 0x1DC;
        inline constexpr uintptr_t WalkSpeedCheck = 0x3C4;
        inline constexpr uintptr_t WalkToPoint = 0x17C;
    }

    namespace InputObject {
        inline constexpr uintptr_t MousePosition = 0xEC;
    }

    namespace Instance {
        inline constexpr uintptr_t ChildrenEnd = 0x8;
        inline constexpr uintptr_t ChildrenStart = 0x78;
        inline constexpr uintptr_t ClassDescriptor = 0x18;
    }
}
