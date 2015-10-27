compile:
	$(CXX) -std=c++11 -O3 -c math.cpp -o math.o
	$(CXX) -O3 -o simplemath math.o -lcmdargs


.PHONY:clean
clean:
	rm -rf simplemath math.o
