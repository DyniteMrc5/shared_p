# shared_p
A shared pointer implementation in c++

git clone https://github.com/DyniteMrc5/shared_p

To use, simply copy the shared_p.hpp to your project and include the file (#include "shared_p.hpp").
To test, run `make`, then run `./shared_p_test`

shared_p allows the user to share an object without having to manually manage object lifetimes (same concept as std::shared_ptr)

Users can provide a custom delete function to ::make_shared allowing custom destruction of T.

Example usages:

	#include "shared_p.hpp"

	void fn(MyType& aType) { .. }
	void fn(shared_p<MyType> aType) { .. }

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
 			 	fn(copy); //  fn could be a function taking a shared_p, or a function taking a int&
 		 	} // (copy deleted here, but sp remains in scope, so data not deleted)
	 
 	} 
 	// (sp deleted, so data also deleted here as sp is last remaining shared_p)

