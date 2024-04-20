
#include "main.h"

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <memory.h>
#include "SoundEngine.h"
#include "fmod.h"

/*	--------------------------------------------------------------------------
@brief	Initialize Sound Manager
@param	rate				frequency of the sounds to play
@param	channels			stereo or mono sound
@param	useDefaultDevice	use the default sound device or find the most capable device
	-------------------------------------------------------------------------- */
SoundManager::SoundManager( unsigned rate, unsigned channels, bool useDefaultDevice )
	:	m_mySounds( NULL ),
		m_numSounds( 0 ),
		m_Init( false ),
		Default_Volume( 127 )
{
	// check we have the correct .dll
/*	if( FSOUND_GetVersion() < FMOD_VERSION )
	{
		return;
	}
*/
	// we prefer to use DirectSound, but if we must, use windows libraries
	FSOUND_SetOutput( FSOUND_OUTPUT_DSOUND );
	if( FSOUND_GetOutput() != FSOUND_OUTPUT_DSOUND )
	{
		FSOUND_SetOutput( FSOUND_OUTPUT_WINMM );
		if( FSOUND_GetOutput() != FSOUND_OUTPUT_WINMM )
		{
			FSOUND_SetOutput( FSOUND_OUTPUT_NOSOUND );
			return;
		}
	}

	// Get a device
	int Device = 0;
	if( !useDefaultDevice )
	{
		// check each device
		unsigned maxscore = 0;
		for( int i = 0; i < FSOUND_GetNumDrivers(); ++i )
		{
			unsigned score = 0;
			unsigned caps = 0;
			FSOUND_GetDriverCaps( i, &caps );

			// weigh each of the device's abilities
			if( caps & FSOUND_CAPS_HARDWARE )
				score += 4;
			if( caps & FSOUND_CAPS_EAX2 )
				score += 2;
			if( caps & FSOUND_CAPS_EAX3 )
				score += 1;

			// if this device is more capable than the previous, prefer to use this
			if( score > maxscore )
			{
				Device = i;
				maxscore = score;
			}
		}
	}
	FSOUND_SetDriver( Device );
	FSOUND_SetMixer( FSOUND_MIXER_AUTODETECT );

	if( !Init( rate, channels ) )
		return;

	m_Init = true;
}

/*	--------------------------------------------------------------------------
@brief	De-Initialize Sound Manager
	-------------------------------------------------------------------------- */
SoundManager::~SoundManager( void )
{
	UnLoadObjects();
	FSOUND_Close();
}

/*	--------------------------------------------------------------------------
@brief	Initialize Sound Manager
@param	rate		frequency of the sounds to play
@param	channels	stereo or mono sound
@return	true if successful
	-------------------------------------------------------------------------- */
bool SoundManager::Init( unsigned rate, unsigned channels )
{
	return ( FSOUND_Init( rate, channels, 0 ) != 0 ); 
}

/*	--------------------------------------------------------------------------
@brief	loads a sound object into memory
@param	p_file	the filename of the sound object
@param	p_map	remaps the index of the sound file into the sound map
@return	the index of the sound file, the index of the sound map if sound map is used
	-------------------------------------------------------------------------- */
signed SoundManager::LoadObject( char *p_file, SoundMap *p_map )
{
	return Load2DObject( p_file, p_map );
}

/*	--------------------------------------------------------------------------
@brief	loads a sound object into memory
@param	p_file	the filename of the sound object
@param	p_map	remaps the index of the sound file into the sound map
@return	the index of the sound file, the index of the sound map if sound map is used
	-------------------------------------------------------------------------- */
signed SoundManager::Load2DObject( char *p_file, SoundMap *p_map )
{
	return LoadObject( p_file, p_map, false );
}

/*	--------------------------------------------------------------------------
@brief	loads a mono sound object into memory
@param	p_file	the filename of the sound object
@param	p_map	remaps the index of the sound file into the sound map
@return	the index of the sound file, the index of the sound map if sound map is used
	-------------------------------------------------------------------------- */
signed SoundManager::Load3DObject( char *p_file, SoundMap *p_map )
{
	return LoadObject( p_file, p_map, true );
}

/*	--------------------------------------------------------------------------
@brief	unloads a sound object from memory
@param	i		the index of the sound file
@param	p_map	remaps the index from the sound map
	-------------------------------------------------------------------------- */
void SoundManager::UnLoadObject( signed i, SoundMap *p_map )
{
	// if we have a map, get the map's index according to the passed index
	if( p_map )
		i = p_map->RemoveSoundIndex(i);

	// test that the index is valid
	if( i < 0 || i >= m_numSounds )
		return;

	// unload the object
	switch( m_mySounds[i].SoundType )
	{
		default:
			break;

		case SoundInfo::Type::Music:
			FMUSIC_FreeSong( m_mySounds[i].SoundHandle.Music );
			break;

		case SoundInfo::Type::Sound:
			FSOUND_Sample_Free( m_mySounds[i].SoundHandle.Sound );
			break;

		case SoundInfo::Type::Stream:
			FSOUND_Stream_Close( m_mySounds[i].SoundHandle.Stream );
			break;
	}
	m_mySounds[i].SoundType = SoundInfo::Type::UnInit;
}

/*	--------------------------------------------------------------------------
@brief	plays a sound object
@param	i		the index of the sound file
@param	p_map	remaps the index from the sound map
@return	if the sound file exists
	-------------------------------------------------------------------------- */
bool SoundManager::PlayObject( signed i, SoundMap *p_map )
{
	// if we use a map, get the index from the map
	if( p_map )
		i = p_map->ConvertIndex(i);

	// check that index is valid
	if( i < 0 || i >= m_numSounds )
		return false;

	// play sound based on type
	switch( m_mySounds[i].SoundType )
	{
		default:
			return false;
			break;

		case SoundInfo::Type::Music:
			ResetSoundEffects();
			FMUSIC_PlaySong( m_mySounds[i].SoundHandle.Music );
			m_mySounds[i].Channel = -1;
			break;

		case SoundInfo::Type::Sound:
			m_mySounds[i].Channel = static_cast<char>( FSOUND_PlaySound( FSOUND_FREE, m_mySounds[i].SoundHandle.Sound ) );
			break;

		case SoundInfo::Type::Stream:
			m_mySounds[i].Channel = static_cast<char>( FSOUND_Stream_Play( FSOUND_FREE, m_mySounds[i].SoundHandle.Stream ) );
			break;
	}
	ResetSoundEffects();
	SetVolume( i, Default_Volume );

	// Scan for any sounds that were using the newest channel that was used for sound
	for( signed it = 0; it < m_numSounds; ++it )
	{
		if( it == i )
			continue;
		if( m_mySounds[i].Channel == m_mySounds[it].Channel )
			m_mySounds[it].Channel = -1;
	}

	return true;
}

/*	--------------------------------------------------------------------------
@brief	stops playing a sound object
@param	i		the index of the sound file
@param	p_map	remaps the index from the sound map
@return	if the sound file exists
	-------------------------------------------------------------------------- */
bool SoundManager::StopObject( signed i, SoundMap *p_map )
{
	// if we use a map, get the index from the map
	if( p_map )
		i = p_map->ConvertIndex(i);

	// check that index is valid
	if( i < 0 || i >= m_numSounds )
		return false;

	// stop sound based on type
	switch( m_mySounds[i].SoundType )
	{
		default:
			return false;
			break;

		case SoundInfo::Type::Music:
			ResetSoundEffects();
			FMUSIC_StopSong( m_mySounds[i].SoundHandle.Music );
			break;

		case SoundInfo::Type::Sound:
			if( m_mySounds[i].Channel >= 0 && m_mySounds[i].Channel < 32 )
				FSOUND_StopSound( m_mySounds[i].Channel );
			break;

		case SoundInfo::Type::Stream:
			FSOUND_Stream_Stop( m_mySounds[i].SoundHandle.Stream );
			break;
	}
	m_mySounds[i].Channel = -1;
	ResetSoundEffects();
	return true;
}

/*	--------------------------------------------------------------------------
@brief	compares two strings up to a certain length to compare their equality
@param	a		string 1
@param	b		string 2
@param	len		maximum length to compare
@return	if string 1 and string 2 are equal, returns true
	-------------------------------------------------------------------------- */
bool SoundManager::TagCompare( unsigned char *a, unsigned char *b, unsigned len )
{
	// compare tags, like strcmp()
	for( unsigned i = 0; i < len; ++i )
	{
		if( a[i] != b[i] )
			return false;
	}
	return true;
}

/*	--------------------------------------------------------------------------
@brief	compares two strings up to a certain length to compare their equality
@param	a		string 1
@param	b		string 2
@param	len		maximum length to compare
@return	if string 1 and string 2 are equal, returns true
	-------------------------------------------------------------------------- */
bool SoundManager::TagCompare( unsigned char *a, char *b, unsigned len )
{
	// compare tags, like strcmp()
	for( unsigned i = 0; i < len; ++i )
	{
		if( static_cast<char>( a[i] /* & 0x7F */ ) != b[i] )
			return false;
	}
	return true;
}

/*	--------------------------------------------------------------------------
@brief	unloads sound objects
@param	p_map	if given, unloads sound objects from the map only
	-------------------------------------------------------------------------- */
void SoundManager::UnLoadObjects( SoundMap *p_map )
{
	if( p_map )
	{
		// unload only the sounds in the soundmap
		for( signed i = 0; i < p_map->GetNumSounds(); ++i )
			UnLoadObject( p_map->RemoveSoundIndex(i) );
		p_map->RemoveSounds();
	}
	else
	{
		// unload all sounds
		for( signed i = 0; i < m_numSounds; ++i )
			UnLoadObject(i);
		free( m_mySounds );

		// not for any important reason
		m_mySounds = NULL;
		m_numSounds = 0;
	}
}

/*	--------------------------------------------------------------------------
@brief	adjusts the 3d properties of the next sound source
@param	Cam		location of the viewer
@param	Src		location of the sound emitting object
@param	Vel		velocity, as change in distance over time(not frames), of the source
	-------------------------------------------------------------------------- */
void SoundManager::Set3DProperties( float *Cam, float *Src, float *Vel )
{
	float temp[3];
	temp[0] = Src[0] - Cam[0];
	temp[1] = Src[1] - Cam[1];
	temp[2] = Src[2] - Cam[2];
	Set3DProperties( temp, Vel );
}

/*	--------------------------------------------------------------------------
@brief	adjusts the 3d properties of the next sound source
@param	RelPos	the relative distance of the object based on that the viewer is at (0,0,0)
@param	Vel		velocity, as change in distance over time(not frames), of the source
	-------------------------------------------------------------------------- */
void SoundManager::Set3DProperties( float *RelPos, float *Vel )
{
	FSOUND_3D_Listener_SetAttributes( RelPos, Vel, 0, 0, 1.0f, 0, 1.0f, 0 );
	FSOUND_Update();
}

/*	--------------------------------------------------------------------------
@brief	sets reverb properties of sound object
@param	i		i=0, reverb off; i=1, underwater reverb
	-------------------------------------------------------------------------- */
void SoundManager::SetReverb( unsigned i )
{
//	FSOUND_Sample_SetMinMaxDistance( samp1, 4.0f, 10000.0f );
	switch( i )
	{
		case 1:
		{
			FSOUND_REVERB_PROPERTIES props = FSOUND_PRESET_UNDERWATER;
			FSOUND_Reverb_SetProperties( &props );
		}	break;

		default:
		{
			FSOUND_REVERB_PROPERTIES props = FSOUND_PRESET_OFF;
			FSOUND_Reverb_SetProperties( &props );
		}	break;
	}
	
}

/*	--------------------------------------------------------------------------
@brief	tests if a sound is playing
@param	i		index of the sound
@param	p_map	remaps the index of the sound file into the sound map
@return	true if the sound is playing
	-------------------------------------------------------------------------- */
bool SoundManager::IsPlaying( int i, SoundMap *p_map )
{
	if( p_map )
		i = p_map->ConvertIndex(i);

	if( i < 0 || i >= m_numSounds || m_mySounds[i].Channel < 0 || m_mySounds[i].Channel >= 32 )
		return false;

	if( FSOUND_IsPlaying(m_mySounds[i].Channel) == TRUE )
		return true;
	m_mySounds[i].Channel = -1;
	return false;
}

/*	--------------------------------------------------------------------------
@brief	gets the volume of a sound object
@param	i		index of the sound
@param	p_map	remaps the index of the sound file into the sound map
@return	new volume the sound will have
	-------------------------------------------------------------------------- */
int SoundManager::GetVolume( signed i, SoundMap *p_map )
{
	if( p_map )
		i = p_map->ConvertIndex(i);

	if( i >= 0 && i < m_numSounds && m_mySounds[i].Channel < 0 || m_mySounds[i].Channel >= 32 )
			return -1;

	int v = FSOUND_GetVolume(m_mySounds[i].Channel);
	return ( v != 0 && FSOUND_GetError() == FMOD_ERR_NONE ) ? v : -1;
}

/*	--------------------------------------------------------------------------
@brief	sets the volume of a sound object
@param	i		index of the sound
@param	v		new volume
@param	p_map	remaps the index of the sound file into the sound map
	-------------------------------------------------------------------------- */
void SoundManager::SetVolume( signed i, unsigned char v,  SoundMap *p_map )
{
	if( p_map )
		i = p_map->ConvertIndex(i);

	if( i >= 0 && i < m_numSounds && m_mySounds[i].Channel >= 0 && m_mySounds[i].Channel < 32 )
		FSOUND_SetVolume( m_mySounds[i].Channel, v );
}

/*	--------------------------------------------------------------------------
@brief	undoes any sound adjustments made in effect to reverb,3d effects
	-------------------------------------------------------------------------- */
void SoundManager::ResetSoundEffects( void )
{
	SetReverb(0);
	float t[3] = { 0, 0, 0 };
	Set3DProperties(t,t);
}

/*	--------------------------------------------------------------------------
@brief	loads an object
@param	p_file		filename of sound
@param	p_map		store index of sound object in sound map
@param	p_3dSound	forces the sound object to a mono format so it can be 3d adjusted	
@return	index of sound in sound list, or index of sound in sound map
	-------------------------------------------------------------------------- */
signed SoundManager::LoadObject( char *p_file, SoundMap *p_map, bool p_3dSound )
{
	unsigned		mySndAttrib = FSOUND_NORMAL;// | FSOUND_LOOP_NORMAL;
	unsigned		fileSize;
	unsigned char	buffer[4];
	FILE *			file;
	SoundInfo		newSound;
	SoundInfo *		temp;
	bool			inserted;
	signed			i;
	const char *	ext = Extension(p_file);
	unsigned char	sType = 0;

	// check that file exists
	file = fopen( p_file, "rb" );
	if( !file )
		return -1;

	// get file size
	fileSize = 0;
	if( !fseek( file, 0, SEEK_END ) )
	{
		fileSize = ftell( file );
		fseek( file, 0, SEEK_SET );
	}

	// attempt to get file type
	fread( &buffer[0], sizeof( buffer[0] ), 4, file );
	fclose( file );

	// set sound container
	memset( &newSound, 0, sizeof(newSound) );

	// because I don't know how to make a string of it using ""s
	unsigned char mp3tag[3] = { 0xff, 0xfb, 0x90 };

	// find file format
	if( TagCompare( &buffer[0], "RIFF", 4 ) || !strcmp(ext,"wav" ) )
		sType = 1;	// WAVE
	else if( TagCompare(&buffer[0],"ID3",3) || TagCompare(&buffer[0],&mp3tag[0],3) || !strcmp(ext,"mp3") )
		sType = 2;	// MP3

	// load file
	if( sType == 1 || sType == 2 )
	{
		// by default, don't stream
		bool Stream = false;

		// stream for large files or mp3s
		if( fileSize > 1024*1024 || sType == 2 )
			Stream = true;

		// if we want to use this as a 3d sound object, we need to force to mono
		if( p_3dSound )
			mySndAttrib |= FSOUND_HW3D | FSOUND_FORCEMONO;

		// set up sound object based if we will stream or not
		if( Stream )
		{
			// streamed sound file
			mySndAttrib |= FSOUND_STREAMABLE;
			newSound.SoundHandle.Stream = FSOUND_Stream_Open( p_file, mySndAttrib, 0, 0 );
			newSound.SoundType = SoundInfo::Type::Stream;
			if( newSound.SoundHandle.Stream == NULL )
				return -1;
		}
		else
		{
			// pre-caches sound file
			newSound.SoundHandle.Sound = FSOUND_Sample_Load( FSOUND_FREE, p_file, mySndAttrib, 0, 0 );
			newSound.SoundType = SoundInfo::Type::Sound;
			if( newSound.SoundHandle.Sound == NULL )
				return -1;
		}
	}
	else
	{
		// music sound file, most likely
		newSound.SoundHandle.Music = FMUSIC_LoadSong( p_file );
		newSound.SoundType = SoundInfo::Type::Music;
		if( newSound.SoundHandle.Music == NULL )
			return -1;
	}

	// adjust sound array
	inserted = false;

	// check if we have an empty spot to place the sound file
	for( i = 0; i < m_numSounds; ++i )
	{
		if( m_mySounds[i].SoundType == SoundInfo::Type::UnInit )
		{
			memcpy( &m_mySounds[i], &newSound, sizeof(SoundInfo) );
			inserted = true;
		}
	}

	// grow the list for a new sound object
	if( !inserted )
	{
		temp = static_cast<SoundInfo *>( malloc( sizeof(SoundInfo) * (m_numSounds+1) ) );
		if( m_numSounds )
			memcpy( temp, m_mySounds, sizeof(SoundInfo)*m_numSounds );
		memcpy( &temp[m_numSounds], &newSound, sizeof(SoundInfo) );
		free( m_mySounds );
		m_mySounds = temp;
		++m_numSounds;
	}

	// add sound reference to map
	return ( p_map )?( p_map->AddSoundIndex(i) ):( i );
}

/*	--------------------------------------------------------------------------
@brief	returns the point on the string where the extension begins
@param	p	filename + folderpath
@return	pointer on p where the extension begins
	-------------------------------------------------------------------------- */
const char *SoundManager::Extension( const char *p ) const
{
	unsigned s = strlen(p);

	// scan backwards
	for( --s; s >= 0; --s )
		if( p[s] == '.' )
			return &p[s+1];
	return NULL;
}

/*
	==========================================================================
*/

/*	--------------------------------------------------------------------------
@brief	initialize sound map
	-------------------------------------------------------------------------- */
SoundMap::SoundMap( void )
	:	m_SoundIndex_out( NULL ),
		m_numSounds( 0 )
{
	//
}

/*	--------------------------------------------------------------------------
@brief	de-initialize sound map
	-------------------------------------------------------------------------- */
SoundMap::~SoundMap( void )
{
	RemoveSounds();
}

/*	--------------------------------------------------------------------------
@brief	adds a sound index to the sound map
@param	i	sound list index to add to sound map
@return	the sound map's index of the sound list index
	-------------------------------------------------------------------------- */
signed SoundMap::AddSoundIndex( signed i )
{
	signed *temp;

	// check if we have a free index
	for( int j = 0; j < m_numSounds; ++j )
	{
		if( m_SoundIndex_out[j] == -1 )
		{
			m_SoundIndex_out[j] = i;
			return j;
		}
	}

	// create new list of indicies
	temp = static_cast<signed *>( malloc( sizeof(signed) * (m_numSounds+1) ) );
	// copy previous data
	if( m_numSounds )
		memcpy( temp, m_SoundIndex_out, sizeof(SoundInfo)*m_numSounds );
	// insert new index
	temp[m_numSounds] = i;
	// readjust pointers
	free( m_SoundIndex_out );
	m_SoundIndex_out = temp;
	// return then increment # of sounds/current sound index
	return m_numSounds++;
}

/*	--------------------------------------------------------------------------
@brief	removes index of sound map index
@param	i	the index of the sound in the sound map
@return	the index of the sound list index
	-------------------------------------------------------------------------- */
signed SoundMap::RemoveSoundIndex( signed i )
{
	// remove the index to the sound, does not free the sound
	if( i >= 0 && i < m_numSounds )
	{
		signed t = m_SoundIndex_out[i];
		m_SoundIndex_out[i] = -1;
		return t;
	}
	return -1;
}

/*	--------------------------------------------------------------------------
@brief	removes all sounds from the sound index
	-------------------------------------------------------------------------- */
void SoundMap::RemoveSounds( void )
{
	// unload all sounds
	for( signed i = 0; i < m_numSounds; ++i )
		RemoveSoundIndex(i);
	free( m_SoundIndex_out );

	// not for any important reason
	m_SoundIndex_out = NULL;
	m_numSounds = 0;
}



void GetJukeboxSongs(std::vector< std::string > &songList)
{
std::string      jukeboxPath = "data/jukebox/";
WIN32_FIND_DATA  fd;  // Find data.
HANDLE           fh;  // Find handle.
std::string      path1  = jukeboxPath + "*.mp3";
std::string      path2  = jukeboxPath + "*.m3u";
FILE *readfile, *check;
char buffer[262];
int c;

  // Clear the list.
  songList.clear();

  fh = ::FindFirstFile(path1.c_str(),&fd);
  if(fh != INVALID_HANDLE_VALUE)
  {
    do
    {
      // Do not find directories.
      if(!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
       songList.push_back( jukeboxPath + fd.cFileName );
    }
    while(::FindNextFile(fh,&fd));
  }

  // Clean up.
  ::FindClose(fh);

  // load winamp playlist files
  fh = ::FindFirstFile(path2.c_str(),&fd);
  if(fh != INVALID_HANDLE_VALUE)
  {
    do
    {
		// open winamp playlist
		readfile = fopen( ( jukeboxPath + fd.cFileName ).c_str(), "rt" );
		if( readfile == NULL )
			continue;

		// verify
		fgets( &buffer[0], 8, readfile );
		if( strcmp( &buffer[0], "#EXTM3U" ) )
			continue;
		fgetc(readfile);	// skip end of line
		
		// read 
		while( !feof(readfile) )
		{
			// clear buffer
			memset( &*buffer, 0, sizeof(buffer) );

			// scan line for filename
			unsigned int i = 0;
			do	{
				// get character
				c = fgetc(readfile);
				// if still in buffer space, add character to buffer
				if( i < sizeof(buffer) / sizeof(buffer[0]) - 1 )
					buffer[i] = static_cast<char>( c );
				// next character
				++i;
			}	while( c != '\n' && c != '\0' && c != -1 );

			// filename was a #EXT tag
			if( buffer[0] == '#' )
				continue;

			// filename was invalid and indicated EOF
			if( buffer[0] == '\0' )
				break;

			// remove odd characters
			for( c = 0; c < 262; ++c )
			{
				if( buffer[c] == '\n' )
					buffer[c] = '\0';
				if( buffer[c] == '\0' )
					break;
			}
			// verify file exists
			check = fopen( &buffer[0], "rb" );
			if( check != NULL )
			{
				// finally add song into songlist
				songList.push_back( &buffer[0] );
				fclose( check );
			}
		}
		fclose( readfile );
    }
    while(::FindNextFile(fh,&fd));
  }
  
  // Clean up.
  ::FindClose(fh);
}

