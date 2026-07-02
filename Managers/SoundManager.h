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

// 音声データ.
struct SoundData {
	// 波形フォーマット.
	WAVEFORMATEX wfex;
	// バッファの先頭アドレス.
	BYTE* pBuffer;
	// バッファのサイズ.
	unsigned int bufferSize;
};


class SoundManager{
public:
	static SoundManager* GetInstance();

	void Initialize();

	void Finalize();

	void LoadTest();

	SoundData RegisterSound(const std::string& name, const std::string& filePath);

	SoundData GetSoundData(const std::string& name);

	SoundData SoundLoadWave(const char* fileName);

	void SoundUnload(SoundData* soundData);

	//void SoundPlayWave(IXAudio2* xAudio2, const SoundData& soundData);
	void SoundPlayWave(const SoundData& soundData);
private:
	Microsoft::WRL::ComPtr<IXAudio2> xAudio2;
	IXAudio2MasteringVoice* masterVoice;
	std::map<std::string, SoundData> sounds_;
};

