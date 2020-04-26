COMPILE_FLAGS = -lrtAudio -std=c++11 -I .

all: pedalboard record

pedalboard:
	clang++ pedalboard.cpp ${COMPILE_FLAGS} -o ./bin/pedalboard

record:
	clang++ record.cpp ${COMPILE_FLAGS} -o ./bin/record

run: all
	./bin/pedalboard
