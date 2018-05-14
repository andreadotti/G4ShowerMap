Example of Geant4 Shower analysis.

Copyright (c) Andrea Dotti (SLAC National Accelerator Laboratory) 2014
adotti@slac.stanford.edu

===== Introduction =========
Example to Build in memory a map of the particle shower with
primary-secondary relation.
The example shows a simple analysis that can be 
performed and navigation of the shower map.
To each particle in the shower associates a 
quantity (a double).

====== How to use =========
As a unit test: 
	make clean
	make
	./test
Read the content of test.cc for an explanation 
and example of all possible operations.
The G4ShowerMap.hh contains the main interfaces 
in the G4ShowerMap::Analysis class.

To integrate into a G4 application, copy
.cc and .hh into the application package and 
add the files to the compilation source.




