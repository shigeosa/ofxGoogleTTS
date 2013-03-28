#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){
	mTTS.setup();
	// mTTS.setup("proxy", port);
}

//--------------------------------------------------------------
void testApp::exit() {
	mTTS.exit();
}

//--------------------------------------------------------------
void testApp::update(){
}

//--------------------------------------------------------------
void testApp::draw(){
	ofBackgroundGradient(64,128);
	ofDrawBitmapStringHighlight(
		"1-8: speak text \n"
		"9: save mp3 file \n"
		"0: get ofSoundPlayer \n"
		"+: up volume \n"
		"-: down volume \n",
		15,15
		);
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){

	switch (key) {
	case '1':
		mTTS.speak("hello");
		break;
	case '2':
		mTTS.speak("hello", ofxGoogleTTS::LANGUAGE_JAPANESE);
		break;
	case '3':
		mTTS.speak("hello", ofxGoogleTTS::LANGUAGE_ESPANOL);
		break;
	case '4':
		mTTS.speak("hello", ofxGoogleTTS::LANGUAGE_ITALIAN);
		break;
	case '5':
		mTTS.speak("openFrameworks is an open source C++ toolkit for creative coding.");
		break;
	case '6':
		mTTS.speak("But that is easier said than done, especially if the parents have humble origins.");
		break;
	case '7':
		mTTS.speak("The Weather Channel and weather.com provide a national and local weather forecast for cities.");
		break;
	case '8':
		mTTS.speak("Since it is hot today,Å@I'm sure the beer after work will taste wonderful.");
		break;
	case '9':
		mTTS.save(ofToDataPath("savedFile.mp3"), "wellcome to openframeworks");
		break;
	case '0':
		if (mTTS.getSoundPlayer(mPlayer, "get sound player")) {
			mPlayer.play();
		}
		break;
	case '+':
		mTTS.setVolume(mTTS.getVolume() + 0.1f);
		break;
	case '-':
		mTTS.setVolume(mTTS.getVolume() - 0.1f);
		break;
	case 'c':
		mTTS.clearSoundPlayers();
		break;

	}
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 

}