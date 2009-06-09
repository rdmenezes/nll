include configure.in

all: nll

nll:
	cd nll && make all

test: nll tester
	cd nllTest && make all && ../$(LIBPATH)/test$(EXECPOSTFIX)

tutorial: nll tester
	cd tutorial && make all && cd ../$(LIBPATH) && ./tutorial$(EXECPOSTFIX)
	
tester:
	cd tester && make all
	
.PHONY: clean tester doc nll all

clean:
	rm -rf *.o *~ core *~ $(LIBNAME) tarball docs html latex
	cd nll && make clean
	cd tester && make clean
	cd nllTest && make clean
	cd tutorial && make clean

distclean: clean
	rm -rf configure.in
	
doc:
	doxygen nll/nll.dox
	
tarball: clean doc all
	mkdir tarball
	cd tarball && mkdir nll && cd nll && mkdir nll nllTest tester doc include tutorial
	cd tarball/nll/tutorial && mkdir data && cd data && mkdir out && mkdir image && mkdir proben1 && cd proben1 && mkdir cancer
	cd tarball/nll && mkdir $(MODE) && cd $(MODE) && mkdir $(PLATFORM)
	cd tarball/nll/nllTest && mkdir data && cd data && mkdir gmm image spect
	cp Makefile ChangeLog License.txt AUTHORS configure nll.sln Readme.txt tarball/nll
	cp nll/*.h tarball/nll/include
	cp tutorial/Makefile tutorial/*.h tutorial/*.cpp tutorial/tutorial.vcproj tarball/nll/tutorial
	cp tutorial/data/proben1/cancer/*.dt tarball/nll/tutorial/data/proben1/cancer
	cp tutorial/data/image/*.bmp tarball/nll/tutorial/data/image
	cp nll/*.h nll/*.cpp nll/nll.vcproj nll/nll.rc nll/Makefile nll/nll.dox nll/todo.txt tarball/nll/nll
	cp docs/html/* tarball/nll/doc
	cp nllTest/*.h nllTest/*.cpp nllTest/nllTest.vcproj nllTest/Makefile tarball/nll/nllTest
	cp nllTest/data/gmm/*.mfc tarball/nll/nllTest/data/gmm
	cp nllTest/data/spect/SPECT* tarball/nll/nllTest/data/spect
	cp nllTest/data/image/*.bmp tarball/nll/nllTest/data/image
	cp tester/*.h tester/*.cpp tester/tester.vcproj tester/ReadMe.txt tester/Makefile tarball/nll/tester