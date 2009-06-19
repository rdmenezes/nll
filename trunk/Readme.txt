Numerical learning library FAQ
------------------------------

1) How to use it?
------------------------------
Under Visual Studio 2005 and newer versions, just
load the project file nll.sln. Build the project and
optionally select a target to run (i.e. tutorial or
unit test) and that's it!
  The project can also be build using the main makefile.
First run the configure to configure the compiler and
compilation options (this file can manually modified).
Example: ./configure debug
It will compile the library in a debug mode.
The makefile has several target:
- nll: to build the nll library
- test: to compile and run the tests
- tutorial: to compile and run the turotials
- tarball: will create a new package from
  the SVN repository.
Example: make clan tutorial
It will recompile NLL and the tutorials then it will run
them.