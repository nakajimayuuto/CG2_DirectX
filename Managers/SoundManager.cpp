#include "SoundManager.h"
#include "../Engine/SystemFile/Convert.h"

using Microsoft::WRL::ComPtr;
SoundManager* SoundManager::GetInstance() {
	static SoundManager instance;
	return &instance;
}

void SoundManager::Initialize() {
	HRESULT result = XAudio2Create(&xAudio2_);

	result = xAudio2_->CreateMasteringVoice(&masterVoice_);

	result = MFStartup(MF_VERSION);
	assert(SUCCEEDED(result));

	sounds_.clear();
	playSoundDatas_.clear();
	soundOneTimeDatas_.clear();

	//SoundData soundData = SoundLoadWave("Resource/Alarm01.wav");
	soundNum_ = 0;
}

void SoundManager::Update() {
	DeleteDatas();
}

void SoundManager::DeleteDatas() {
	std::vector<std::string> deleteHandles;
	std::vector<uint32_t> deleteNums;

	for (std::pair<std::string, PlaySoundData*> data : playSoundDatas_) {
		if (IsFinishedSound(data.first)) {
			if (!data.second->canLoop) {
				deleteHandles.push_back(data.first);
			}
		}
	}

	for (std::string& deleteHandle : deleteHandles) {
		SoundStop(deleteHandle);
	}


	for (std::pair<uint32_t, PlaySoundData*> data : soundOneTimeDatas_) {
		XAUDIO2_VOICE_STATE state;

		data.second->voice->GetState(&state);

		if (state.BuffersQueued == 0) {
			deleteNums.push_back(data.first);
		}
	}

	for (uint32_t& deleteNum : deleteNums) {
		soundOneTimeDatas_[deleteNum]->voice->Stop();
		soundOneTimeDatas_[deleteNum]->voice->FlushSourceBuffers();
		soundOneTimeDatas_.erase(deleteNum);
	}
}

void SoundManager::Finalize() {
	xAudio2_.Reset();

	for (auto i : sounds_) {
		SoundUnload(&i.second);
	}

	MFShutdown();
}

SoundData SoundManager::LoadTest(const std::string& fileName) {
	// ここで読み込み.
	ComPtr<IMFSourceReader> reader;

	// 楽だね.
	HRESULT hr = MFCreateSourceReaderFromURL(
		Convert::ConvertString(fileName).c_str(),
		nullptr,
		&reader
	);

	ComPtr<IMFMediaType> mediaType;

	MFCreateMediaType(&mediaType);

	// ここで音声ファイルをPCMにするらしい.
	mediaType->SetGUID(
		MF_MT_MAJOR_TYPE,
		MFMediaType_Audio);

	mediaType->SetGUID(
		MF_MT_SUBTYPE,
		MFAudioFormat_PCM);

	reader->SetCurrentMediaType(
		(DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM,
		nullptr,
		mediaType.Get());

	//WaveFormatを取得するらしい
	ComPtr<IMFMediaType> currentType;

	reader->GetCurrentMediaType(
		MF_SOURCE_READER_FIRST_AUDIO_STREAM,
		&currentType);

	WAVEFORMATEX* waveFormat = nullptr;

	MFCreateWaveFormatExFromMFMediaType(
		currentType.Get(),
		&waveFormat,
		nullptr);

	// 音声データの取得
	DWORD flags = 0;

	ComPtr<IMFSample> sample;

	reader->ReadSample(
		MF_SOURCE_READER_FIRST_AUDIO_STREAM,
		0,
		nullptr,
		&flags,
		nullptr,
		&sample);

	if (flags & MF_SOURCE_READERF_ENDOFSTREAM) {
		assert(false);
	}
	ComPtr<IMFMediaBuffer> buffer;
	std::vector<BYTE> pcmData;


	while (true)
	{
		DWORD flags = 0;

		ComPtr<IMFSample> sample;

		reader->ReadSample(
			MF_SOURCE_READER_FIRST_AUDIO_STREAM,
			0,
			nullptr,
			&flags,
			nullptr,
			&sample);

		if (flags & MF_SOURCE_READERF_ENDOFSTREAM)
			break;

		if (!sample)
			continue;

		// Bufferの取得

		sample->ConvertToContiguousBuffer(&buffer);

		BYTE* audioData = nullptr;

		DWORD maxLength = 0;
		DWORD currentLength = 0;

		buffer->Lock(
			&audioData,
			&maxLength,
			&currentLength);

		pcmData.insert(
			pcmData.end(),
			audioData,
			audioData + currentLength);

		buffer->Unlock();

	}

	SoundData data;
	data.pcmData = pcmData;
	data.waveFormat = *waveFormat;

	CoTaskMemFree(waveFormat);

	return data;
}

SoundData SoundManager::RegisterSound(const std::string& name, const std::string& filePath) {
	sounds_[name] = LoadTest(filePath);

	return sounds_[name];
}

SoundData SoundManager::GetSoundData(const std::string& name) {
	auto it = sounds_.find(name);

	assert(it != sounds_.end());
	return it->second;
}

void SoundManager::SetSoundDataVolume(const std::string& name, float volume) {
	auto it = playSoundDatas_.find(name);

	assert(it != playSoundDatas_.end());
	it->second->volume = volume;
}

void SoundManager::SetSoundDataSpeed(const std::string& name, float speed) {
	auto it = playSoundDatas_.find(name);

	assert(it != playSoundDatas_.end());
	it->second->currentSpeed = speed;
}

PlaySoundData* SoundManager::GetPlaySoundData(const std::string& name) {
	auto it = playSoundDatas_.find(name);

	assert(it != playSoundDatas_.end());
	return it->second;
}

// 変更予定
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

	//soundData.wfex = format.fmt;
	//soundData.pBuffer = reinterpret_cast<BYTE*>(pBuffer);
	//soundData.bufferSize = data.size;

	return soundData;
}

void SoundManager::SoundUnload(SoundData* soundData) {
	//delete[] soundData->pBuffer;
	//
	//soundData->pBuffer = 0;
	//soundData->bufferSize = 0;
	//soundData->wfex = {};
}

void SoundManager::SoundPlay(const SoundData& soundData, float speed, float volume, SoundType type, bool canLoop, std::string handle) {
	auto it = playSoundDatas_.find(handle);
	if (it != playSoundDatas_.end()) {
		return;
	}

	HRESULT result;

	// 波形フォーマットをもとにSourceVoiceの作成.
	IXAudio2SourceVoice* pSourceVoice = nullptr;
	result = xAudio2_->CreateSourceVoice(&pSourceVoice, &soundData.waveFormat);
	assert(SUCCEEDED(result));

	// 再生する波形データの設定.
	XAUDIO2_BUFFER buf{};
	buf.pAudioData = soundData.pcmData.data();
	buf.AudioBytes = soundData.pcmData.size();
	buf.Flags = XAUDIO2_END_OF_STREAM;

	if (canLoop) {
		buf.LoopLength = 0;
		buf.LoopBegin = 0;
		buf.LoopCount = XAUDIO2_LOOP_INFINITE;
	}

	// 波形データの再生.
	result = pSourceVoice->SubmitSourceBuffer(&buf);
	result = pSourceVoice->Start();

	PlaySoundData* data;
	data = new PlaySoundData();

	data->buffer = buf;
	data->voice = pSourceVoice;
	data->canLoop = canLoop;
	data->currentSpeed = speed;
	data->volume = volume;
	data->type_ = type;

	playSoundDatas_[handle] = data;
}

void SoundManager::SoundPlay(const SoundData& soundData, float speed, float volume, SoundType type) {
	HRESULT result;

	// 波形フォーマットをもとにSourceVoiceの作成.
	IXAudio2SourceVoice* pSourceVoice = nullptr;
	result = xAudio2_->CreateSourceVoice(&pSourceVoice, &soundData.waveFormat);
	assert(SUCCEEDED(result));

	// 再生する波形データの設定.
	XAUDIO2_BUFFER buf{};
	buf.pAudioData = soundData.pcmData.data();
	buf.AudioBytes = soundData.pcmData.size();
	buf.Flags = XAUDIO2_END_OF_STREAM;

	// 波形データの再生.
	result = pSourceVoice->SubmitSourceBuffer(&buf);
	result = pSourceVoice->Start();
	soundOneTimeDatas_[soundNum_] = new PlaySoundData();
	soundOneTimeDatas_[soundNum_]->buffer = buf;
	soundOneTimeDatas_[soundNum_]->voice = pSourceVoice;
	soundOneTimeDatas_[soundNum_]->canLoop = false;
	soundOneTimeDatas_[soundNum_]->currentSpeed = speed;
	soundOneTimeDatas_[soundNum_]->volume = volume;
	soundOneTimeDatas_[soundNum_]->type_ = type;

	soundNum_++;
}

void SoundManager::SoundPause(std::string handle) {
	auto it = playSoundDatas_.find(handle);
	if (it != playSoundDatas_.end()) {
		return;
	}

	GetPlaySoundData(handle)->voice->Stop();
}

void SoundManager::SoundResume(std::string handle) {
	auto it = playSoundDatas_.find(handle);
	if (it != playSoundDatas_.end()) {
		return;
	}

	GetPlaySoundData(handle)->voice->Start();
}

void SoundManager::SoundStop(std::string handle) {
	auto it = playSoundDatas_.find(handle);
	if (it == playSoundDatas_.end()) {
		return;
	}

	GetPlaySoundData(handle)->voice->Stop();
	GetPlaySoundData(handle)->voice->FlushSourceBuffers();
	playSoundDatas_.erase(handle);
}

bool SoundManager::IsFinishedSound(std::string handle) {
	auto it = playSoundDatas_.find(handle);
	if (it == playSoundDatas_.end()) {
		return false;
	}

	XAUDIO2_VOICE_STATE state;

	GetPlaySoundData(handle)->voice->GetState(&state);

	if (state.BuffersQueued == 0) {
		return true;
	}

	return false;
};

void SoundManager::SoundPlayWave(const SoundData& soundData) {

	HRESULT result;

	// 波形フォーマットをもとにSourceVoiceの作成.
	IXAudio2SourceVoice* pSourceVoice = nullptr;
	result = xAudio2_->CreateSourceVoice(&pSourceVoice, &soundData.waveFormat);
	assert(SUCCEEDED(result));

	// 再生する波形データの設定.
	XAUDIO2_BUFFER buf{};
	buf.pAudioData = soundData.pcmData.data();
	buf.AudioBytes = soundData.pcmData.size();
	buf.Flags = XAUDIO2_END_OF_STREAM;

	// 波形データの再生.
	result = pSourceVoice->SubmitSourceBuffer(&buf);
	result = pSourceVoice->Start();

	// 元のデータ.
	/*
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
	*/
}
