include configure.in


all: nll

nll:
	cd nll && make all

test: nll tester
	cd nllTest && make all && cd ../$(LIBPATH)/ && ./test$(EXECPOSTFIX)

tutorial: nll tester
	cd tutorial && make all && cd ../$(LIBPATH) && ./tutorial$(EXECPOSTFIX)

tester:
	cd tester && make all

.PHONY: clean tester doc nll all

clean:
	rm -rf *.o *~ *~ $(LIBNAME) tarball docs html latex
	cd nll && make clean
	cd tester && make clean
	cd nllTest && make clean
	cd tutorial && make clean

distclean: clean
	rm -rf configure.in

doc:
	cd nll && doxygen nll.dox
	mv nll/docs .

tarball: clean doc all
	mkdir tarball
	cd tarball && mkdir nll && cd nll && mkdir nll nllTest tester doc include tutorial
	cd tarball/nll/tutorial && mkdir data && cd data && mkdir usps && mkdir out && mkdir image && mkdir proben1 && mkdir uci && cd proben1 && mkdir cancer
	cd tarball/nll/tutorial/data/uci && mkdir iris spam wine yeast
	cp tutorial/data/uci/iris/*.data tarball/nll/tutorial/data/uci/iris/
	cp tutorial/data/uci/spam/*.data tarball/nll/tutorial/data/uci/spam/
	cp tutorial/data/uci/wine/*.data tarball/nll/tutorial/data/uci/wine/
	cp tutorial/data/uci/yeast/*.data tarball/nll/tutorial/data/uci/yeast/
	cp tutorial/data/usps/*.txt tarball/nll/tutorial/data/usps/
	cd tarball/nll && mkdir $(MODE) && cd $(MODE) && mkdir $(PLATFORM)
	cd tarball/nll/nllTest && mkdir data && cd data && mkdir gmm image spect medical ica feature histogram manifolds ddf bnt
	cp Makefile ChangeLog License.txt AUTHORS configure nll.sln Readme.txt tarball/nll
	cp nll/*.h tarball/nll/include
	cp tutorial/Makefile tutorial/*.h tutorial/*.cpp tutorial/tutorial.vcxproj tarball/nll/tutorial
	cp tutorial/data/proben1/cancer/*.dt tarball/nll/tutorial/data/proben1/cancer
	cp tutorial/data/proben1/cancer/*.data tarball/nll/tutorial/data/proben1/cancer
	cp tutorial/data/image/*.bmp tarball/nll/tutorial/data/image
	cp nll/*.h nll/*.cpp nll/nll.vcxproj nll/nll.vcxproj.filters nll/nll.rc nll/Makefile nll/nll.dox nll/todo.txt tarball/nll/nll
	cp docs/html/* tarball/nll/doc
	cp nllTest/*.h nllTest/*.cpp nllTest/nllTest.vcxproj nllTest/Makefile tarball/nll/nllTest
	cp nllTest/data/gmm/*.mfc tarball/nll/nllTest/data/gmm
	cp nllTest/data/spect/SPECT* tarball/nll/nllTest/data/spect
	cp nllTest/data/image/*.bmp tarball/nll/nllTest/data/image
	cp nllTest/data/medical/pet-NAC.mf2 nllTest/data/medical/test1.mf2 nllTest/data/medical/pet.mf2 nllTest/data/medical/MR-1.mf2 tarball/nll/nllTest/data/medical
	cp nllTest/data/ica/*.bmp tarball/nll/nllTest/data/ica
	cp nllTest/data/feature/*.bmp tarball/nll/nllTest/data/feature
	cp nllTest/data/histogram/*.txt tarball/nll/nllTest/data/histogram
	cp nllTest/data/manifolds/*.txt tarball/nll/nllTest/data/manifolds
	cp nllTest/data/ddf/*.bmp tarball/nll/nllTest/data/ddf
	cp nllTest/data/bnt/*.txt tarball/nll/nllTest/data/bnt
	cp tester/*.h tester/*.cpp tester/tester.vcxproj tester/ReadMe.txt tester/Makefile tarball/nll/tester