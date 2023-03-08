COMPILER = g++

COMPILE_FLAGS = -lpthread \
								-lboost_system \
								-I crow/include \
								-std=c++1z \
							 	-I . \
								-D CROW_DISABLE_STATIC_DIR \
								-I cycfi/Q/q_lib/include \
								-I cycfi/infra/include

MATPLOT_FLAGS = -I/usr/include/python2.7 \
								-I/System/Library/Frameworks/Python.framework/Versions/2.7/Extras/lib/python/numpy/core/include \
								-lpython2.7

UNAME := $(shell uname)
ifeq ($(UNAME), Linux)
	# GCC complains about some ABI differences in one of the libraries, but it
	# doesn't appear to cause issues.
	COMPILE_FLAGS += -Wno-psabi
	# Linux and OSX spell RtAudio differently for some maddening reason.
	COMPILE_FLAGS += -lrtaudio
	# Linux and OSX link in SDL2 differently
	COMPILE_FLAGS += `sdl2-config --cflags --libs`
endif
ifeq ($(UNAME), Darwin)
	COMPILE_FLAGS += -lrtAudio
	COMPILE_FLAGS += -lsdl2
endif


makedir:
	mkdir -p bin

all: makedir record server

plot: makedir
	${COMPILER} plot.cpp ${COMPILE_FLAGS} ${MATPLOT_FLAGS} -o ./bin/plot

server: makedir
	${COMPILER} web/main.cpp ${COMPILE_FLAGS} -o ./bin/server

record: makedir
	${COMPILER} record.cpp ${COMPILE_FLAGS} -o ./bin/record

run: all
	./bin/pedalboard
