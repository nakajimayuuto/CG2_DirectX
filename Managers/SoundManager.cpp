#include "SoundManager.h"

SoundManager* SoundManager::GetInstance() {
	static SoundManager instance;
	return &instance;
}

void SoundManager::Initialize() {
	HRESULT result = XAudio2Create(&xAudio2);

	result = xAudio2->CreateMasteringVoice(&masterVoice);

	//SoundData soundData = SoundLoadWave("Resource/Alarm01.wav");
}

void SoundManager::Finalize() {
	xAudio2.Reset();

	for (auto i : sounds_) {
		SoundUnload(&i.second);
	}
}

SoundData SoundManager::RegisterSound(const std::string& name, const std::string& filePath){
	sounds_[name] = SoundLoadWave(filePath.c_str());

	return sounds_[name];
}

SoundData SoundManager::GetSoundData(const std::string& name){
	auto it = sounds_.find(name);

	assert(it != sounds_.end());
	return it->second;
}

SoundData SoundManager::SoundLoadWave(const char* fileName) {
	// 1. ファイルオープン.

	// ファイル入力ストリームのインスタンス.
	std::ifstream file;
	// .wavファイルをバイナリモードで開く.
	file.open(fileName, std::ios_base::binary);
	// ファイルオープン失敗を検出する.
	assert(file.is_open());


	// 2. .wavデータ読み込み.

	// RIFFヘッダーの読み込み.
	RiffHeader riff;
	file.read(reinterpret_cast<char*>(&riff), sizeof(riff));
	// ファイルがRIFFかチェック.
	if (strncmp(riff.chunk.id, "RIFF", 4) != 0) {
		assert(false);
	}
	// タイプがWAVEかチェック.
	if (strncmp(riff.type, "WAVE", 4) != 0) {
		assert(false);
	}

	// Formatチャンクの読み込み.
	FormatChunk format = {};
	// チャンクヘッダーの確認.
	file.read(reinterpret_cast<char*>(&format), sizeof(ChunkHeader));
	if (strncmp(format.chunk.id, "fmt ", 4) != 0) {
		assert(false);
	}

	// チャンク本体の読み込み.
	assert(format.chunk.size <= sizeof(format.fmt));
	file.read(reinterpret_cast<char*>(&format.fmt), format.chunk.size);

	// Dataチャンクの読み込み.
	ChunkHeader data;
	file.read(reinterpret_cast<char*>(&data), sizeof(data));
	// JUNKチャンクを検出した場合.
	if (strncmp(data.id, "JUNK", 4) == 0) {
		// 読み取り位置をJUNKチャンクの終わりまで進める.
		file.seekg(data.size, std::ios_base::cur);
		// 再読み込み.
		file.read(reinterpret_cast<char*>(&data), sizeof(data));
	}

	if (strncmp(data.id, "LIST", 4) == 0) {
		// 読み取り位置をJUNKチャンクの終わりまで進める.
		file.seekg(data.size, std::ios_base::cur);
		// 再読み込み.
		file.read(reinterpret_cast<char*>(&data), sizeof(data));
	}

	if (strncmp(data.id, "data", 4) != 0) {
		assert(false);
	}

	// Dataチャンクのデータ部(波形データ)の読み込み.
	char* pBuffer = new char[data.size];
	file.read(pBuffer, data.size);


	// 3. ファイルクローズ.

	// Waveファイルを閉じる.
	file.close();


	// 4. 読み込んだ音声データをreturn.

	// returnする為の音声データ.
	SoundData soundData = {};

	soundData.wfex = format.fmt;
	soundData.pBuffer = reinterpret_cast<BYTE*>(pBuffer);
	soundData.bufferSize = data.size;

	return soundData;
}

void SoundManager::SoundUnload(SoundData* soundData) {
	delete[] soundData->pBuffer;

	soundData->pBuffer = 0;
	soundData->bufferSize = 0;
	soundData->wfex = {};
}

void SoundManager::SoundPlayWave(const SoundData& soundData) {
	HRESULT result;

	// 波形フォーマットをもとにSourceVoiceの作成.
	IXAudio2SourceVoice* pSourceVoice = nullptr;
	result = xAudio2->CreateSourceVoice(&pSourceVoice, &soundData.wfex);
	assert(SUCCEEDED(result));

	// 再生する波形データの設定.
	XAUDIO2_BUFFER buf{};
	buf.pAudioData = soundData.pBuffer;
	buf.AudioBytes = soundData.bufferSize;
	buf.Flags = XAUDIO2_END_OF_STREAM;

	// 波形データの再生.
	result = pSourceVoice->SubmitSourceBuffer(&buf);
	result = pSourceVoice->Start();
}