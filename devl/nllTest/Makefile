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
test-discriminant.o\
test-feature-selection.o\
test-fit-histogram.o\
test-gaussian-bayesian-inference.o\
test-gaussian-tfm.o\
test-genetic-algorithm.o\
test-geometry.o\
test-gram-schmidt-orthgonalization.o\
test-haar-features.o\
test-hmm.o\
test-image-spatial.o\
test-integral-image-3d.o\
test-kdTree.o\
test-kernel-pca.o\
test-kurtosis.o\
test-labelize.o\
test-linear-regression.o\
test-locally-linear-embedding.o\
test-lut.o\
test-markov-chain.o\
test-math-floor.o\
test-matrix-cholesky.o\
test-maximum-intensity-projection.o\
test-mlp.o\
test-morphology.o\
test-mpr.o\
test-naive-bayes.o\
test-optimizer.o\
test-optimizer-parameters.o\
test-pca.o\
test-planar-2d-registration.o\
test-potential-table.o\
test-principal-independent-component.o\
test-pseudo-inverse.o\
test-quaternion.o\
test-ransac.o\
test-rbf-network.o\
test-region-growing.o\
test-registration-3d.o\
test-regression.o\
test-relieff.o\
test-sammon-projection.o\
test-slice.o\
test-surf.o\
test-surf-3d.o\
test-transformation.o\
test-transformation-ddf-3d.o\
test-transformation-normalization.o\
test-tree.o\
test-visual-quasi-periodicity.o\
test-volume.o\
test-volume-compressor.o\
test-volume-distance-transform.o\
test-volume-interpolator-3d.o\
test-volume-io.o\
test-volume-mapper.o\
test-volume-mapper-ddf.o\
test-volume-resampling.o


.cpp.o: $(DEPS)
	$(CC) $(FLAGS) $(INCL) -c -o $@ $<

all: $(OBJ)
	$(CC) $(OBJ) -o $(EXECUTABLE) -L../$(LIBPATH) $(LIBS)
	mv $(EXECUTABLE) ../$(LIBPATH)

.PHONY: clean
	
clean:
	rm -rf *.o *~ core *~ ../$(LIBPATH)/$(EXECUTABLE) docs