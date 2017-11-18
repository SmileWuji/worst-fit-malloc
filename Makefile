CC = gcc
CFLAGS = -c -std=c99 -Wall 

all: wfmalloc

wfmalloc: wf_malloc.o wf_malloc_test.o
	$(CC) wf_malloc.o wf_malloc_test.o -o wfmalloc

wf_malloc.o: wf_malloc.c wf_malloc.h
	$(CC) $(CFLAGS) wf_malloc.c wf_malloc.h

wf_malloc_test.o: wf_malloc_test.c wf_malloc.h
	$(CC) $(CFLAGS) wf_malloc_test.c wf_malloc.h

clean:
	rm -f *.o *.gch wfmalloc
