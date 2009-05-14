include ../configure.in

EXECUTABLE=tutorial$(EXECPOSTFIX)
DEPS=*.h
LIBS=-lm -l$(LIBPREFIXLINK)nll -l$(LIBPREFIXLINK)tester
INCL=-I..

OBJ=database-cancer.o\
main.o\
test-breast-cancer.o\
test-classifier-knn.o\
test-classifier-nn.o\
tutorial-classifier-optimization.o\
tutorial-feature-pca.o\
tutorial-feature-selection.o\
tutorial-partition-database.o\
tutorial-pipeline.o

%.o: %.cpp $(DEPS)
	$(CC) -DNLL_NOT_MULTITHREADED $(FLAGS) $(INCL) -c -o $@ $<

all: $(OBJ)
	$(CC) $(OBJ) -o $(EXECUTABLE) -L../$(LIBPATH) $(LIBS)
	mv $(EXECUTABLE) ../$(LIBPATH)

.PHONY: clean
	
clean:
	rm -rf *.o *~ core *~ ../$(LIBPATH)/$(EXECUTABLE) docs