//© 2016 Michael Cox
#include "shared_p.hpp"
#include "gtest/gtest.h"
#include <string>

#ifdef _MSC_VER
	// If editing in Visual Studio, define these
	// macros to allow compilation and manual inspection.
	#define TEST(testclass, Title) void Title()
	#define ASSERT_EQ(X,Y)
	#define ASSERT_NE(X,Y)
#endif

// Simple object that owns an int*
struct Data
{
	// takes ownership of aData
	Data(int* aData) : iData(aData)	{ }
	Data(Data&& other) = default;
	Data(const Data& other) = default;

	~Data()
	{
		// set to something other than *iData
		*iData = 0xdeadbeef;
		//std::cout << iData << " - *iData = " << std::hex << *iData << std::endl;
		delete iData;
		iData = nullptr;
	}

	int* iData;
};

// Tests compilation
TEST(Basics, CanCreateShPofIntPtr)
{
	shared_p<int> s = shared_p<int>::make_shared(new int(5));
}

// Tests that d is set to null after shared_p is created
// Tests that int* is deleted (value is set to something other than 5) (accessing deleted* is technically undefined behaviour)
TEST(Basics, CanCreateShPOfData)
{
	int* five = new int(5);
	Data* d = new Data(five);
	{
		shared_p<Data> s = shared_p<Data>::make_shared(std::move(d));
		ASSERT_EQ(*five, 5);
		ASSERT_EQ(d, nullptr);
	}
	ASSERT_NE(*five, 5);
}

// Tests multiple shared_p
TEST(Basics, CanCreate2ShP)
{
	int* five = new int(5);
	Data* d = new Data(five);
	{
		shared_p<Data> s = shared_p<Data>::make_shared(std::move(d));
		ASSERT_EQ(d, nullptr);
		ASSERT_EQ(*five, 5);

		shared_p<Data> t = s;
		ASSERT_EQ(*five, 5);
	}
	ASSERT_NE(*five, 5);
}

// Test multiple shared_p with different object lifetimes
TEST(Basics, CanCreate2ShP_DiffSharedLifetimes)
{
	int* five = new int(5);
	Data* d = new Data(five);
	{
		shared_p<Data> s = shared_p<Data>::make_shared(std::move(d));
		ASSERT_EQ(d, nullptr);

		{
			shared_p<Data> t = s;
		} // t destroyed here
		
		// five should still be 5
		ASSERT_EQ(*five, 5);
	} // s deleted here
	
	ASSERT_NE(*five, 5);
}

// Test operator T& works
TEST(Basics, TestOperatorT)
{
	int* five = new int(5);
	Data* d = new Data(five);
	{
		shared_p<Data> s = shared_p<Data>::make_shared(std::move(d));
		ASSERT_EQ(d, nullptr);

		ASSERT_EQ(*five, 5);

		*((Data&)s).iData = 6;

		ASSERT_EQ(*five, 6);
	}
	ASSERT_NE(*five, 5);
}

// Test T& get() works
TEST(Basics, TestOperatorGet)
{
	int* five = new int(5);
	Data* d = new Data(five);
	{
		shared_p<Data> s = shared_p<Data>::make_shared(std::move(d));
		ASSERT_EQ(d, nullptr);

		ASSERT_EQ(*five, 5);

		*(s.get().iData) = 6;

		ASSERT_EQ(*five, 6);
	}
	ASSERT_NE(*five, 5);
}

void deleteFn(Data* aData)
{
	*aData->iData = 7;
}

//Test with custom deleter
TEST(Basics, TestDeleter)
{
	int* five = new int(5);
	Data* d = new Data(five);
	Data* copy = d;
	{
		shared_p<Data> s = shared_p<Data>::make_shared(std::move(d), deleteFn);
		ASSERT_EQ(d, nullptr);

		ASSERT_EQ(*five, 5);
	}
	ASSERT_EQ(*five, 7);
	delete copy;
}

const int abcdef = 0xabcdefff;
void DataFn(Data& aData)
{
	*aData.iData = abcdef;
}

//Test conversion to Data& automatically
TEST(Basics, TestCanCallFunctionWithoutSharedPSigniture)
{
	int* five = new int(5);
	Data* d = new Data(five);
	Data* copy = d;
	{
		shared_p<Data> s = shared_p<Data>::make_shared(std::move(d));
		ASSERT_EQ(*five, 5);
		DataFn(s);
		ASSERT_EQ(*five, abcdef);
	}
}

#ifdef _MSC_VER
int main(int argc, char** argv)
{
	CanCreateShPofIntPtr();
	CanCreateShPOfData();
	CanCreate2ShP();
	CanCreate2ShP_DiffSharedLifetimes();
	TestOperatorT();
	TestOperatorGet();
	TestDeleter();
	TestCanCallFunctionWithoutSharedPSigniture();
	return 0;
}
#endif
