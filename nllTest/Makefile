include ../configure.in

EXECUTABLE=test$(EXECPOSTFIX)
DEPS=*.h
LIBS=-lm -l$(LIBPREFIXLINK)nll -l$(LIBPREFIXLINK)tester
INCL=-I..

OBJ=nllTest.o\
test-classifier-adaboost.o\
test-classifier-gmm.o\
test-classifier-mlp.o\
test-classifier-nearest-neighbor.o\
test-classifier-spect.o\
test-classifier-svm.o\
test-feature-selection.o\
test-genetic-algorithm.o\
test-labelize.o\
test-morphology.o\
test-optimizer-parameters.o\
test-optimizer.o\
test-pca.o\
test-preprocessing.o\
test-region-growing.o\
test.o

%.o: %.cpp $(DEPS)
	$(CC) $(FLAGS) $(INCL) -c -o $@ $<

all: $(OBJ)
	echo "---linking----"
	echo "LIBPATH=" $(LIBPATH)
	$(CC) $(OBJ) -o $(EXECUTABLE) -L../$(LIBPATH) $(LIBS)
	mv $(EXECUTABLE) ../$(LIBPATH)

.PHONY: clean
	
clean:
	rm -rf *.o *~ core *~ ../$(LIBPATH)/$(EXECUTABLE) docs