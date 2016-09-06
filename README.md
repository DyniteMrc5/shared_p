# shared_p
A shared pointer implementation in c++

git clone https://github.com/DyniteMrc5/shared_p

To use, simply copy the shared_p.hpp to your project and include the file (`#include "shared_p.hpp"`).

Construction is done via the static `shared_p<T>::make_shared(T*&&);`
	
This means that in order to call this function, the T argument should be an r-value reference.
This can be achieved with `std::move(myPtr)`.
After the call, `myPtr` will be `nullptr`. This is to emphasise that the shared_p takes ownership.


To test, run `make`, then run `./shared_p_test`.
The test makes use of https://github.com/google/googletest. (It has been added as a git submodule.)

shared_p allows the user to share an object without having to manually manage object lifetimes (same concept as std::shared_ptr).
Users provide shared_p a pointer and the shared_p then becomes the owner of that memory's lifetime. shared_p will delete the pointer when the last copy of the shared_p is destroyed. 

Users can provide a custom delete function to ::make_shared allowing custom destruction of T.

shared_p provides an `operator T&`. This means that for any function which requires the original type as a reference argument the shared_p can be passed directly.

Example usages:

	#include "shared_p.hpp"

	void fn(MyType& aType) { .. }

	void somefunc()
 	{
 	//----------------------------------------------------------------
 			shared_p<int> sp = shared_p<int>::make_shared(new int(5));
 	//---------------------------------------------------------------- or
 			MyType* data = new MyType(5);
 			shared_p<MyType> sp = shared_p<MyType>::make_shared(std::move(data));
 	//		(data now null - 'ownership transferred')
 	//	-----------------------------------------------------------------
 		 	{
 			 	shared_p<MyType> copy = sp;
 			 	fn(copy); //  fn could be a function taking a shared_p, or a function taking a MyType&

 		 	} // (copy deleted here, but sp remains in scope, so data not deleted)
	 
 	} 
 	// (sp deleted, so data also deleted here as sp is last remaining shared_p)


When running shared_p_test the results should look like the following:

	Running main() from gtest_main.cc
	[==========] Running 8 tests from 1 test case.
	[----------] Global test environment set-up.
	[----------] 8 tests from Basics
	[ RUN      ] Basics.CanCreateShPofIntPtr
	[       OK ] Basics.CanCreateShPofIntPtr (0 ms)
	[ RUN      ] Basics.CanCreateShPOfData
	[       OK ] Basics.CanCreateShPOfData (0 ms)
	[ RUN      ] Basics.CanCreate2ShP
	[       OK ] Basics.CanCreate2ShP (0 ms)
	[ RUN      ] Basics.CanCreate2ShP_DiffSharedLifetimes
	[       OK ] Basics.CanCreate2ShP_DiffSharedLifetimes (0 ms)
	[ RUN      ] Basics.TestOperatorT
	[       OK ] Basics.TestOperatorT (0 ms)
	[ RUN      ] Basics.TestOperatorGet
	[       OK ] Basics.TestOperatorGet (0 ms)
	[ RUN      ] Basics.TestDeleter
	[       OK ] Basics.TestDeleter (0 ms)
	[ RUN      ] Basics.TestCanCallFunctionWithoutSharedPSigniture
	[       OK ] Basics.TestCanCallFunctionWithoutSharedPSigniture (0 ms)
	[----------] 8 tests from Basics (2 ms total)
	
	[----------] Global test environment tear-down
	[==========] 8 tests from 1 test case ran. (2 ms total)
	[  PASSED  ] 8 tests.
