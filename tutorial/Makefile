include ../configure.in

EXECUTABLE=tutorial$(EXECPOSTFIX)
DEPS=*.h
LIBS=-lm -l$(LIBPREFIXLINK)nll -l$(LIBPREFIXLINK)tester
INCL=-I..

OBJ=database-cancer.o\
database-iris.o\
database-spam.o\
database-usps.o\
database-wine.o\
database-yeast.o\
main.o\
test-breast-cancer.o\
test-classifier-knn.o\
test-classifier-nn.o\
test-iris-database.o\
test-kpca.o\
test-optimizer.o\
test-spam-database.o\
test-usps-database.o\
test-wine-database.o\
test-yeast-database.o\
tutorial-classifier-optimization.o\
tutorial-distance-transform.o\
tutorial-feature-pca.o\
tutorial-feature-selection.o\
tutorial-partition-database.o

%.o: %.cpp $(DEPS)
	$(CC) $(FLAGS) $(INCL) -c -o $@ $<

all: $(OBJ)
	$(CC) $(OBJ) -o $(EXECUTABLE) -L../$(LIBPATH) $(LIBS)
	mv $(EXECUTABLE) ../$(LIBPATH)

.PHONY: clean
	
clean:
	rm -rf *.o *~ core *~ ../$(LIBPATH)/$(EXECUTABLE) docs