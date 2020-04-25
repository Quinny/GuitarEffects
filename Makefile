run:
	clang++ main.cpp -lrtAudio -std=c++11 && ./a.out

debug:
	clang++ main.cpp -lrtAudio -std=c++11 -g3
