CXXFLAGS += -std=c++11
#CPPFLAGS += -DDEBUG

pdp8:	PDP8.o main.o
	g++ -o $@ $^
	
	
.PHONY:	run
run: pdp8
	./pdp8
	
.PHONY:	clean
clean:	
	$(RM) pdp8 *.o