include ../configure.in

EXECUTABLE=test$(EXECPOSTFIX)
DEPS=*.h
LIBS=-lm -l$(LIBPREFIXLINK)nll -l$(LIBPREFIXLINK)tester
INCL=-I..

OBJ=nllTest.o\
test-allocator.o\
test-bracketing.o\
test-classifier-adaboost.o\
test-classifier-gmm.o\
test-classifier-mlp.o\
test-classifier-nearest-neighbor.o\
test-classifier-spect.o\
test-classifier-svm.o\
test-feature-selection.o\
test-genetic-algorithm.o\
test-geometry.o\
test-haar-features.o\
test-hmm.o\
test-kdTree.o\
test-kernel-pca.o\
test-lut.o\
test-markov-chain.o\
test-math-floor.o\
test-matrix-cholesky.o\
test-maximum-intensity-projection.o\
test-mlp.o\
test-morphology.o\
test-optimizer-parameters.o\
test-optimizer.o\
test-pca.o\
test-quaternion.o\
test-region-growing.o\
test-regression.o\
test-relieff.o\
test-transformation-normalization.o\
test-transformation.o\
test-volume-io.o\
test-volume-resampling.o\
test-volume.o

.cpp.o: $(DEPS)
	$(CC) $(FLAGS) $(INCL) -c -o $@ $<

all: $(OBJ)
	$(CC) $(OBJ) -o $(EXECUTABLE) -L../$(LIBPATH) $(LIBS)
	mv $(EXECUTABLE) ../$(LIBPATH)

.PHONY: clean
	
clean:
	rm -rf *.o *~ core *~ ../$(LIBPATH)/$(EXECUTABLE) docs