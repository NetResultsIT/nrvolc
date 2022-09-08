#include "NrVolumeChangerLinux.h"

#include <QDebug>
#include <alsa/asoundlib.h>

NrVolumeChangerLinuxImpl::NrVolumeChangerLinuxImpl(QObject *parent)
    : NrVolumeChanger(parent)
{

}

int NrVolumeChangerLinuxImpl::setDefaultInputVolume(double percent)
{

    long min, max;
    snd_mixer_t *handle;
    snd_mixer_selem_id_t *sid;
    const char *card = "default:2";
    const char *selem_name = "Mic";

    snd_mixer_open(&handle, 0);
    snd_mixer_attach(handle, card);
    snd_mixer_selem_register(handle, NULL, NULL);
    snd_mixer_load(handle);

    snd_mixer_selem_id_alloca(&sid);
    snd_mixer_selem_id_set_index(sid, 0);
    snd_mixer_selem_id_set_name(sid, selem_name);
    snd_mixer_elem_t* elem = snd_mixer_find_selem(handle, sid);

    snd_mixer_selem_get_capture_volume_range(elem, &min, &max);
    snd_mixer_selem_set_capture_volume_all(elem, percent * max / 100);

    snd_mixer_close(handle);
    return 0;
}

int NrVolumeChangerLinuxImpl::setDefaultOutputVolume(double percent)
{

    char **hints;
    /* Enumerate sound devices */
    int err = snd_device_name_hint(-1, "pcm", (void***)&hints);
    if (err != 0)
       return -1;//Error! Just return

    char** n = hints;
    while (*n != NULL) {

        char *name = snd_device_name_get_hint(*n, "NAME");

        if (name != NULL && 0 != strcmp("null", name)) {
            //Copy name to another buffer and then free it
            qDebug() << name;
            free(name);
        }
        n++;
    }//End of while

    //Free hint buffer too
    snd_device_name_free_hint((void**)hints);

    long min, max;
    snd_mixer_t *handle;
    snd_mixer_selem_id_t *sid;
    const char *card = "default:1";
    const char *selem_name = "Master";

    snd_mixer_open(&handle, 0);
    snd_mixer_attach(handle, card);
    snd_mixer_selem_register(handle, NULL, NULL);
    snd_mixer_load(handle);

    snd_mixer_selem_id_alloca(&sid);
    snd_mixer_selem_id_set_index(sid, 0);
    snd_mixer_selem_id_set_name(sid, selem_name);
    snd_mixer_elem_t* elem = snd_mixer_find_selem(handle, sid);

    snd_mixer_selem_get_playback_volume_range(elem, &min, &max);
    qDebug() << "output min max range: " << min << max;
    snd_mixer_selem_set_playback_volume_all(elem, percent * max /100);

    snd_mixer_close(handle);

    return 0;
}



double NrVolumeChangerLinuxImpl::getDefaultInputVolume() const
{


    long min, max;
    snd_mixer_t *handle;
    snd_mixer_selem_id_t *sid;
    const char *card = "default:2";
    const char *selem_name = "Mic";

    snd_mixer_open(&handle, 0);
    snd_mixer_attach(handle, card);
    snd_mixer_selem_register(handle, NULL, NULL);
    snd_mixer_load(handle);

    snd_mixer_selem_id_alloca(&sid);
    snd_mixer_selem_id_set_index(sid, 0);
    snd_mixer_selem_id_set_name(sid, selem_name);
    snd_mixer_elem_t* elem = snd_mixer_find_selem(handle, sid);

    snd_mixer_selem_channel_id_t channel = SND_MIXER_SCHN_MONO;
    snd_mixer_selem_get_capture_volume(elem, channel, &max);

    qDebug() << "read value from alsa mixer input:" << max;
    snd_mixer_close(handle);
    return max;
}

double NrVolumeChangerLinuxImpl::getDefaultOutputVolume() const
{
    long min, max, vol = 0;
    snd_mixer_t *handle;
    snd_mixer_selem_id_t *sid;
    const char *card = "default:1";
    const char *selem_name = "Master";

    snd_mixer_open(&handle, 0);
    snd_mixer_attach(handle, card);
    snd_mixer_selem_register(handle, NULL, NULL);
    snd_mixer_load(handle);

    snd_mixer_selem_id_alloca(&sid);
    snd_mixer_selem_id_set_index(sid, 0);
    snd_mixer_selem_id_set_name(sid, selem_name);
    snd_mixer_elem_t* elem = snd_mixer_find_selem(handle, sid);

    snd_mixer_selem_channel_id_t channel = SND_MIXER_SCHN_FRONT_LEFT;
    snd_mixer_selem_get_playback_volume(elem, channel, &vol);
    qDebug() << "read value from alsa mixer output:" << vol << " channel:" << channel;
    snd_mixer_selem_get_playback_volume_range(elem, &min, &max);
    qDebug() << "output min max range: " << min << max;
    snd_mixer_close(handle);
    qDebug() << max-vol << max-min << (float)((max-vol) / ((max-min)*1.0)) << ((max-vol) / (max-min) * 100);
    return 100 - (((max-vol)*1.0 / (max-min)*1.0) * 100);
}




