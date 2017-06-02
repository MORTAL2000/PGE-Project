/*
  SDL_mixer:  An audio mixer library based on the SDL library
  Copyright (C) 1997-2017 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

  This is the source needed to decode an MP3 into a waveform.
*/

/* $Id$ */

#if defined(MP3_MUSIC) || defined(MP3_MAD_MUSIC)

#include <SDL_mixer_ext/SDL_mixer_ext.h>

#include "load_mp3.h"

#if defined(MP3_MUSIC)
#include "dynamic_mp3.h"
#elif defined(MP3_MAD_MUSIC)
#include "music_mad.h"
#endif

SDL_AudioSpec *Mix_LoadMP3_RW(SDL_RWops *src, int freesrc, SDL_AudioSpec *spec, Uint8 **audio_buf, Uint32 *audio_len)
{
    /* note: spec is initialized to mixer spec */
    #if defined(MP3_MUSIC)
    SMPEG *mp3 = NULL;
    SMPEG_Info info;
    #elif defined(MP3_MAD_MUSIC)
    mad_data *mp3_mad = NULL;/* Sam, you forgot to initialize this :P */
    #endif
    long samplesize;
    int read_len = 0;
    const Uint32 chunk_len = 4096;
    int err = 0;
    int errisset = 0;
    char *errStr = NULL;

#define LoadMP3Err(err) if(!errisset) { errStr=(char*)err; Mix_SetError(err); errisset=1; }

    if((!src) || (!spec) || (!audio_buf) || (!audio_len))
        return NULL;

    if(!err)
    {
        *audio_len = 0;
        *audio_buf = (Uint8 *) SDL_malloc(chunk_len);
        err = (*audio_buf == NULL);
    }

    if(!err)
        err = ((Mix_Init(MIX_INIT_MP3) & MIX_INIT_MP3) == 0);
    else LoadMP3Err("Out of memory")

        if(!err)
        {
            #if defined(MP3_MUSIC)
            mp3 = smpeg.SMPEG_new_rwops(src, &info, freesrc, 0);
            err = (mp3 == NULL);
            #elif defined(MP3_MAD_MUSIC)
            mp3_mad = mad_openFileRW(src, spec, freesrc);
            err = (mp3_mad == NULL);
            #endif
        }
        else LoadMP3Err("Can't initialize MP3")

        #if defined(MP3_MUSIC)
            if(!err)
                err = !info.has_audio;
        #endif

    if(!err)
    {
        Uint8 *tempBuff;
        /* for "read once for audio length" */
        int hits = 0;
        int left = 0;

        #if defined(MP3_MUSIC)

        smpeg.SMPEG_actualSpec(mp3, spec);

        smpeg.SMPEG_enableaudio(mp3, 1);
        smpeg.SMPEG_enablevideo(mp3, 0);

        smpeg.SMPEG_play(mp3);

        /* read once for audio length */
        while((read_len = smpeg.SMPEG_playAudio(mp3, *audio_buf, chunk_len)) > 0)
            * audio_len += read_len;

        smpeg.SMPEG_stop(mp3);

        #elif defined(MP3_MAD_MUSIC)

        mad_start(mp3_mad);

        tempBuff = (Uint8 *)SDL_malloc(chunk_len);
        err = (tempBuff == NULL);

        /* read once for audio length */
        if(!err)
        {
            while(mad_isPlaying(mp3_mad))
            {
                left = mad_getSamples(mp3_mad, tempBuff, chunk_len);
                if(left >= 0)
                {
                    read_len = chunk_len - left;
                    *audio_buf = (Uint8 *)SDL_realloc(*audio_buf, *audio_len + read_len);
                    err = (*audio_buf == NULL);
                    if(!err)
                    {
                        memcpy(*audio_buf + *audio_len, tempBuff, read_len);
                        *audio_len += read_len;
                    }
                }
                hits++;
            }
        }
        SDL_free(tempBuff);
        mad_stop(mp3_mad);

        #endif
        err = (read_len < 0);
    }
    else
        LoadMP3Err("Can't open file (mayby not exists or you has no permissions)")

        if(!err)
        {
            /* Don't return a buffer that isn't a multiple of samplesize */
            samplesize = ((spec->format & 0xFF) / 8) * spec->channels;
            *audio_len &= ~(samplesize - 1);
        }
        else LoadMP3Err("Error ocouped while reading samples")

        #if defined(MP3_MUSIC)
            if(mp3)
            {
                smpeg.SMPEG_delete(mp3);
                mp3 = NULL;
                /* Deleting the MP3 closed the source if desired */
                freesrc = SDL_FALSE;
            }
        #elif defined(MP3_MAD_MUSIC)
            if(mp3_mad)
            {
                mad_closeFile(mp3_mad);
                mp3_mad = NULL;
                /* Deleting the MP3 closed the source if desired */
                freesrc = SDL_FALSE;
            }
        #endif

    if(freesrc)
    {
        SDL_RWclose(src);
        src = NULL;
    }

    /* handle error */
    if(err)
    {
        if(*audio_buf != NULL)
        {
            SDL_free(*audio_buf);
            *audio_buf = NULL;
        }
        *audio_len = 0;
        spec = NULL;
    }
    return spec;
}

#endif
