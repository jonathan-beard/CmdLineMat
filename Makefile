compile:
	$(CXX) -std=c++11 -O2 -c math.cpp -o math.o
	$(CXX) -o simplemath math.o -lcmdargs


.PHONY:clean
clean:
	rm -rf simplemath math.o
