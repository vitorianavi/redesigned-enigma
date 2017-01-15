all: general

# targets das outras partes vem pra ca tbm
general: upload findrec seek1

upload: upload.o hashing.o intbtree.o
	g++ upload.o hashing.o intbtree.o -o upload -lboost_regex

findrec: findrec.o hashing.o
	g++ findrec.o hashing.o -o findrec

seek1: seek1.o hashing.o
	g++ seek1.o hashing.o intbtree.o -o seek1

seek1.o: seek1.cpp
	g++ -c seek1.cpp --std=c++14 -g

findrec.o: findrec.cpp
	g++ -c findrec.cpp --std=c++14 -g

upload.o: upload.cpp
	g++ -c upload.cpp --std=c++14 -g

hashing.o: hashing.cpp
	g++ -c hashing.cpp --std=c++14 -g

intbtree.o: intbtree.cpp
	g++ -c intbtree.cpp --std=c++14 -g
