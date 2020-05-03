CLANG = clang++-3.7

COMPILE_FLAGS = -lrtAudio -std=c++1z\
								-I .\
								-I cycfi/Q/q_lib/include \
								-I cycfi/infra/include

all: pedalboard record

pedalboard:
	${CLANG} pedalboard.cpp ${COMPILE_FLAGS} -o ./bin/pedalboard

record:
	${CLANG} record.cpp ${COMPILE_FLAGS} -o ./bin/record

run: all
	./bin/pedalboard
