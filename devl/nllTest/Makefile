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
test-hmm.o\
test-kdTree.o\
test-labelize.o\
test-markov-chain.o\
test-matrix-cholesky.o\
test-mlp.o\
test-morphology.o\
test-optimizer-parameters.o\
test-optimizer.o\
test-pca.o\
test-preprocessing.o\
test-quaternion.o\
test-region-growing.o\
test-relieff.o\
test.o

%.o: %.cpp $(DEPS)
	$(CC) $(FLAGS) $(INCL) -c -o $@ $<

all: $(OBJ)
	$(CC) $(OBJ) -o $(EXECUTABLE) -L../$(LIBPATH) $(LIBS)
	mv $(EXECUTABLE) ../$(LIBPATH)

.PHONY: clean
	
clean:
	rm -rf *.o *~ core *~ ../$(LIBPATH)/$(EXECUTABLE) docs