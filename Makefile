COMPILER = clang++-3.7

COMPILE_FLAGS = -lpthread \
								-lboost_system \
								-I crow/include \
								-std=c++1z \
								-I . \
								-I cycfi/Q/q_lib/include \
								-I cycfi/infra/include \
								-lsdl2

MATPLOT_FLAGS = -I/usr/include/python2.7 \
								-I/System/Library/Frameworks/Python.framework/Versions/2.7/Extras/lib/python/numpy/core/include \
								-lpython2.7

UNAME := $(shell uname)
ifeq ($(UNAME), Linux)
	# Linux doesn't support clang, so swap the compiler to gcc.
	COMPILER = g++
	# GCC complains about some ABI differences in one of the libraries, but it
	# doesn't appear to cause issues.
	COMPILE_FLAGS += -Wno-psabi
	# Linux and OSX spell RtAudio differently for some maddening reason.
	COMPILE_FLAGS += -lrtaudio
endif
ifeq ($(UNAME), Darwin)
	COMPILE_FLAGS += -lrtAudio
endif

all: record server

plot:
	${COMPILER} plot.cpp ${COMPILE_FLAGS} ${MATPLOT_FLAGS} -o ./bin/plot

server:
	${COMPILER} web/main.cpp ${COMPILE_FLAGS} -o ./bin/server

record:
	${COMPILER} record.cpp ${COMPILE_FLAGS} -o ./bin/record

run: all
	./bin/pedalboard
