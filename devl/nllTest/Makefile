include ../configure.in

EXECUTABLE=test$(EXECPOSTFIX)
DEPS=*.h
LIBS=-lm -l$(LIBPREFIXLINK)nll -l$(LIBPREFIXLINK)tester
INCL=-I..

OBJ=nllTest.o\
test.o\
test-affine-tfm-estimator.o\
test-allocator.o\
test-boosting.o\
test-bracketing.o\
test-classifier-adaboost.o\
test-classifier-diagnostics.o\
test-classifier-discriminant.o\
test-classifier-gmm.o\
test-classifier-mlp.o\
test-classifier-naive-bayes.o\
test-classifier-nearest-neighbor.o\
test-classifier-spect.o\
test-classifier-svm.o\
test-converter.o\
test-deformable-transformation.o\
test-discriminant.o

.cpp.o: $(DEPS)
	$(CC) $(FLAGS) $(INCL) -c -o $@ $<

all: $(OBJ)
	$(CC) $(OBJ) -o $(EXECUTABLE) -L../$(LIBPATH) $(LIBS)
	mv $(EXECUTABLE) ../$(LIBPATH)

.PHONY: clean
	
clean:
	rm -rf *.o *~ core *~ ../$(LIBPATH)/$(EXECUTABLE) docs