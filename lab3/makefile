
CC = g++
CFLAGS = 
MAIN_FILE = main.o
LINK_FILES = ROBDD.o
EXE_FILE = Lab3
CUBDD_FLAG1 = cudd-3.0.0/cudd/
CUBDD_FLAG2 = cudd-3.0.0/util/
CUBDD_FLAG3 = cudd-3.0.0/cudd/.libs/libcudd.a


#---------------------
all: $(LINK_FILES) $(MAIN_FILE)
	$(CC) -I $(CUBDD_FLAG1) -I $(CUBDD_FLAG2) $(LINK_FILES) $(MAIN_FILE) $(CUBDD_FLAG3) -o $(EXE_FILE)

%.o: %.cpp
	$(CC) -I $(CUBDD_FLAG1) -I $(CUBDD_FLAG2) $(CFLAGS) -c $<

run1:
	./$(EXE_FILE) ./case1/case1.txt ./case1/my_case1.out

run2:
	./$(EXE_FILE) ./case2/case2.txt ./case2/my_case2.out

run3:
	./$(EXE_FILE) ./case3/case3.txt ./case3/my_case3.out

clean:
	rm $(MAIN_FILE) $(LINK_FILES) $(EXE_FILE)