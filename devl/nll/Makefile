include ../configure.in
LIBS=-lm
LIBNAME=$(LIBPREFIX)nll$(LIBPOSTFIX)
DEPS=bracketing.h\
brent.h\
buffer1D-converter.h\
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
feature-transformation-kernel-pca.h\
feature-transformation-normalization.h\
feature-transformation-pca.h\
feature-transformation.h\
gabor-filter.h\
gabor.h\
generic-operations.h\
genetic-algorithm.h\
gmm.h\
hmm-continuous.h\
hmm.h\
id-maker.h\
image-binary-transformation.h\
image-converter.h\
image-convolution.h\
image-distance-transform.h\
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
kernel-functions.h\
kernel-pca.h\
kmeans.h\
labelize.h\
log.h\
logger.h\
lsdbc.h\
lut.h\
markov-chain.h\
mask-exporter.h\
math-correlation.h\
math-distribution-gaussian.h\
math-distribution-multinormal.h\
math-distribution-uniform.h\
math-floor.h\
math-quaternion.h\
math-sampling.h\
math.h\
matrix-basic-operations.h\
matrix-basic.h\
matrix-cholesky.h\
matrix-converter.h\
matrix-covariance.h\
matrix-decomposition.h\
matrix-sort.h\
matrix-svd.h\
matrix.h\
metric.h\
mlp.h\
multiplanar-reconstruction.h\
nll.h\
normalization.h\
optimizer-client.h\
optimizer-genetic-algorithm.h\
optimizer-grid-search.h\
optimizer-harmony-search-memory.h\
optimizer-harmony-search.h\
optimizer-parameters.h\
optimizer-powell.h\
optimizer.h\
pca.h\
powell.h\
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
singleton.h\
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
utility.h\
volume-interpolator.h\
volume-io-mf2.h\
volume-spatial.h\
volume.h

OBJ=configuration.o math-floor.o math.o nll.o optimizer-harmony-search-memory.o optimizer-harmony-search.o optimizer-parameters.o svm-linear.o svm.o

.cpp.o: $(DEPPS)
	$(CC) $(FLAGS) -c -o $@ $<

all: $(OBJ)
	cd .. && mkdir -p $(MODE) && mkdir -p $(MODE)/$(PLATFORM)
	$(CC) -shared -o $(LIBNAME) $(OBJ) $(LIBS)
	mv $(LIBNAME) ../$(LIBPATH)

.PHONY: clean


clean:
	rm -rf *.o *~ core *~ ../$(LIBPATH)/$(LIBNAME) docs