include ../configure.in

LIBNAME=$(LIBPREFIX)tester$(LIBPOSTFIX)
DEPS=register.h
LIBS=-lm -l$(LIBPREFIXLINK)nll
INCL=-I..

OBJ=register.o

%.o: %.cpp $(DEPS)
	$(CC) $(FLAGS) -DNO_MULTITHREADED_UNIT_TESTS $(INCL) -c -o $@ $<

all: $(OBJ)
	$(CC) $(OBJ) -shared -o $(LIBNAME) -L../$(LIBPATH) $(LIBS)
	mv $(LIBNAME) ../$(LIBPATH)

.PHONY: clean
	
clean:
	rm -rf *.o *~ core *~ ../$(LIBPATH)/$(LIBNAME) docs