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

#define MAX_BUF_SIZE	128
#define MAX_SAMPLES	65536

static char *snd_device = "default";
snd_pcm_t *capture_handle;
snd_pcm_hw_params_t *hw_params;

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
	unsigned int srate = 44100;

	if ((err = snd_pcm_open(&capture_handle, snd_device,
			SND_PCM_STREAM_CAPTURE, 0)) < 0)
	{
		fprintf(stderr, "cannot open audio device %s (%s)\n",
				snd_device, snd_strerror(err));
		return OPEN_ERROR;
	}

	if ((err = snd_pcm_hw_params_malloc(&hw_params)) < 0)
	{
		fprintf(
				stderr,
				"cannot allocate hardware parameter structure (%s)\n",
				snd_strerror(err));
		return MALLOC_ERROR;
	}

	if ((err = snd_pcm_hw_params_any(capture_handle, hw_params)) < 0)
	{
		fprintf(
				stderr,
				"cannot initialize hardware parameter structure (%s)\n",
				snd_strerror(err));
		return ANY_ERROR;
	}

	if ((err = snd_pcm_hw_params_set_access(capture_handle, hw_params,
			SND_PCM_ACCESS_RW_INTERLEAVED)) < 0)
	{
		fprintf(stderr, "cannot set access type (%s)\n",
				snd_strerror(err));
		return ACCESS_ERROR;
	}

	if ((err = snd_pcm_hw_params_set_format(capture_handle, hw_params,
			SND_PCM_FORMAT_U8)) < 0)
	{
		fprintf(stderr, "cannot set sample format (%s)\n",
				snd_strerror(err));
		return FORMAT_ERROR;
	}

	if ((err = snd_pcm_hw_params_set_rate_near(capture_handle, hw_params,
			&srate, 0)) < 0)
	{
		fprintf(stderr, "cannot set sample rate (%s)\n",
				snd_strerror(err));
		return RATE_ERROR;
	}

	if ((err = snd_pcm_hw_params_set_channels(capture_handle, hw_params, 1))
			< 0)
	{
		fprintf(stderr, "cannot set channel count (%s)\n",
				snd_strerror(err));
		return CHANNELS_ERROR;
	}

	if ((err = snd_pcm_hw_params(capture_handle, hw_params)) < 0)
	{
		fprintf(stderr, "cannot set parameters (%s)\n",
				snd_strerror(err));
		return PARAMS_ERROR;
	}

	snd_pcm_hw_params_free(hw_params);

	if ((err = snd_pcm_prepare(capture_handle)) < 0)
	{
		fprintf(
				stderr,
				"cannot prepare audio interface for use (%s)\n",
				snd_strerror(err));
		return PREPARE_ERROR;
	}

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
	wav_h.NumChannels = 1;
	wav_h.BitsPerSample = 8;
	wav_h.Subchunk2Size = MAX_SAMPLES * (uint32_t) wav_h.NumChannels
			* (uint32_t) wav_h.BitsPerSample / 8;
	wav_h.ChunkSize = (uint32_t) wav_h.Subchunk2Size + 36;
	wav_h.Subchunk1Size = 16;
	wav_h.AudioFormat = 1;
	wav_h.SampleRate = 44100;
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
	fwav = fopen("test_wav1.wav", "wb");
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
	char wav_data[MAX_BUF_SIZE];

	for (i = 0; i < ncount; i++)
	{
		cout << "Start" << endl;
		if ((err = snd_pcm_readi(capture_handle, wav_data, MAX_BUF_SIZE)) != MAX_BUF_SIZE)
		{
			fprintf(
					stderr,
					"read from audio interface failed (%s)\n",
					snd_strerror(err));
			return SNDREAD_ERROR;
		}
		cout << "Read" << endl;
		if (fwav != NULL)
		{
			fwrite(wav_data, 1, MAX_BUF_SIZE, fwav);
		}
		else
		{
			fprintf(stderr, "cannot write data to file\n");
			return FOPEN_ERROR;
		}

		cout << i << endl;
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
