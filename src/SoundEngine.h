/*! ========================================================================

      @file    SoundEngine.h
      @author  Brian Rosmond
      @brief   holds sound files and messes with them
      
      (c) 2004 DigiPen (USA) Corporation, all rights reserved.
      
    ======================================================================== */

#ifndef SOUNDENGINE_H
#define SOUNDENGINE_H

#include "fmod.h"

#include <vector>
#include <string>

#pragma comment( lib, "fmodvc.lib" )

// instead of worrying about what the sound type is, call it a sound object
struct SoundInfo
{
	SoundInfo()
		:	Channel(-1), SoundType(UnInit)
	{}

	enum Type { UnInit = 0, Music, Sound, Stream } SoundType;
	char	Channel;

	union
	{
		FMUSIC_MODULE *		Music;
		FSOUND_SAMPLE *		Sound;
		FSOUND_STREAM *		Stream;
	} SoundHandle;
};

// Generally a container of sounds, holds indicies instead of sound
class SoundMap
{
	public:
		SoundMap( void );
		~SoundMap( void );

		// functions that the user will not have to worry about
		signed AddSoundIndex( signed i );
		signed RemoveSoundIndex( signed i );
		void RemoveSounds( void );

		// get the # of sounds loaded
		inline const signed GetNumSounds( void ) const
		{
			return m_numSounds;
		};

		// Convert the index from global to sound index
		inline const signed ConvertIndex( const signed i ) const
		{
			return ( i >= 0 && i < m_numSounds ) ? m_SoundIndex_out[i] : -1;
		};

	private:
		signed *	m_SoundIndex_out;
		signed		m_numSounds;
};

// the main sound class
class SoundManager
{
	public:
		SoundManager( unsigned rate = 22050, unsigned channels = 32, bool useDefaultDevice = true );
		~SoundManager( void );

		// a lot of functions
		bool Init( unsigned rate, unsigned channels );
		signed LoadObject( char *p_file, SoundMap *p_map = NULL );
		signed Load2DObject( char *p_file, SoundMap *p_map = NULL );
		signed Load3DObject( char *p_file, SoundMap *p_map = NULL );
		bool PlayObject( signed i, SoundMap *p_map = NULL );
		bool StopObject( signed i, SoundMap *p_map = NULL );
		void UnLoadObject( signed i, SoundMap *p_map = NULL );
		void UnLoadObjects( SoundMap *p_map = NULL );

		// 3d stuff and reverb functions
		void Set3DProperties( float *Cam, float *Src, float *Vel );
		void Set3DProperties( float *RelPos, float *Vel );
		void SetReverb( unsigned i );
		bool IsPlaying( signed i, SoundMap *p_map = NULL );
		int GetVolume( signed i, SoundMap *p_map = NULL );
		void SetVolume( signed i, unsigned char v, SoundMap *p_map = NULL );

		unsigned char	Default_Volume;

		inline void ResetGlobalVolume( void )
		{	for(signed i = 0;i<32;++i) FSOUND_SetVolume(i,Default_Volume); };

	private:
		// stuff the user won't need to see, ever
		signed LoadObject( char *p_file, SoundMap *p_map, bool p_3dSound );
		bool TagCompare( unsigned char *a, unsigned char *b, unsigned len );
		bool TagCompare( unsigned char *a, char *b, unsigned len );
		void ResetSoundEffects( void );
		const char *Extension( const char *p ) const;

		// all the sound info we have
		SoundInfo	*m_mySounds;
		signed		m_numSounds;
		bool		m_Init;
};

// call GetJukeboxSongs() to get a list of songs that the jukebox
// should play.
// songList will be cleared by this function.
void GetJukeboxSongs(std::vector< std::string > &songList);

/*
void ExampleUsage()
{
	int				i;
	SoundManager	sMan;
	SoundMap		p1,p2;

	i = sMan.LoadObject( "test.wav", &p1 );		// will return 0 if all goes right, sound #0
	i = sMan.LoadObject( "test.mp3", &p1 );		// will return 1 if all goes right, sound #1
	i = sMan.LoadObject( "test2.mp3", &p2 );	// will return 0 if all goes right, sound #2
	sMan.SetReverb(1);
	sMan.PlayObject(i,&p2);						// will play test2.mp3, as i == 0 and p2[0] == "test2.mp3"
	sMan.PlayObject(0);							// will play test.wav, as test.wav is sound 0
	sMan.PlayObject(1,&p1);						// will play test.mp3, as p1[1] == "test.mp3"

	//	Note: MP3's and large WAVs( > 1MB ) will automatically be set to be streamed
}
*/

#endif