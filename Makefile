all: general

# targets das outras partes vem pra ca tbm
general: upload

upload: upload.o hashing.o
	g++ upload.o hashing.o -o upload -lboost_regex

upload.o: upload.cpp
	g++ -c upload.cpp --std=c++14 -g

hashing.o: hashing.cpp
	g++ -c hashing.cpp --std=c++14 -g
