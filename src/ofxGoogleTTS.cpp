//
//  ofxGoogleTTS.cpp
//  Created by Osamu Shigeta on 3/28/2013.
//
#include "ofxGoogleTTS.h"

static const std::string GOOGLE_TTS_URI("http://translate.google.com/translate_tts");
static const std::string MP3_FILE_PATH("tempTTSData");
static const float DEFAULT_VOLUME(1.f);


ofxGoogleTTS::ofxGoogleTTS()
{
	++mRefCount;
}

ofxGoogleTTS::~ofxGoogleTTS()
{
	exit();
	--mRefCount;
	if (mRefCount <= 0) {
		mRefCount = 0;
		Poco::File file(ofToDataPath("tempTTSData"));
		if (file.exists()) {
			file.remove(true);
		}
	}
}

void ofxGoogleTTS::setup( const std::string& proxyHost/*=""*/, int proxyPort/*=0*/ )
{
	createTempFolder();

	mVol = DEFAULT_VOLUME;
	mLoader.setup(proxyHost, proxyPort);
	ofAddListener(mLoader.mLoadSuccess, this, &ofxGoogleTTS::loadSuccess);
	ofAddListener(mLoader.mLoadFailed, this, &ofxGoogleTTS::loadFailed);
}

void ofxGoogleTTS::exit()
{
	ofRemoveListener(mLoader.mLoadSuccess, this, &ofxGoogleTTS::loadSuccess);
	ofRemoveListener(mLoader.mLoadFailed, this, &ofxGoogleTTS::loadFailed);
	mLoader.clear();
	mPlayers.clear();
}

void ofxGoogleTTS::speak( const std::string& text, Language language )
{
	const std::string lang(ofxGoogleTTS::getLangTag(language));
	const std::string playerName(getPlayerName(lang, text));

	if (0 == mPlayers.count(playerName)) {
		//const std::string fileName(getFileName(mPlayers.size()));
		const std::string fileName(getFileName(ofGetElapsedTimeMillis()));
		mPlayers[playerName] = ofSoundPlayer();
		mLoader.entry(mPlayers[playerName], playerName, fileName, text, language);
		if (!mLoader.isThreadRunning()) mLoader.startThread();
	} else {
		mPlayers[playerName].setVolume(mVol);
		mPlayers[playerName].play();
	}
}

float ofxGoogleTTS::getVolume() const {
	return mVol;
}

void ofxGoogleTTS::setVolume(float vol) {
	mVol = ofClamp(vol, 0.f, 1.f);
}

bool ofxGoogleTTS::save(const std::string& fileName, const std::string& text, Language language)
{
	mLoader.lock();
	bool ret(mLoader.save(ofToDataPath(fileName), text, language));
	mLoader.unlock();
	return ret;
}

bool ofxGoogleTTS::getSoundPlayer(ofSoundPlayer& player, const std::string& text, Language language/*=LANGUAGE_ENGLISH*/)
{
	//const std::string fileName(getFileName(mPlayers.size()));
	const std::string fileName(getFileName(ofGetElapsedTimeMillis()));
	bool ret(false);
	mLoader.lock();
	ret = mLoader.save(ofToDataPath(fileName), text, language);
	if (ret) {
		ret = player.loadSound(fileName);
	}
	mLoader.unlock();
	return ret;
}

bool ofxGoogleTTS::isSpeaking()
{
	for (std::map<std::string, ofSoundPlayer>::iterator it=mPlayers.begin(); it!=mPlayers.end(); ++it) {
		if (it->second.getIsPlaying()) return true;
	}
	return false;
}

void ofxGoogleTTS::clearSoundPlayers()
{
	mLoader.clear();
	mPlayers.clear();
	//createTempFolder();
}

////////////////////////////////////////////////////////////////////////////
std::string ofxGoogleTTS::getPlayerName(const std::string& lang, const std::string& text) const
{
	return lang + "_" + text;
}

std::string ofxGoogleTTS::getFileName(int index) const
{
	return MP3_FILE_PATH + "/" + ofToString(index) + ".mp3";
}

bool ofxGoogleTTS::createTempFolder()
{
	Poco::File file(ofToDataPath("tempTTSData"));
	if (file.exists()) {
		file.remove(true);
	}
	return file.createDirectory();
}

void ofxGoogleTTS::loadSuccess(std::string& name)
{
	if (0 != mPlayers.count(name)) {
		mPlayers[name].setVolume(mVol);
		mPlayers[name].play();
	}
}

void ofxGoogleTTS::loadFailed(std::string& name)
{
	if (0 != mPlayers.count(name)) mPlayers.erase(name);
}

//////////////////////////////////////////////////////////////////////////////////////

void ofxGoogleTTS::ofThreadedTTSLoader::setup(const std::string& proxyHost, int proxyPort)
{
	Poco::URI uri(GOOGLE_TTS_URI);
	mSession.setHost(uri.getHost());
	mSession.setPort(uri.getPort());
	if (0 != proxyHost.size()) {
		mSession.setProxy(proxyHost, proxyPort);
	}
}

void ofxGoogleTTS::ofThreadedTTSLoader::entry(ofSoundPlayer& player, const std::string& playerName, const std::string& fileName, const std::string& text, Language language)
{
	ofPlayerLoaderEntry entry(player, playerName, fileName, text, language);
	lock();
	mPlayerEntries.push_back(entry);
	unlock();
}

void ofxGoogleTTS::ofThreadedTTSLoader::threadedFunction()
{
	while(isThreadRunning()) {
		lock();
		for (std::list<ofPlayerLoaderEntry>::iterator it=mPlayerEntries.begin(); it!=mPlayerEntries.end(); ++it) {
			mPlayerProcEntries.push_back(*it);
		}
		mPlayerEntries.clear();
		unlock();
		
		if (mPlayerProcEntries.size()) {
			ofPlayerLoaderEntry& e(mPlayerProcEntries.front());

			if (save(ofToDataPath(e.fileName), e.text, e.language)) {
				if (e.player.loadSound(e.fileName))
				{
					ofNotifyEvent(mLoadSuccess, e.playerName);
				} else {
					ofNotifyEvent(mLoadFailed, e.playerName);
				}
			} else {
				ofNotifyEvent(mLoadFailed, e.playerName);
			}
			mPlayerProcEntries.pop_front();
		}

		lock();
		if ((mPlayerEntries.size() == 0) && (mPlayerProcEntries.size() == 0)) {
			stopThread();
		}
		unlock();
	}
}

bool ofxGoogleTTS::ofThreadedTTSLoader::save(const std::string& fileName, const std::string& text, Language language)
{
	Poco::URI uri(GOOGLE_TTS_URI);
	const std::string lang(getLangTag(language));
	std::string data(uri.getPathAndQuery() + uriEncode("?tl=" + lang + "&q=" + text));
	
	Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET, data, Poco::Net::HTTPMessage::HTTP_1_1);
	Poco::Net::HTTPResponse response;
	mSession.sendRequest(request);
	std::istream& rs(mSession.receiveResponse(response));
	std::string soundData;
	if (response.getStatus() != Poco::Net::HTTPResponse::HTTP_UNAUTHORIZED) {
		Poco::StreamCopier::copyToString(rs, soundData);
		std::ofstream of(fileName, std::ios::binary);
		of << soundData << std::endl;
		return true;
	}
	return false;
}

void ofxGoogleTTS::ofThreadedTTSLoader::clear()
{
	lock();
	stopThread();
	mPlayerEntries.clear();
	mPlayerProcEntries.clear();
	unlock();
}

std::string ofxGoogleTTS::ofThreadedTTSLoader::uriEncode( const std::string& str ) const
{
	std::string reserved, encodedStr;
	Poco::URI::encode(str, reserved, encodedStr);
	return encodedStr;
}

std::string ofxGoogleTTS::getLangTag( Language lang )
{
	switch (lang) {
	case LANGUAGE_ENGLISH:
		return "en";
	case LANGUAGE_JAPANESE:
		return "ja";
	case LANGUAGE_ESPANOL:
		return "es";
	case LANGUAGE_ITALIAN:
		return "it";
	case LANGUAGE_FRENCH:
		return "fr";
	case LANGUAGE_GERMAN:
		return "de";
	case LANGUAGE_RUSSIAN:
		return "ru";
	case LANGUAGE_DUTCH:
		return "nl";
	case LANGUAGE_GREEK:
		return "el";
	case LANGUAGE_KOREAN:
		return "ko";	
	};
	return "";
}

int ofxGoogleTTS::mRefCount=0;

