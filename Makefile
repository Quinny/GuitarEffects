COMPILE_FLAGS = -lrtAudio -std=c++11 -I .

all:
	clang++ main.cpp ${COMPILE_FLAGS}

run: all
	./a.out

record:
	clang++ record.cpp ${COMPILE_FLAGS}
