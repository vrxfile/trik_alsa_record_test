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

#define MAX_BUF_SIZE	512

char	stmp1[128];		// Temp string
FILE*	fwav;			// Input file descriptor
struct wav_header		// Wav file header structure
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
struct wav_header wav_h;	// Wav header data
char wav_data[MAX_BUF_SIZE];	// Wav data

int main(int argc, char *argv[])
{
	return 0;
}
