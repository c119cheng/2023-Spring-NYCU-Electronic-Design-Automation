
CC = g++
CFLAGS =  -std=c++11
MAIN_FILE = main.o
LINK_FILES = MR.o
EXE_FILE = lab2

#---------------------

all: $(LINK_FILES) $(MAIN_FILE)
	$(CC) -o $(EXE_FILE) $^

%.o: %.cpp
	$(CC) $(CFLAGS) -c $<

run1:
	./$(EXE_FILE) ./case1/case1.in ./case1/my_case1.out

run2:
	./$(EXE_FILE) ./case2/case2.in ./case2/my_case2.out

run3:
	./$(EXE_FILE) ./case3/case3.in ./case3/my_case3.out

demo1:
	time ./$(EXE_FILE) ./case1/case1.in ./case1/my_case1.out
	./draw ./case1/case1.in ./case1/my_case1.out

demo2:
	time ./$(EXE_FILE) ./case2/case2.in ./case2/my_case2.out
	./draw ./case2/case2.in ./case2/my_case2.out

demo3:
	time ./$(EXE_FILE) ./case3/case3.in ./case3/my_case3.out
	./draw ./case3/case3.in ./case3/my_case3.out

clean:
	rm $(MAIN_FILE) $(LINK_FILES) $(EXE_FILE)