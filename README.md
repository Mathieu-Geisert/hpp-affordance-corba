#  Humanoid Path Planner - AFFORDANCE-CORBA module

Copyright 2016 LAAS-CNRS

Author: Anna Seppala

##Description
hpp-affordance-corba implements python bindings for hpp-affordance, and presents a few example files.
Please refer to this [link](https://github.com/anna-seppala/hpp-affordance) for information on hpp-affordance.

##Installation on ubuntu-14.04 64 bit with ros-indigo

To install hpp-affordance-corba:

  1. install HPP
	- see https://github.com/humanoid-path-planner/hpp-doc

  2. install HPP-AFFORDANCE
	- see https://github.com/anna-seppala/hpp-affordance

  3. Clone the HPP-AFFORDANCE-CORBA repository onto your local computer and update the submodule:

			git clone https://github.com/anna-seppala/hpp-affordance-corba.git
			cd $HPP_AFFORDANCE_CORBA_DIR/
			git submodule update --init --recursive

  4. Use CMake to install the HPP-AFFORDANCE-CORBA library. For instance:

			mkdir build
			cd build
			cmake ..
			make install

  5. Optionally, install the HPP-RBPRM and HPP-RBPRM-CORBA packages that implement an efficient acyclic contact planner,
		 and its python bindings, respectively. (Make sure you are on branch "affordance" in both repositories!)
	- see https://github.com/anna-seppala/hpp-rbprm, and
		https://github.com/anna-seppala/hpp-rbprm-corba

##Documentation

  Open $DEVEL_DIR/install/share/doc/hpp-affordance-corba/doxygen-html/index.html in a web browser and you
  will have access to the code documentation. If you are using ipython, the documentation of the methods implemented
  is also directly available in a python console.

##Example

To see how to use the CORBA server and the affordance functionality, please refer to the python scripts provided within the 'tests' directory of this package. These python scripts use the HyQ model found in the 'data' directory (retrieved from https://github.com/iit-DLSLab/hyq-description).

To run the test files, launch the hpp-affordance-server executable, then open a python terminal, and copy one of the test scripts (e.g. test-affordance-description.py) into the python terminal bit by bit. This allows you to see the procedure in the viewer as you go through the comments in the example script.
