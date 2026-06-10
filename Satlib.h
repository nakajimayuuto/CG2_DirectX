#pragma once
#define NOMINMAX
#include "./Engine/SystemFile/GlobalVariables.h"
#include "./Engine/SystemFile/GameSystem.h"
#include "./Engine/SystemFile/DeltaTime.h"
#include "./Engine/SystemFile/ImGui.h"
#include "./Engine/SystemFile/Debug.h"

#include "./Engine/Math/TimedCall.h"
#include "./Engine/Math/Collision.h"
#include "./Engine/Math/Easing.h"
#include "./Engine/Math/Random.h"
#include "./Engine/Math/Beats.h"
#include "./Engine/Math/Shape.h"
#include "./Engine/Math/Math.h"

#include "./Engine/Renderer/DirectionalLight.h"
#include "./Engine/Renderer/PointLight.h"
#include "./Engine/Renderer/Particles.h"
#include "./Engine/Renderer/Renderer.h"
#include "./Engine/Renderer/Camera.h"

#include "./Managers/CollisionManager.h"
#include "./Managers/ParticleManager.h"
#include "./Managers/ModelManager.h"
#include "./Managers/SceneManager.h"
#include "./Managers/SoundManager.h"
#include "./Managers/InputManager.h"	

#include "./Environment.h"