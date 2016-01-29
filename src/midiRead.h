#ifndef MIDIREAD_H_INCLUDED
#define MIDIREAD_H_INCLUDED

/*
 * midiRead.h - structs and functional headers to store the midi info.
 *
 */

typedef struct noteEvent noteEvent;
struct noteEvent{
        int pitch;
        int absStart;
        int absDuration;
        int track;
        int channel;
};

typedef struct noteList noteList;
struct noteList{
        noteEvent* notes;
        int length;
};

typedef struct timeSig timeSig;
struct timeSig {
        int numerator;
        int denominator;
        int delta;
};

typedef struct key key;
struct key {
	int keyNum;
	int keyType;
};

void sortNotes(noteList* myNotes);
void swap(noteList* myNotes, int a, int b);
void quickSortNotes(noteList* myNotes, int start, int end);
void initializeFile(char** midiInput, FILE** in);
void readMidi(FILE* f, key* pieceKey, timeSig* pieceTime, noteList* rv);
void copyEventFields( noteEvent* a, noteEvent b );
void printAll(noteList notes, key pieceKey, timeSig pieceTime, char* inputFile);


#endif //MIDIREAD_H_INCLUDED 
