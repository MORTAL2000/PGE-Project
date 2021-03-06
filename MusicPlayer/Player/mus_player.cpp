#include "mus_player.h"

#include <QMessageBox>
#include <QSettings>
#include "../MainWindow/musplayer_qt.h"

#include "../wave_writer.h"

/*!
 *  SDL Mixer wrapper
 */
namespace PGE_MusicPlayer
{
    static MusPlayer_Qt *mw = nullptr;

    Mix_Music *s_playMus = nullptr;
    Mix_MusicType type  = MUS_NONE;
    bool reverbEnabled = false;

    static bool g_playlistMode = false;
    static int  g_loopsCount = -1;
    static bool g_hooksDisabled = false;

    static int      g_sample_rate = MIX_DEFAULT_FREQUENCY;
    static Uint16   g_sample_format = MIX_DEFAULT_FORMAT;
    static int      g_channels = MIX_DEFAULT_CHANNELS;

    static void musicStoppedHook();

    static const char *format_to_string(Uint16 format)
    {
        switch(format)
        {
        case AUDIO_U8:
            return "U8";
        case AUDIO_S8:
            return "S8";
        case AUDIO_U16LSB:
            return "U16LSB";
        case AUDIO_S16LSB:
            return "S16LSB";
        case AUDIO_U16MSB:
            return "U16MSB";
        case AUDIO_S16MSB:
            return "S16MSB";
        case AUDIO_S32MSB:
            return "S32MSB";
        case AUDIO_S32LSB:
            return "S32LSB";
        case AUDIO_F32MSB:
            return "F32MSB";
        case AUDIO_F32LSB:
            return "F32LSB";
        default:
            return "Unknown";
        }
    }

    void loadAudioSettings()
    {
        QSettings setup;
        g_sample_rate = setup.value("Audio-SampleRate", 44100).toInt();
        g_sample_format = static_cast<Uint16>(setup.value("Audio-SampleFormat", AUDIO_S16).toUInt());
        g_channels = setup.value("Audio-Channels", 2).toInt();
    }

    void saveAudioSettings()
    {
        QSettings setup;
        setup.setValue("Audio-SampleRate", g_sample_rate);
        setup.setValue("Audio-SampleFormat", g_sample_format);
        setup.setValue("Audio-Channels", g_channels);
        setup.sync();
    }

    int getSampleRate()
    {
        return g_sample_rate;
    }

    Uint16 getSampleFormat()
    {
        return g_sample_format;
    }

    int getChannels()
    {
        return g_channels;
    }

    void setSpec(int rate, Uint16 format, int channels)
    {
        g_sample_rate = rate;
        g_sample_format = format;
        g_channels = channels;
    }

    bool openAudio(QString &error)
    {
        return openAudioWithSpec(error, g_sample_rate, g_sample_format, g_channels);
    }

    bool openAudioWithSpec(QString &error, int rate, Uint16 format, int channels)
    {
        if(Mix_OpenAudio(rate, format, channels, 4096) == -1)
        {
            error = QString("Failed to open audio stream: %1\n\n"
                            "Given spec:\n"
                            "- Sample rate %2\n"
                            "- Sample format %3\n"
                            "- Channels count %4\n")
                    .arg(Mix_GetError())
                    .arg(rate)
                    .arg(format_to_string(format))
                    .arg(channels);
            return false;
        }
        Mix_AllocateChannels(16);
        return true;
    }

    void closeAudio()
    {
        if(mw)
            QMetaObject::invokeMethod(mw, "musicStopped", Qt::QueuedConnection);
        Mix_CloseAudio();
    }

    bool reloadAudio(QString &error)
    {
        closeAudio();
        return openAudio(error);
    }

    void initHooks()
    {
        Mix_HookMusicFinished(&musicStoppedHook);
    }

    void setMainWindow(void *mwp)
    {
        mw = reinterpret_cast<MusPlayer_Qt*>(mwp);
    }

    const char *musicTypeC()
    {
        return (
                   type == MUS_NONE ? "No Music" :
                   type == MUS_CMD ? "CMD" :
                   type == MUS_WAV ? "PCM Wave" :
                   type == MUS_MOD ? "Tracker music" :
                   type == MUS_MID ? "MIDI" :
                   type == MUS_OGG ? "OGG" :
                   type == MUS_MP3 ? "MP3" :
                   type == MUS_FLAC ? "FLAC" :
#ifdef SDL_MIXER_X
#   if SDL_MIXER_MAJOR_VERSION > 2 || \
    (SDL_MIXER_MAJOR_VERSION == 2 && SDL_MIXER_MINOR_VERSION >= 2)
                   type == MUS_OPUS ? "OPUS" :
#   endif
                   type == MUS_ADLMIDI ? "IMF/MUS/XMI" :
                   type == MUS_GME ? "Game Music Emulator" :
#else
#   if SDL_MIXER_MAJOR_VERSION > 2 || \
    (SDL_MIXER_MAJOR_VERSION == 2 && SDL_MIXER_MINOR_VERSION > 0) || \
    (SDL_MIXER_MAJOR_VERSION == 2 && SDL_MIXER_MINOR_VERSION == 0 && SDL_MIXER_PATCHLEVEL >= 4)
                   type == MUS_OPUS ? "OPUS" :
#   endif
#endif
                   "<Unknown>");
    }
    QString musicType()
    {
        return QString(musicTypeC());
    }

    /*!
     * \brief Spawn warning message box with specific text
     * \param msg text to spawn in message box
     */
    static void error(QString msg)
    {
        QMessageBox::warning(nullptr,
                             "SDL2 Mixer ext error",
                             msg,
                             QMessageBox::Ok);
    }

    void stopMusic()
    {
        Mix_HaltMusicStream(PGE_MusicPlayer::s_playMus);
    }

    void disableHooks()
    {
        g_hooksDisabled = true;
    }

    void enableHooks()
    {
        g_hooksDisabled = false;
    }

    QString getMusTitle()
    {
        if(s_playMus)
            return QString(Mix_GetMusicTitle(s_playMus));
        else
            return QString("[No music]");
    }

    QString getMusArtist()
    {
        if(s_playMus)
            return QString(Mix_GetMusicArtistTag(s_playMus));
        else
            return QString("[Unknown Artist]");
    }

    QString getMusAlbum()
    {
        if(s_playMus)
            return QString(Mix_GetMusicAlbumTag(s_playMus));
        else
            return QString("[Unknown Album]");
    }

    QString getMusCopy()
    {
        if(s_playMus)
            return QString(Mix_GetMusicCopyrightTag(s_playMus));
        else
            return QString("");
    }

    static void musicStoppedHook()
    {
        if(g_hooksDisabled)
            return;
        if(mw)
            QMetaObject::invokeMethod(mw, "musicStopped", Qt::QueuedConnection);
    }

    void setMusicLoops(int loops)
    {
        g_loopsCount = loops;
    }

    bool playMusic()
    {
        if(s_playMus)
        {
            if(Mix_PlayMusic(s_playMus, g_playlistMode ? 0 : g_loopsCount) == -1)
            {
                error(QString("Mix_PlayMusic: ") + Mix_GetError());
                return false;
                // well, there's no music, but most games don't break without music...
            }
        }
        else
        {
            error(QString("Play nothing: Mix_PlayMusic: ") + Mix_GetError());
            return false;
        }
        return true;
    }

    void changeVolume(int volume)
    {
        Mix_VolumeMusicStream(PGE_MusicPlayer::s_playMus, volume);
        DebugLog(QString("Mix_VolumeMusic: %1\n").arg(volume));
    }

    bool openFile(QString musFile)
    {
        type = MUS_NONE;
        if(s_playMus != nullptr)
        {
            Mix_FreeMusic(s_playMus);
            s_playMus = nullptr;
        }

        QByteArray p = musFile.toUtf8();
        s_playMus = Mix_LoadMUS(p.data());
        if(!s_playMus)
        {
            error(QString("Mix_LoadMUS(\"" + QString(musFile) + "\"): ") + Mix_GetError());
            return false;
        }
        type = Mix_GetMusicType(s_playMus);
        DebugLog(QString("Music type: %1").arg(musicType()));
        return true;
    }




    static void* s_wavCtx = nullptr;

    // make a music play function
    // it expects udata to be a pointer to an int
    static void myMusicPlayer(void *udata, Uint8 *stream, int len)
    {
        ctx_wave_write(udata, reinterpret_cast<const short *>(stream), len / 2);
    }

    void startWavRecording(QString target)
    {
        if(s_wavCtx)
            return;
        if(!s_playMus)
            return;

        /* Record 20 seconds to wave file */
        s_wavCtx = ctx_wave_open(44100, target.toLocal8Bit().data());
        ctx_wave_enable_stereo(s_wavCtx);
        Mix_SetPostMix(myMusicPlayer, s_wavCtx);
    }

    void stopWavRecording()
    {
        if(!s_wavCtx)
            return;
        ctx_wave_close(s_wavCtx);
        s_wavCtx = nullptr;
        Mix_SetPostMix(nullptr, nullptr);
    }

    bool isWavRecordingWorks()
    {
        return (s_wavCtx != nullptr);
    }
}
