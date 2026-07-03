#pragma once
#include <xaudio2.h>
#pragma comment(lib,"xaudio2.lib")
#include <fstream>
#include <assert.h>
#include <map>
#include "../externals/DirectXTex/d3dx12.h"
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mfobjects.h>
#include <mferror.h>
#include <wrl/client.h>

#pragma comment(lib,"mfplat.lib")
#pragma comment(lib,"mfreadwrite.lib")
#pragma comment(lib,"mfuuid.lib")
#pragma comment(lib,"mf.lib")
#include "../Engine/Math/Easing.h"

// チャンクヘッダ.
struct ChunkHeader {
	char id[4]; // チャンク毎のID.
	int32_t size; // チャンクサイズ.
};

// RIFFヘッダチャンク.
struct RiffHeader {
	ChunkHeader chunk; // "RIFF"
	char type[4]; // "WAVE"
};

// FMTチャンク.
struct FormatChunk {
	ChunkHeader chunk; // "fml"
	WAVEFORMATEX fmt; // 波形フォーマット.
};

enum SoundType {
	kBGM,
	kSoundEffect,
	kSoundCount,
};

// 音声データ.
struct SoundData {
	// 波形フォーマット.
	//WAVEFORMATEX wfex;
	// バッファの先頭アドレス.
	//BYTE* pBuffer;
	// バッファのサイズ.
	//unsigned int bufferSize;
	WAVEFORMATEX waveFormat{};
	std::vector<BYTE> pcmData;
};

struct PlaySoundData {
	IXAudio2SourceVoice* voice = nullptr;
	XAUDIO2_BUFFER buffer{};

	float currentSpeed = 1.0f;
	float volume = 1.0f;
	bool canLoop = false;
};

class SoundManager{
public:
	static SoundManager* GetInstance();

	void Initialize();

	void Update();

	void Finalize();

	SoundData LoadTest();

	SoundData RegisterSound(const std::string& name, const std::string& filePath);

	SoundData GetSoundData(const std::string& name);

	PlaySoundData GetPlaySoundData(const std::string& name);

	SoundData SoundLoadWave(const char* fileName);

	void SoundUnload(SoundData* soundData);

	/// <summary>
	/// 音源を再生する.
	/// </summary>
	/// <param name="soundData">音声データ</param>
	/// <param name="speed">速度</param>
	/// <param name="volume">音量</param>
	/// <param name="type">音源の分類</param>
	/// <param name="canLoop">ループ再生するか</param>
	/// <param name="handle">音源のハンドル(主にBGM用)</param>
	void SoundPlay(const SoundData& soundData,float speed,float volume, SoundType type,bool canLoop,std::string handle);
	
	/// <summary>
	/// 音源を再生する(効果音等の一瞬流す音声用).
	/// </summary>
	/// <param name="soundData">音声データ</param>
	/// <param name="speed">速度</param>
	/// <param name="volume">音量</param>
	/// <param name="type">音源の分類</param>
	void SoundPlay(const SoundData& soundData, float speed, float volume, SoundType type);

	void SoundPause(std::string handle) { GetPlaySoundData(handle).voice->Stop(); };

	void SoundResume(std::string handle) { GetPlaySoundData(handle).voice->Start(); };

	//void SoundPlayWave(IXAudio2* xAudio2, const SoundData& soundData);
	void SoundPlayWave(const SoundData& soundData);
private:
	Microsoft::WRL::ComPtr<IXAudio2> xAudio2;
	IXAudio2MasteringVoice* masterVoice;
	std::map<std::string, SoundData> sounds_;
	std::map<std::string, PlaySoundData> playSoundDatas_;
	std::map<uint32_t, PlaySoundData> playSoundEffectDatas_;
};

