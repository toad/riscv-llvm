#ifndef SUBCLASS_TEST_HH
#define SUBCLASS_TEST_HH

#include "Test.hh"

class SubclassTest : public Test {
	private:
		long x; // Force 8 byte alignment.

	public:
		SubclassTest();
		void print();
};

#endif
