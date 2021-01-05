This is a modified version of the L-Py framework by Frédéric Boudon et al.
For original documentation refer to:
http://openalea.gforge.inria.fr/dokuwiki/doku.php?id=packages:vplants:lpy:main
as well as the paper:
F. Boudon, T. Cokelaer, C. Pradal and C. Godin, L-Py, an open L-systems framework in Python, FSPM 2010

L-Py is an L-System simulation framework for Python implemented in C++ and exposed to Python via Boost::Python.

This is a custom version intended to be used for lstring derivation only, leaving out the aspects of turtle interpretation and visualization originally provided by the PlantGL framework. Dependencies on an external PlantGL lib have been removed.
The intention is to be able to provide custom implementation of the interpretation aspects (e.g. via the Blender Python interface) and to be able to use the powerful lstring derivation capabilities of the L-Py framework as a more modular library.


=== DEPENDENCIES ===

the following packages need to be installed:

- Visual Studio 2017 (2015 and 2019 should also work).

- Python 3.7.6 x64
	(downloaded from: https://www.python.org/downloads/ ).
	The x64 version is needed to compile a x64 version of the LPY library which will be compatible with Blender >2.80.
	I originally had Python installed with Anaconda but I could not find all the directories I wanted so ended up installing this version separately.

- Qt5Core 5.14.2 
	You can download the free trial to have all the libs and directories needed : https://www.qt.io/download

- BoostPython 1.72 
	downloaded from the official website: https://www.boost.org/users/download/ 
	You either download the source or the prebuilt Windows binaries.


=== INSTALLATION ===

1.	Create a new project from source code with Visual Studio

2.	Include the “src” folder from the lpy-lsystems-adapted-ade repo to the newly created project

3.	Let Visual Studio Create the solution. This might take some time.

4.	Exclude de files using the solution explorer window by right clicking the files’ names:
	a.	interpretation.cpp & interpretation.h
	b.	plot.cpp & plot.h
	c.	export_interpretation.cpp
	d.	export_plot.cpp
	
5.	Open the project properties (Menu “Project” > Properties):
	a.	At the Top of the window, select:
		i.	“Release” for the “Configuration”
		ii.	“x64” for the “Platform”
	b.	In the General panel:
		i.	Target extension: .dll
		ii.	Configuration Type: Dynamic Library (.dll)
	c.	In VC++ Directories, Edit “Include Directories” to add:
		i.	***\Python\376_x64\include
		ii.	***\boost_1_72_0  (the root folder containing the whole boost dependencies you downloaded)
		iii.***\Qt\5.14.2\msvc2017_64\include
	d.	In Linker > General, edit “Additional Library Directories” to add:
		i.	***\Python\376_x64\tcl
		ii.	***\Python\376_x64\libs
		iii.***\Python\376_x64\Lib
		iv.	***\Python\376_x64\DLLs
		v.	***\boost_1_72_0\stage\lib (the directory containing the the BoostPython libraries. You have to build them if you have not downloaded the binaries)
		vi.	***\Qt\5.14.2\msvc2017_64\bin
		vii.***\Qt\5.14.2\msvc2017_64\lib
	e.	In Linker > Input, Edit “Additional Dependencies” to add:
		i.	Qt5Core.lib

6.	Hit “Build Solution” in the Build menu

7.	Let it do its stuff (pray if you think it helps).
	I had a lot of warnings even for the version that worked fine in the end so just overlook those.

8.	Rename the generated .dll to “lpy.pyd”: this exact name is important!
	The generated .dll should be available in the x64 folder at the root of your project.

9.	Check that “lpy.pyd” loads well with the Python you used for the compilation
	To do that, write “import lpy” in a script saved in a directory containing a copy of “lpy.pyd”

10.	If you passed step 16 then copy “lpy.pyd” in ***\Blender_2.83.10_LTS\2.83\scripts\modules.
	Don’t forget to also include  the lindenmaker add-on compatible with Blender >2.80 in the directory
	***\Blender_2.83.10_LTS\2.83\scripts\addons\lindenmaker (see the pull requests to have the updated code)

11.  Have Fun! :-D
