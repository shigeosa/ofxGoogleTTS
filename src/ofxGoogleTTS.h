//
//  ofxGoogleTTS.h
//  Created by Osamu Shigeta on 3/28/2013.
//
#pragma once

#include <string>
#include "ofMain.h"

#include "Poco/URI.h" 
#include "Poco/File.h"
#include "Poco/StreamCopier.h" 
#include "Poco/Net/HTTPClientSession.h"
#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTTPResponse.h"

class ofxGoogleTTS
{
public:
	enum Language
	{
		LANGUAGE_ENGLISH,
		LANGUAGE_JAPANESE,
		LANGUAGE_ESPANOL,
		LANGUAGE_ITALIAN,
		LANGUAGE_FRENCH,
		LANGUAGE_GERMAN,
		LANGUAGE_RUSSIAN,
		LANGUAGE_DUTCH,
		LANGUAGE_GREEK,
		LANGUAGE_KOREAN
	};	
	ofxGoogleTTS();
	~ofxGoogleTTS();
	void setup(const std::string& proxyHost="", int proxyPort=0);
	void exit();
	void speak(const std::string& text, Language language=LANGUAGE_ENGLISH);

	float getVolume() const;
	void setVolume(float vol);	//!< [0.f, 1.f]

	bool save(const std::string& fileName, const std::string& text, Language language=LANGUAGE_ENGLISH);
	bool getSoundPlayer(ofSoundPlayer& player, const std::string& text, Language language=LANGUAGE_ENGLISH);
	bool isSpeaking()/*const*/;

	void clearSoundPlayers();

private:
	std::string getPlayerName(const std::string& lang, const std::string& text) const;
	std::string getFileName(int index) const;
	bool createTempFolder();

	void loadSuccess(std::string& name);
	void loadFailed(std::string& name);

private:
	class  ofThreadedTTSLoader : public ofThread
	{
	private:
		struct ofPlayerLoaderEntry
		{
			ofPlayerLoaderEntry(ofSoundPlayer& player, const std::string& playerName, const std::string& fileName, const std::string text, Language language)
				: player(player), playerName(playerName), fileName(fileName), text(text), language(language) {};
			ofSoundPlayer& player;
			std::string playerName;
			std::string fileName;
			std::string text;
			Language language;
		};
	public:
		void setup(const std::string& proxyHost, int proxyPort);
		void entry(ofSoundPlayer& player, const std::string& playerName, const std::string& fileName, const std::string& text, Language language);
		bool save(const std::string& fileName, const std::string& text, Language language=LANGUAGE_ENGLISH);
		void clear();
		
		ofEvent<std::string> mLoadSuccess;
		ofEvent<std::string> mLoadFailed;
	private:
		virtual void threadedFunction();
		std::string uriEncode(const std::string& str) const;
	private:
		std::list<ofPlayerLoaderEntry> mPlayerEntries;
		std::list<ofPlayerLoaderEntry> mPlayerProcEntries;
		Poco::Net::HTTPClientSession mSession;
	};
	static std::string getLangTag(Language lang);

private:
	std::map<std::string, ofSoundPlayer> mPlayers;
	float mVol;
	ofThreadedTTSLoader mLoader;
	static int mRefCount;
};