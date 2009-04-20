include ../configure.in

LIBNAME=$(LIBPREFIX)tester$(LIBPOSTFIX)
DEPS=register.h
LIBS=

OBJ=register.o

%.o: %.cpp $(DEPS)
	$(CC) $(FLAGS) -c -o $@ $<

all: $(OBJ)
	$(CC) $(OBJ) -shared -o $(LIBNAME) $(LIBS)
	mv $(LIBNAME) ../$(LIBPATH)

.PHONY: clean
	
clean:
	rm -rf *.o *~ core *~ ../$(LIBPATH)/$(LIBNAME) docs