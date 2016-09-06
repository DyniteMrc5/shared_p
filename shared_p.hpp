//© 2016 Michael Cox
#pragma once
#include <atomic>
#include <iostream>

/*
 * shared_p - A shared pointer implementation
 *
 * shared_p allows the user to share an object without having to 
 * manually manage object lifetimes.
 * Users can provide a custom delete function to ::make_shared allowing custom destruction of T.
 *
 * Example usages:
 *
 * {
 * ----------------------------------------------------------------
 * 		shared_p<int> sp = shared_p<int>::make_shared(new int(5));
 * ---------------------------------------------------------------- or
 * 		int* data = new int(5);
 * 		shared_p<int> sp = shared_p<int>::make_shared(std::move(data));
 * 		(data now null - 'ownership transferred')
 * 	-----------------------------------------------------------------
 * 		 {
 * 			 shared_p<int> copy = sp;
 * 			 fn(copy);     fn could be a function taking a shared_p, or a function taking a int&
 * 		 } (copy deleted here, but sp remains in scope, so data not deleted)
 * 
 * } 
 * (sp deleted, so data also deleted here as sp is last remaining shared_p)
 *
 */
template <typename T>
class shared_p 
{
public:

	// -------------------------------------------- permitted constructors:

	// Copy and Move Contsructors:
	shared_p(const shared_p& other);
	shared_p(shared_p&& other);

	/* Public constructor (static)
	
	    Usage:	shared_p<T>::make_shared(new T())

	    Usage : T* managedT = new T(); shared_p<T>::make_shared(std::move(managedT));
		        (NB: managedT subsequently nullptr after call.)
	*/
	static shared_p<T> make_shared(T*&& aOther, void (*aDeleteFn)(T*));
	static shared_p<T> make_shared(T*&& aOther);
	
	// --------------------------------------------- disallowed/deleted constructors:

	/* Public constructor (R-value reference) 
		- not allowed (due to trying to enforce the fact that this object is in control of the memory, rather than the stack).
		 - use T*&& version*/
	static shared_p<T>&& make_shared(T&& aOther) = delete;

	/* Public constructor (Reference) 
		- now allowed (due to wanting to enforce move semantics) */
	static shared_p<T>&& make_shared(T& aOther) = delete;
	
	/* Public constructor (Reference-of-pointer type)
		- not allowed (due to wanting to enforce move semantics) 
		- use T*&& version */
	static shared_p<T> make_shared(T*& aOther) = delete;
	
	/* Public constructor (const R-Value Reference-of-pointer type)
		- not allowed (due to const, we want to set other to null)
		- use T*&& version */
	static shared_p<T>& make_shared(const T*&& aOther) = delete;
	
	// Assignment operator= (not supported)
	shared_p& operator=(const shared_p& aOther) = delete;

	// ---------------------------------------------

	//Destructor
	virtual ~shared_p();

	// How many shared_p's reference this control block?
	int count();

	/*	overload to T&. (T remains under shared_p management)

		Allows passing of shared_p, as if it was a T&, 
		so can call functions with signitures like : void fn(T&) as fn(shared_p);
	*/
	operator T&();

	// returns a reference to the shared data object (remains under shared_p management)
	T& get();

private:

	/*
	shared_ctrl_block - a single instance of which will be shared between all copies of shared_p for a particular T.
	*/
	template <typename C> //typename C, as we have to use a different typename from outer class, but typeof C == typeof T in use.
	struct shared_ctrl_block
	{
		shared_ctrl_block(C* aData, void(*aDeleteFn)(C*));
		~shared_ctrl_block();
		
		// The managed object
		C* iObject;
		
		// Atomic int, this is important as it is what makes the whole thing thread safe
		std::atomic_int iCount;
		
		// Custom delete fn
		void(*iDeleteFn)(C*);
	};

	/* Real constructor - private to prevent construction. Users should use make_shared.
	 * aData is the object to be managed
	 * aDeleteFn is a custom delete function, to be called to delete the object. Is cast to global operator delete when no argument is supplied.
	 */
	shared_p(T* aData, void(*aDeleteFn)(T*) = reinterpret_cast<void(*)(T*)>(operator delete));

	// Control Block (pointer shared between all copies of shared_p for an individual T)
	shared_ctrl_block<T>* iControlBlock;
};


template<typename T>
inline shared_p<T>::~shared_p()
{
	// atomically decrement count - when count is 0, delete the control block also
	/*
	 * This is thread safe as if there is no way to write code such that it would be possible for one thread
	 * to be deleteing the last shared_p (i.e. when previous==1), as well as at the same time there being a copy being created elsewhere.
	 * If we are here and previous == 1, 'this' has gone out of scope and there's no way to create a copy of this.
	 * 
	 * The following is invalid regardless of shared_pointers as the lifetime of s is not protected:
	 * 		shared_p<int> s = .. ;
	 * 		pthread_create(fn(), &s)
	 */
	int previous = iControlBlock->iCount.fetch_sub(1);
	if (previous == 1)
	{
		delete iControlBlock;
		iControlBlock = nullptr;
	}
}

template<typename T>
inline shared_p<T>::shared_p(const shared_p & aOther)
{
	//std::cout << "Calling shared_p copy constructor" << std::endl;
	if (&aOther == this)
	{
		return;
	}

	// Add one first, as a memory leak is worse than a double delete
	aOther.iControlBlock->iCount.fetch_add(1);
	iControlBlock = aOther.iControlBlock;
}

template<typename T>
inline shared_p<T>::shared_p(shared_p && aOther)
{
	//std::cout << "Calling shared_p move constructor" << std::endl;

	if (&aOther == this)
	{
		return;
	}

	iControlBlock = aOther.iControlBlock;
	aOther.iControlBlock = nullptr;
}

template<typename T>
inline shared_p<T> shared_p<T>::make_shared(T *&& aOther, void(*aDeleteFn)(T*))
{
	//std::cout << "Calling make_shared T*&&" << std::endl;
	T* data = aOther;
	aOther = nullptr;
	shared_p<T>* share = new shared_p(data, aDeleteFn);
	return std::move(*(share));
}

template<typename T>
inline shared_p<T> shared_p<T>::make_shared(T *&& aOther)
{
	return shared_p<T>::make_shared(std::move(aOther), nullptr);
}

template<typename T>
inline shared_p<T>::shared_p(T* aData, void(*aDeleteFn)(T*))
{
	//std::cout << "Calling shared_p constructor" << std::endl;
	iControlBlock = new shared_ctrl_block<T>(aData, aDeleteFn);
}

template<typename T>
inline int shared_p<T>::count()
{
	return static_cast<int>(iControlBlock->iCount);
}

template<typename T>
inline shared_p<T>::operator T&()
{
	return *iControlBlock->iObject;
}

template<typename T>
inline T & shared_p<T>::get()
{
	return operator T&();
}

template<typename T>
template<typename C>
inline shared_p<T>::shared_ctrl_block<C>::shared_ctrl_block(C * aData, void(*aDeleteFn)(C*))
	: iObject(aData), iCount(1), iDeleteFn(aDeleteFn)
{
}

template<typename T>
template<typename C>
inline shared_p<T>::shared_ctrl_block<C>::~shared_ctrl_block()
{
	//std::cout << "Deleting control block:" << this << std::endl;
	if (iDeleteFn)
	{
		iDeleteFn(iObject);
	}
	else {
		delete iObject;
	}

	iObject = nullptr;
}
