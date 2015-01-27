all:
	gcc -O3 -Wall -o aho_corasick aho_corasick.c

clean:
	rm aho_corasick