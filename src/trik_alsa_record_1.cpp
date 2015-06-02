//============================================================================
// Name        : trik_motors_function_1.cpp
// Author      : Rostislav Varzar
// Version     :
// Copyright   : Free license
// Description : C++, Ansi-style
//============================================================================

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <alsa/asoundlib.h>

using namespace std;

#define NO_ERROR	0
#define OPEN_ERROR	1
#define MALLOC_ERROR	2
#define ANY_ERROR	3
#define ACCESS_ERROR	4
#define FORMAT_ERROR	5
#define RATE_ERROR	6
#define CHANNELS_ERROR	7
#define PARAMS_ERROR	8
#define PREPARE_ERROR	9
#define FOPEN_ERROR	10
#define FCLOSE_ERROR	11
#define SNDREAD_ERROR	12
#define START_ERROR	13

#define MAX_BUF_SIZE	512
//#define MAX_SAMPLES	38400
#define MAX_SAMPLES	256000

static char* snd_device = "default";
snd_pcm_t* capture_handle;
snd_pcm_hw_params_t* hw_params;
snd_pcm_info_t* s_info;
unsigned int srate = 44100;
unsigned int nchan = 2;
char* wav_name = "test_wav11.wav";

FILE* fwav; // Input file descriptor
struct wav_header // Wav file header structure
{
	uint8_t ChunkID[4];
	uint32_t ChunkSize;
	uint8_t Format[4];
	uint8_t Subchunk1ID[4];
	uint32_t Subchunk1Size;
	uint16_t AudioFormat;
	uint16_t NumChannels;
	uint32_t SampleRate;
	uint32_t ByteRate;
	uint16_t BlockAlign;
	uint16_t BitsPerSample;
	uint8_t Subchunk2ID[4];
	uint32_t Subchunk2Size;
};
struct wav_header wav_h; // Wav header data

/// Open and init default sound card params
int init_soundcard()
{
	int err = 0;

	if ((err = snd_pcm_open(&capture_handle, snd_device,
			SND_PCM_STREAM_CAPTURE, 0)) < 0)
	{
		fprintf(stderr, "cannot open audio device %s (%s, %d)\n",
				snd_device, snd_strerror(err), err);
		return OPEN_ERROR;
	}

	if ((err = snd_pcm_hw_params_malloc(&hw_params)) < 0)
	{
		fprintf(
				stderr,
				"cannot allocate hardware parameter structure (%s, %d)\n",
				snd_strerror(err), err);
		return MALLOC_ERROR;
	}

	if ((err = snd_pcm_hw_params_any(capture_handle, hw_params)) < 0)
	{
		fprintf(
				stderr,
				"cannot initialize hardware parameter structure (%s, %d)\n",
				snd_strerror(err), err);
		return ANY_ERROR;
	}

	if ((err = snd_pcm_hw_params_set_access(capture_handle, hw_params,
			SND_PCM_ACCESS_RW_INTERLEAVED)) < 0)
	{
		fprintf(stderr, "cannot set access type (%s, %d)\n",
				snd_strerror(err), err);
		return ACCESS_ERROR;
	}

	if ((err = snd_pcm_hw_params_set_format(capture_handle, hw_params,
			SND_PCM_FORMAT_S16_LE)) < 0)
	{
		fprintf(stderr, "cannot set sample format (%s, %d)\n",
				snd_strerror(err), err);
		return FORMAT_ERROR;
	}

	if ((err = snd_pcm_hw_params_set_rate_near(capture_handle, hw_params,
			&srate, 0)) < 0)
	{
		fprintf(stderr, "cannot set sample rate (%s, %d)\n",
				snd_strerror(err), err);
		return RATE_ERROR;
	}

	if ((err = snd_pcm_hw_params_set_channels(capture_handle, hw_params, nchan))
			< 0)
	{
		fprintf(stderr, "cannot set channel count (%s, %d)\n",
				snd_strerror(err), err);
		return CHANNELS_ERROR;
	}

	if ((err = snd_pcm_hw_params(capture_handle, hw_params)) < 0)
	{
		fprintf(stderr, "cannot set parameters (%s, %d)\n",
				snd_strerror(err), err);
		return PARAMS_ERROR;
	}

	if ((err = snd_pcm_prepare(capture_handle)) < 0)
	{
		fprintf(
				stderr,
				"cannot prepare audio interface for use (%s, %d)\n",
				snd_strerror(err), err);
		return PREPARE_ERROR;
	}
	/*
	cout << "Parameters of PCM:" << endl;
	cout << snd_pcm_name(capture_handle) << endl;
	cout << snd_pcm_type(capture_handle) << endl;
	cout << snd_pcm_stream(capture_handle) << endl;
	cout << snd_pcm_poll_descriptors_count(capture_handle) << endl;
	cout << snd_pcm_state(capture_handle) << endl;
	cout << snd_pcm_avail(capture_handle) << endl;
	cout << snd_pcm_avail_update(capture_handle) << endl;
	cout << snd_pcm_rewindable(capture_handle) << endl;
	cout << snd_pcm_forwardable(capture_handle) << endl;
	cout << "-------------------------------------" << endl;
	cout << snd_pcm_info_malloc(&s_info) << endl;
	cout << snd_pcm_info(capture_handle, s_info) << endl;
	cout << snd_pcm_info_get_device(s_info) << endl;
	cout << snd_pcm_info_get_subdevice(s_info) << endl;
	cout << snd_pcm_info_get_stream(s_info) << endl;
	cout << snd_pcm_info_get_card(s_info) << endl;
	cout << snd_pcm_info_get_id(s_info) << endl;
	cout << snd_pcm_info_get_name(s_info) << endl;
	cout << snd_pcm_info_get_subdevice_name(s_info) << endl;
	cout << snd_pcm_info_get_class(s_info) << endl;
	cout << snd_pcm_info_get_subclass(s_info) << endl;
	cout << snd_pcm_info_get_subdevices_count(s_info) << endl;
	cout << snd_pcm_info_get_subdevices_avail(s_info) << endl;
	cout << "-------------------------------------" << endl;
	cout << snd_pcm_hw_params_current(capture_handle, hw_params) << endl;
	cout << snd_pcm_hw_params_is_double(hw_params) << endl;
	cout << snd_pcm_hw_params_is_batch(hw_params) << endl;
	cout << snd_pcm_hw_params_is_block_transfer(hw_params) << endl;
	cout << snd_pcm_hw_params_is_monotonic(hw_params) << endl;
	cout << snd_pcm_hw_params_can_overrange(hw_params) << endl;
	cout << snd_pcm_hw_params_can_pause(hw_params) << endl;
	cout << snd_pcm_hw_params_can_resume(hw_params) << endl;
	cout << snd_pcm_hw_params_is_half_duplex(hw_params) << endl;
	cout << snd_pcm_hw_params_is_joint_duplex(hw_params) << endl;
	cout << snd_pcm_hw_params_can_sync_start(hw_params) << endl;
	cout << snd_pcm_hw_params_can_disable_period_wakeup(hw_params) << endl;
	cout << snd_pcm_hw_params_get_sbits(hw_params) << endl;
	cout << snd_pcm_hw_params_get_fifo_size(hw_params) << endl;
	cout << "-------------------------------------" << endl;
	unsigned int *tmp1 = (unsigned int *)malloc(sizeof(unsigned int));
	int *tmp2 = (int *)malloc(sizeof(int));
	cout << snd_pcm_hw_params_get_channels(hw_params, tmp1) << endl; cout << *tmp1 << endl;
	cout << snd_pcm_hw_params_get_channels_min(hw_params, tmp1) << endl; cout << *tmp1 << endl;
	cout << snd_pcm_hw_params_get_channels_max(hw_params, tmp1) << endl; cout << *tmp1 << endl;
	cout << snd_pcm_hw_params_get_rate(hw_params, tmp1, tmp2) << endl; cout << *tmp1 << " " << *tmp2 << endl;
	cout << snd_pcm_hw_params_get_rate_min(hw_params, tmp1, tmp2) << endl; cout << *tmp1 << " " << *tmp2 << endl;
	cout << snd_pcm_hw_params_get_rate_max(hw_params, tmp1, tmp2) << endl; cout << *tmp1 << " " << *tmp2 << endl;
	cout << snd_pcm_hw_params_get_rate_resample(capture_handle, hw_params, tmp1) << endl; cout << *tmp1 << endl;
	cout << snd_pcm_hw_params_get_export_buffer(capture_handle, hw_params, tmp1) << endl; cout << *tmp1 << endl;
	cout << snd_pcm_hw_params_get_period_wakeup(capture_handle, hw_params, tmp1) << endl; cout << *tmp1 << endl;
	cout << snd_pcm_hw_params_get_period_time(hw_params, tmp1, tmp2) << endl; cout << *tmp1 << " " << *tmp2 << endl;
	cout << snd_pcm_hw_params_get_period_time_min(hw_params, tmp1, tmp2) << endl; cout << *tmp1 << " " << *tmp2 << endl;
	cout << snd_pcm_hw_params_get_period_time_max(hw_params, tmp1, tmp2) << endl; cout << *tmp1 << " " << *tmp2 << endl;
	*/

	snd_pcm_hw_params_free(hw_params);
	//snd_pcm_info_free(s_info);
	//free(tmp1);
	//free(tmp2);

	return NO_ERROR;
}

/// Close soundcard
int close_soundcard()
{
	return snd_pcm_close(capture_handle);
}

/// Init wav file header
int init_wav_header()
{
	wav_h.ChunkID[0] = 'R';
	wav_h.ChunkID[1] = 'I';
	wav_h.ChunkID[2] = 'F';
	wav_h.ChunkID[3] = 'F';
	wav_h.Format[0] = 'W';
	wav_h.Format[1] = 'A';
	wav_h.Format[2] = 'V';
	wav_h.Format[3] = 'E';
	wav_h.Subchunk1ID[0] = 'f';
	wav_h.Subchunk1ID[1] = 'm';
	wav_h.Subchunk1ID[2] = 't';
	wav_h.Subchunk1ID[3] = ' ';
	wav_h.Subchunk2ID[0] = 'd';
	wav_h.Subchunk2ID[1] = 'a';
	wav_h.Subchunk2ID[2] = 't';
	wav_h.Subchunk2ID[3] = 'a';
	wav_h.NumChannels = nchan;
	wav_h.BitsPerSample = 16;
	wav_h.Subchunk2Size = MAX_SAMPLES * (uint32_t) wav_h.NumChannels
			* (uint32_t) wav_h.BitsPerSample / 8;
	wav_h.ChunkSize = (uint32_t) wav_h.Subchunk2Size + 36;
	wav_h.Subchunk1Size = 16;
	wav_h.AudioFormat = 1;
	wav_h.SampleRate = srate;
	wav_h.ByteRate = (uint32_t) wav_h.SampleRate
			* (uint32_t) wav_h.NumChannels
			* (uint32_t) wav_h.BitsPerSample / 8;
	wav_h.BlockAlign = (uint32_t) wav_h.NumChannels
			* (uint32_t) wav_h.BitsPerSample / 8;

	return NO_ERROR;
}

/// Open wav file and write header
int init_wav_file()
{
	fwav = fopen(wav_name, "wb");
	if (fwav != NULL)
	{
		fwrite(&wav_h, 1, sizeof(wav_h), fwav);
	}
	else
	{
		fprintf(stderr, "cannot open wav file to write data\n");
		return FOPEN_ERROR;
	}

	return NO_ERROR;
}

/// Close wav file
int close_wav_file()
{
	if (fwav != NULL)
	{
		fclose(fwav);
	}
	else
	{
		fprintf(stderr, "cannot close wav file\n");
		return FCLOSE_ERROR;
	}

	return NO_ERROR;
}

/// Do record data to file
int do_record()
{
	uint32_t ncount = MAX_SAMPLES / MAX_BUF_SIZE;
	uint32_t i = 0;
	int err = 0;
	char wav_data[MAX_BUF_SIZE * 4];
	if ((err = snd_pcm_start(capture_handle)) < 0)
	{
		fprintf(
				stderr,
				"cannot start soundcard (%s, %d)\n",
				snd_strerror(err), err);
		return START_ERROR;
	}
	for (i = 0; i < ncount; i++)
	{

		if ((err = snd_pcm_readi(capture_handle, wav_data, MAX_BUF_SIZE)) != MAX_BUF_SIZE)
		{
			fprintf(
					stderr,
					"read from audio interface failed (%s, %d)\n",
					snd_strerror(err), err);
			if (err == -32) // Broken pipe
			{
				if ((err = snd_pcm_prepare(capture_handle)) < 0)
				{
					fprintf(
							stderr,
							"cannot prepare audio interface for use (%s, %d)\n",
							snd_strerror(err), err);
					return PREPARE_ERROR;
				}
			}
			else
			{
				return SNDREAD_ERROR;
			}
		}

		if (fwav != NULL)
		{
			fwrite(wav_data, 1, MAX_BUF_SIZE * 4, fwav);
		}
		else
		{
			fprintf(stderr, "cannot write data to file\n");
			return FOPEN_ERROR;
		}
	}
	return NO_ERROR;
}

int main(int argc, char *argv[])
{
	cout << "Open default soundcard" << endl;
	init_soundcard();

	cout << "Init wave file header" << endl;
	init_wav_header();

	cout << "Open and init wave file" << endl;
	init_wav_file();


	cout << "Record process..." << endl;
	do_record();

	cout << "Close wave file" << endl;
	close_wav_file();

	cout << "Close default soundcard" << endl;
	close_soundcard();

	return 0;
}
