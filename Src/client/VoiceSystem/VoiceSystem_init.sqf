// ======================================================
// Copyright (c) 2017-2024 the ReSDK_A3 project
// sdk.relicta.ru
// ======================================================

#include <engine.h>
#include <script.h>
#include <VoiceSystem_widgetEnums.h>

//Включенный флаг отключает компиляцию старых функций
#define VOICE_DISABLE_LEGACYCODE

//Новый алгоритм затухания звука
#define VOICE_USE_NEW_ALGORITM_VOICE_INTERSECTION

#define VS_MAXIMUM_VOLUME_DISTANCE 60

#define VOICE_USE_LATEST_INTERSECTION_ALG

#ifdef EDITOR
	#define VOICE_DEBUG_LATEST_INTERSECTION_ALG
#endif

#include "VoiceSystem_keysConstant.sqf"
#include "VoiceSystem_uncategorized.sqf"
//Всё что не влезло в первый файл по препроцессору влезет во вторую часть
#include "VoiceSystem_part2.sqf"

//Публичный интерфейс управления
#include "VoiceSystem_publicInterface.sqf"
