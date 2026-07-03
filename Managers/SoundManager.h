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

	SoundType type_;
};

class SoundManager{
public:
	static SoundManager* GetInstance();

	void Initialize();

	void Update();

	void Finalize();

	SoundData LoadTest(const std::string& fileName);

	SoundData RegisterSound(const std::string& name, const std::string& filePath);

	SoundData GetSoundData(const std::string& name);

	void SetSoundDataVolume(const std::string& name, float volume);

	void SetSoundDataSpeed(const std::string& name, float speed);

	PlaySoundData* GetPlaySoundData(const std::string& name);

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

	
	void SoundPause(std::string handle);

	void SoundResume(std::string handle);

	void SoundStop(std::string handle);

	void SetSoundVolume(std::string handle, float volume) { GetPlaySoundData(handle)->voice->SetVolume(volume); SetSoundDataVolume(handle, volume); };
	
	void SetSoundSpeed(std::string handle, float speed) { GetPlaySoundData(handle)->voice->SetFrequencyRatio(speed); SetSoundDataSpeed(handle, speed); };

	bool IsFinishedSound(std::string handle);

	//void SoundPlayWave(IXAudio2* xAudio2, const SoundData& soundData);
	void SoundPlayWave(const SoundData& soundData);
private:
	void DeleteDatas();
private:
	Microsoft::WRL::ComPtr<IXAudio2> xAudio2_;
	IXAudio2MasteringVoice* masterVoice_;
	std::map<std::string, SoundData> sounds_;
	std::map<std::string, PlaySoundData*> playSoundDatas_;
	std::map<uint32_t, PlaySoundData*> soundOneTimeDatas_;

	uint32_t soundNum_ = 0;
};

