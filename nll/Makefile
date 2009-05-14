include ../configure.in
LIBS=-lm
LIBNAME=$(LIBPREFIX)nll$(LIBPOSTFIX)
DEPS=buffer1D-converter.h\
buffer1D.h\
classifier-adaboost.h\
classifier-gmm.h\
classifier-mlp.h\
classifier-nearest-neighbor.h\
classifier-svm.h\
classifier.h\
database-input-adapter.h\
database-manipulation.h\
database.h\
debug-decompose.h\
debug-io.h\
ensure.h\
feature-selection-best-first.h\
feature-selection-genetic-algorithm.h\
feature-selection-pearson.h\
feature-selection-relieff.h\
feature-selection.h\
gabor-filter.h\
gabor.h\
generic-operations.h\
genetic-algorithm.h\
gmm.h\
id-maker.h\
image-binary-transformation.h\
image-converter.h\
image-convolution.h\
image-interpolator.h\
image-io.h\
image-mask.h\
image-morphology.h\
image-resample.h\
image-snr.h\
image-threshold.h\
image-transformation.h\
image-utility.h\
image.h\
index-mapper.h\
io.h\
kd-tree.h\
kmeans.h\
labelize.h\
log.h\
logger.h\
lsdbc.h\
mask-exporter.h\
math-correlation.h\
math-distribution-gaussian.h\
math-distribution-uniform.h\
math-sampling.h\
math.h\
matrix-basic-operations.h\
matrix-basic.h\
matrix-converter.h\
matrix-covariance.h\
matrix-decomposition.h\
matrix-sort.h\
matrix-svd.h\
matrix.h\
metric.h\
multi-layered-perceptron.h\
nll.h\
optimizer-client.h\
optimizer-genetic-algorithm.h\
optimizer-grid-search.h\
optimizer-harmony-search-memory.h\
optimizer-harmony-search.h\
optimizer-parameters.h\
optimizer.h\
preprocessing-bridge-image-vector.h\
preprocessing-classifier.h\
preprocessing-image-center.h\
preprocessing-image-gabor.h\
preprocessing-image-place.h\
preprocessing-image-resample.h\
preprocessing-raw-normalize.h\
preprocessing-raw-select.h\
preprocessing-typelist-concatenate.h\
preprocessing-typelist-operation.h\
preprocessing-typelist.h\
preprocessing-unit.h\
region-growing.h\
relief.h\
sequence-converter.h\
static-assert.h\
static-vector.h\
stdafx.h\
stopping-condition.h\
svm.h\
timer.h\
type-traits-inherited.h\
type-traits-memory.h\
type-traits.h\
types.h\
unreachable.h\
utility-pure.h\
utility.h

OBJ=math.o multi-layered-perceptron.o nll.o optimizer-harmony-search-memory.o optimizer-harmony-search.o optimizer-parameters.o svm.o

%.o: %.cpp $(DEPS)
	$(CC) -DNLL_NOT_MULTITHREADED $(FLAGS) -c -o $@ $<

all: $(OBJ)
	$(CC) -DNLL_NOT_MULTITHREADED -shared -o $(LIBNAME) $(OBJ) $(LIBS)
	mv $(LIBNAME) ../$(LIBPATH)

.PHONY: clean
	
clean:
	rm -rf *.o *~ core *~ ../$(LIBPATH)/$(LIBNAME) docs