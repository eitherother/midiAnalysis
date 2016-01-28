/* 
 * midiRead.c - functions to read data from text file produced by
 *              midicsv and store that data into custom structs
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include "midiRead.h"

void copyEventFields( noteEvent* a, noteEvent b ) {
	(*a).pitch = b.pitch;
	(*a).absStart = b.absStart;
	(*a).absDuration = b.absDuration;
	(*a).channel = b.channel;
	(*a).track = b.track;
}

// quick sort algorithm, sorted on absolute starting time
void swap(noteList* myNotes, int a, int b) {
	noteEvent temp;
	copyEventFields(&temp, (*myNotes).notes[a]);
	copyEventFields(&((*myNotes).notes[a]), (*myNotes).notes[b]);
	copyEventFields(&((*myNotes).notes[b]), temp);
}	

void quickSortNotes(noteList* myNotes, int start, int end) {
	if (start < end) {
		int l = start+1;
		int r = end;
		int pivot = (*myNotes).notes[start].absStart;

		while(l<r) {
			if ((*myNotes).notes[l].absStart <= pivot) {
				l++;
			}
			else if ((*myNotes).notes[r].absStart >= pivot) {
				r--;
			}
			else {
				swap(myNotes, l, r);
			}
		}
		if((*myNotes).notes[l].absStart < pivot) {
			swap(myNotes, l, start);
			l--;
		}
		else {
			l--;
			swap(myNotes, l, start);
		}
		quickSortNotes(myNotes, start, l);
		quickSortNotes(myNotes, r, end);
	}
}

void readMidi(FILE* f, key* pieceKey, timeSig* pieceTime, noteList* rv) {
	int arrSize = 1;  // array size
	int i = 0;        // index
	int val1, val2, val3, val4, val5; // integers to store midi data
	char line[100];   // buffer to store each line from file
	char* pch;       // pointer to store each value from line
	int total = 0;   // total number of notes

	// allocate piece struct and array of notes
	noteEvent *notes = (noteEvent*) malloc(arrSize * sizeof(noteEvent));

	// parse file one line at a time
	while(!feof(f)) {
		if (fgets(line, 100, f) != NULL) {
			// get the first two integers
			pch = strtok(line, ", ");
			val1 = atoi(pch);
			pch = strtok(NULL, ", ");
			val2 = atoi(pch);
			pch = strtok(NULL, ", ");

			// Check the third value in the line
			if (strcmp(pch, "Note_on_c") == 0) {
				// get the remaining values
				pch = strtok(NULL, ", ");
				val3 = atoi(pch);
				pch = strtok(NULL, ", ");
				val4 = atoi(pch);
				pch = strtok(NULL, ", ");
				val5 = atoi(pch);

				// if this is actually a note-off event...
				if (val5 == 0) {
					// Search for the correct struct and set off time
					for (i = 0; i < total; i++) {
						if (val1 == notes[i].track &&
						    val3 == notes[i].channel &&
						    val4 == notes[i].pitch &&
						    notes[i].absDuration == -1) {
							notes[i].absDuration = val2 - notes[i].absStart;
						break;
						}
					}
				}
				else {  // else, allocate new note and set fields
					if (total >= arrSize) {
						arrSize = arrSize * 2;
						notes = (noteEvent*) realloc(notes, arrSize * sizeof(noteEvent));
					}
					notes[total].pitch = val4;
					notes[total].absStart = val2;
					notes[total].absDuration = -1;
					notes[total].channel = val3;
					notes[total].track = val1;
					total++;
				}
			}
			else if (strcmp(pch, "Note_off_c") == 0) {
				pch = strtok(NULL, ", ");
				val3 = atoi(pch);
				pch = strtok(NULL, ", ");
				val4 = atoi(pch);
				for (i = 0; i < total; i++) {
					if (val1 == notes[i].track &&
					    val3 == notes[i].channel &&
					    val4 == notes[i].pitch &&
					    notes[i].absDuration == -1) {
						notes[i].absDuration = val2 - notes[i].absStart;
						break;
					}
				}
			}
			else if (strcmp(pch, "Time_signature") == 0) {
				pch = strtok(NULL, ", ");
				pieceTime->numerator = atoi(pch);
				pch = strtok(NULL, ", ");
				pieceTime->denominator =  1 << atoi(pch);
				pch = strtok(NULL, ", ");
			}
			else if(strcmp(pch, "Key_signature") == 0) {
				pch = strtok(NULL, ", ");

				int keyNum =  atoi(pch);
				pch = strtok(NULL, ",\" ");

				int keyType;
				if(strcmp(pch,"major")==0){
					keyType = 0;
				}
				else{
					keyType = 1;
				}
				pieceKey->keyNum = keyNum;
				pieceKey->keyType = keyType;

			}
			else if (strcmp(pch, "Header") == 0) {
				pch = strtok(NULL, ", ");
				pch = strtok(NULL, ", ");
				pch = strtok(NULL, ", ");
				pieceTime->delta = atoi(pch);
			}
		}
	}
	
	rv->notes = notes;
	rv->length = total;
	return;
}

void initializeFile(char** midiInput, FILE** in) {
	pid_t child, wpid;
	int status;
	int fd;

	// Fork a process
	if ((child = fork()) < 0) {
		perror("Could not fork process");
	}

	// child runs midicsv on argv[0], and redirects the
	// output to a file called temp_midi.txt
	if (child == 0) {
		if ((fd = open("temp_midi.txt", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR)) < 0) {
			perror("open");
		}
		if (dup2(fd, 1) < 0) {
			perror("dup2");
		}
		if (close(fd) < 0) {
			perror("close");
		}
		execve("./src/midicsv", midiInput, 0);
	}
	// parent waits for child to finish
	else if ((wpid = waitpid(child, &status, 0)) < 0) {
		perror("waitpid error");
	}

	// open text generatated by MIDICSV program
	if ( (*in = fopen("temp_midi.txt", "r")) == NULL) {
		perror("fopen");
	}

	// fork a process to remove temp_midi.txt
	if ((child = fork()) < 0) {
		perror("Could not fork process");
	}

	// child removes temp_midi.txt
	if (child == 0) {
		execl("/bin/rm", "/bin/rm", "temp_midi.txt", NULL);
	}
	// parent waits for child to finish
	else if ((wpid = waitpid(child, &status, 0)) < 0) {
		perror("waitpid error");
	}

	return;
}

void printAll(noteList notes, key pieceKey, timeSig pieceTime, char * inputFile) {
	int i;
	// Optional declaration for bassline extraction
	// noteList bassLine;
	char fileout[80];
	char * filename;
	int directories = 0;
	char * pch;

	// create output file name based on input midi file name
	strncpy(fileout, "results/", 10);
	pch = strchr(inputFile, '/');
	while (pch != NULL) {
		directories++;
		pch = strchr(pch+1,'/');
	}
	filename = strtok(inputFile, "/");
	for (i = 0; i < directories; i++) {
		filename = strtok(NULL, "/");
	}
	filename = strtok(filename, ".");
	strcat(filename, ".txt");
	strcat(fileout, filename);
	FILE* out = fopen(fileout, "w");

	// print to file
	fprintf(out, "***** Key Sig and Time Sig Info *****\n\n");
	fprintf(out, "KeyNum (in range -7 to 7): %d, KeyType (major or minor): %d\n\n", pieceKey.keyNum, pieceKey.keyType);
	fprintf(out, "Time Sig: %d / %d, Division: %d\n\n", pieceTime.numerator, 
           pieceTime.denominator, pieceTime.delta);

	fprintf(out, "***** Pitches *****\n\n");
	fprintf(out, "Number of Notes: %d\n\n", notes.length);
	for (i = 0; i < notes.length; i++) {
		fprintf(out, "Pitch %d  Start: %d  End: %d  Track: %d  Chan: %d\n",
			notes.notes[i].pitch,
			notes.notes[i].absStart,
			notes.notes[i].absDuration,
			notes.notes[i].track,
			notes.notes[i].channel);
	}

	fclose(out);
	printf("\nAnalysis has been output to %s\n\n", fileout);
}
