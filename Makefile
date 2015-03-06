CXXFLAGS += -std=c++11

pdp8:	PDP8.o main.o
	g++ -o $@ $^
	
	
.PHONY:	run
run: pdp8
	./pdp8 > log
	
.PHONY:	clean
clean:	
	$(RM) pdp8 *.o