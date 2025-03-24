.phony all:
all: ACS

ACS: main.c linked_list.c
	gcc -pthread -Wall main.c linked_list.c -o ACS

.PHONY clean:
clean:
	-rm -rf *.o *.exe
