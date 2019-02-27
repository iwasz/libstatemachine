Disclaimer
==========
This library is a part of my toolbox, which means that its API may change without 
any further notice. 

This library was created for dealing with various modems which use AT commands,
and it was deployed on 32 bit microcontrolers in production as well as in my
personal projects. Using this state machine looks like this:

	machine.state (INIT, INITIAL).transition ..... TODO

How to compile
==============
Collect all the dependencies:

* [Catch2](https://github.com/catchorg/Catch2)
* [etlcpp](https://www.etlcpp.com)  (is it really required?)
* libmicro (???)

Please pull those to a directory where libstatemachine directory resides.