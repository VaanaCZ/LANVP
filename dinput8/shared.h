// ----------------------------------------------------------------------------
// shared.h
//
// Copyright (c) 2021-2024 Vaana
// ----------------------------------------------------------------------------

#pragma once

#define VPATCH_VERSION_MAJOR "2"
#define VPATCH_VERSION_MINOR "0"
#define VPATCH_VERSION_PATCH ""

#define VPATCH_VERSION		VPATCH_VERSION_MAJOR "." VPATCH_VERSION_MINOR VPATCH_VERSION_PATCH
#define VPATCH_VERSION_MENU VPATCH_VERSION_MAJOR "·" VPATCH_VERSION_MINOR VPATCH_VERSION_PATCH

// Helper classes

class I3DEngine
{
public:
	void* __vftptr;
	BYTE padding[0xE4];
	float framerate;
	BYTE padding2[0xE8];
	int viewWidth;
	int viewHeight;
};

class Clue
{
public:
	void* __vftptr;
	BYTE padding[0x20];
	void* __vftptr2;
};

class InspectionObject
{
public:
	void* __vftptr;
	BYTE padding[0x30];
	Clue* clue;
};

class ActorInstance;
class InspectionSession;
class InspectionMode;
class AnimEventHandler;

class InspectionStage
{
public:
	void* __vftptr;
	BYTE padding[0x8];
	void* __vftptr2;
	int state;
	BYTE padding2[0x50];
	AnimEventHandler* eventHandler;
};

class InspectionSystem
{
public:
	struct TB
	{
		ActorInstance* actor;
		InspectionSession* session;
		BYTE padding[0x4];
		InspectionMode* mode;
		BYTE padding2[0x4];
		InspectionObject* object;
		BYTE padding3[0x1C];
		InspectionObject** object1;
		InspectionObject** object2;
	};

	void* __vftptr;
	InspectionStage* stage;
	BYTE padding[0x8];
	TB tb;
};

class ICamera
{
public:
	BYTE padding[0x64];
	float fov;
	BYTE padding2[0x18];
};

class BlendCamera : ICamera
{
public:
	BYTE padding3[0x10];
	ICamera* from;
	ICamera* to;
	float blendingTarget;
	BYTE padding4[0x8];
	float blendingFactor;
};

class CameraManagerProperties;

class CameraManager
{
public:
	BlendCamera* blendCamera;						// class BlendCamera
	ICamera* clampYModifier;						// class ClampYModifier
	ICamera* collisionModifier;						// class CollisionModifier
	ICamera* springModifier;						// class SpringModifier
	ICamera* sphericalSpringModifier;				// class SphericalSpringModifier
	ICamera* noiseModifier;							// class NoiseModifier
	ICamera* zoomModifier;							// class ZoomModifier
	ICamera* elevationModifier;						// class ElevationModifier
	ICamera* cameraRelativeLookAtModifier;			// class CameraRelativeLookAtModifier
	ICamera* followModifier;						// class FollowModifier
	ICamera* nonPositionalInterpolationModifier;	// class NonPositionalInterpolationModifier
	ICamera* pullAroundModifier;					// class PullAroundModifier
	ICamera* jumpModifier;							// class JumpModifier
	ICamera* attentionFocusModifier;				// class AttentionFocusModifier
	ICamera* attentionFocusLookAtModifier;			// class AttentionFocusLookAtModifier
	ICamera* lookModifier2;							// class LookModifier2
	ICamera* lookModifier3;							// class LookModifier3
	ICamera* ambushCamera;							// class AmbushCamera
	ICamera* arrestCamera;							// class ArrestCamera
	ICamera* balancePlatformCamera;					// class BalancePlatformCamera
	ICamera* brawlingLockOnCamera;					// class BrawlingLockOnCamera
	ICamera* brawlingMoveCamera;					// class BrawlingMoveCamera
	ICamera* cameraCutCamera;						// class CameraCutCamera
	ICamera* chandelierCamera;						// class ChandelierCamera
	ICamera* clueConversationCamera;				// class ClueConversationCamera
	ICamera* conversationCamera;					// class ConversationCamera
	ICamera* coverCamera;							// class CoverCamera
	ICamera* cutsceneCamera;						// class CutsceneCamera
	ICamera* fallingCamera;							// class FallingCamera
	ICamera* flyCamera;								// class FlyCamera
	ICamera* gamewellCamera;						// class GamewellCamera
	ICamera* gunfightEndSequenceCamera;				// class GunfightEndSequenceCamera
	ICamera* idleCamera;							// class IdleCamera
	ICamera* inspectionCamera;						// class InspectionCamera
	ICamera* ledgerCamera;							// class LedgerCamera
	ICamera* locatorZoomCamera;						// class LocatorZoomCamera
	ICamera* manualZoomCamera;						// class ManualZoomCamera
	ICamera* playerCamera;							// class PlayerCamera
	ICamera* scriptedSequenceCamera;				// class ScriptedSequenceCamera
	ICamera* shootingCamera;						// class ShootingCamera
	ICamera* simpleCutCamera;						// class SimpleCutCamera
	ICamera* sittingCamera;							// class SittingCamera
	ICamera* staticCamera;							// class StaticCamera
	ICamera* tackleCamera;							// class TackleCamera
	ICamera* trolleyCarCamera;						// class TrolleyCarCamera
	ICamera* vantagePointCamera;					// class VantagePointCamera
	ICamera* vehicleFollowCamera;					// class VehicleFollowCamera
	ICamera* uIMainMenu3DCamera;					// class UIMainMenu3DCamera
	ICamera* vehicleSideCamera;						// class VehicleSideCamera
	ICamera* vehicleShowRoomCamera;					// class VehicleShowRoomCamera
	CameraManagerProperties* properties;
	BYTE padding[0x14];
	ICamera* activeCamera;
};

// Common signatures

#define MASK 0xFFFFFFFF // fixme
#define HERE 0xDDDDDDDD // fixme

static DWORD sigEngineDestructor[] =
{
		0x8B, 0x96, 0x9C, 0x00, 0x00, 0x00,
		0x83, 0xE1, 0xF8,
		0x8B, 0x01,
		0x8B, 0x40, 0x0C,
		0x52,
		0xFF, 0xD0,
		0xC7, 0x05, HERE, MASK, MASK, MASK, MASK, 0x00, 0x00, 0x00, 0x00,
		0x8B, 0x4C, 0x24, 0x10,
		0x64, 0x89, 0x0D, 0x00, 0x00, 0x00, 0x00
};

template<typename T> inline T lerp(const float c, const T& a, const T& b)
{
	return a + c * (b - a);
}