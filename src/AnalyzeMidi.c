/*
 * AnalyzeMidi.c - skeletal code to perform analysis of midi files
 *
 * Note that additional functions must be added to perform the desired
 * analytical tasks.  This framework is particularly suitable for 
 * statistical studies of pitches or intervals.  In its current state, 
 * this code simply prints out all of the note information using the 
 * printAll() function, which is designed for testing purposes only.
 *
 */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "midiRead.h"

int main(int argc, char** argv) {
	FILE* in;
	noteList notes;
	key pieceKey;
	timeSig pieceTime;
	int i;

	if(argc == 1){
		printf("\nNot enough arguments!\nUsage ./MidiHarmonic song.midi\n");
		printf("(Second argument must be path from local directory)\n\n");
		exit(1);
	}

	// Read Midi File
	initializeFile(argv, &in);
	readMidi(in, &pieceKey, &pieceTime, &notes); 

	// Sort notes by start time (quicksort: requires first and last index)
	quickSortNotes(&notes, 0, notes.length-1);

	// For testing: print out all notes to .txt file
	printAll(notes, pieceKey, pieceTime, argv[1]);

	free(notes.notes);

	return 0;
}
