CC=gcc
TARGETS=AnalyzeMidi

main: src/AnalyzeMidi.c src/midiRead.* src/midicsv
	gcc -o AnalyzeMidi src/midiRead.c src/AnalyzeMidi.c

clean:
	rm -f $(TARGETS)
