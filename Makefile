compile:
	$(CXX) -std=c++11 -Ofast -c math.cpp -o math.o
	$(CXX) -O3 -o simplemath math.o -lcmdargs


.PHONY:clean
clean:
	rm -rf simplemath math.o
