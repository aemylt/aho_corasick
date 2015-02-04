CMPHLIB=-L/usr/local/lib/libcmph.la -lcmph

all:
	gcc -O3 -Wall -o aho_corasick aho_corasick.c

clean:
	rm aho_corasick

hash-lookup:
	$(CC) $(CARGS) hash_lookup.c -o hash_lookup $(CMPHLIB)

hash-lookup-clean:
	rm hash_lookup