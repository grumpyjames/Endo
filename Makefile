SHELL = /bin/bash
.SUFFIXES:
.SUFFIXES: .cpp .o
all: 
	g++ -Wall -g -I../boost_1_40_0/ save_endo.cpp dna_string.cpp -o save_endo