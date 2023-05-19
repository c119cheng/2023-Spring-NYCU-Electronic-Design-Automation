
CC = g++
CFLAGS =  -std=c++11 -O3
MAIN_FILE = main.o
LINK_FILES = FM.o
EXE_FILE = Lab1

#---------------------

all: $(LINK_FILES) $(MAIN_FILE)
	$(CC) -o $(EXE_FILE) $^

%.o: %.cpp
	$(CC) $(CFLAGS) -c $<

demo1:
	time ./$(EXE_FILE) input.hgr
	./verifier input.hgr output.txt

demo2:
	time ./$(EXE_FILE) input2.txt.hgr
	./verifier input2.txt.hgr output.txt

demo3:
	time ./$(EXE_FILE) input3.txt.hgr
	./verifier input3.txt.hgr output.txt

demo4:
	time ./$(EXE_FILE) input4.txt.hgr
	./verifier input4.txt.hgr output.txt

s1:
	./shmetis input.hgr 2 5
s2:
	./shmetis input2.txt.hgr 2 5
s3:
	./shmetis input3.txt.hgr 2 5

clean:
	rm $(MAIN_FILE) $(LINK_FILES) $(EXE_FILE)