# Flags for the NLL target #
The compilation flags can be set to configure the project builds:
  * DEBUG\_BUFFER1D: if set all the buffer IO will be printed on stdout
  * NLL\_NOT\_MULTITHREADED: if set, this dll should only be used on a single thread, removing the dependency on openMP
  * NLL\_NO\_ENSURE: if set, the ensure mecanism will be disabled

# Flags for the tester target #
  * NLL\_TESTER\_NO\_REGRESSION disable the regression test. It will still write the timing reports, but not compare them.

These flags can be set in the Microsoft project as well as in the configure.in file.