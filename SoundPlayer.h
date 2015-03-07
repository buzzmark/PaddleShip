#ifndef __SoundPlayer_h_
#define __SoundPlayer_h_

#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include <SDL/SDL_audio.h>

class SoundPlayer {
public:
	SoundPlayer();
	void startBgMusic();
	void playPaddleSwing();
	void playScore();
	void playShipHit();

protected:
	Mix_Music *bgMusic;
	Mix_Chunk *paddleSwing;
	Mix_Chunk *score;
	Mix_Chunk *shipHit;

};

#endif // #ifndef __SoundPlayer_h_
