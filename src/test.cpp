#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <functional>

#include "dm.h"
#include "remoting.h"

using namespace std;
using namespace UCS;

class UCSTestStruct: public UCSStruct {
	
	public:
	
		// initialize here, using copy constructor to avoid duplication
		
		UCSInt field = UCSInt (fields -> get<UCSInt> ("supa"));
		//UCSInt field1 = UCSInt (fields, "trupa");
		//UCSString strField = UCSString (fields, "my_liebe_string");
		//UCSArray<UCSInt> arrayField = UCSArray<UCSInt> (fields, "funky_array");
	
	public:
		
		// no inheriting constructors in gcc 4.7, sadly!

		UCSTestStruct (): UCSStruct () { }

		UCSTestStruct (shared_ptr<UCSValue> value):
			UCSStruct (value) { }

};

void half (int x, int y) {
	cout << "x: " << x << " y: " << y << endl;
	// return y + x/2;
}

int main (int argc, char *argv[]) {
	
	// std::function<void(int,int)> fn1 = half;
	// fn1 (4, 100);

	shared_ptr<UCSNamespace> testNS (new UCSNamespace);

	UCSRemotingClient client (testNS);

	UCSString testStr (testNS -> get<UCSString> ("bound"));
	UCSString testStr2 (testNS -> get<UCSString> ("bound"));

	testStr = "bzzz";
	cout << "testStr2:" << (string) testStr2 <<  endl;

	UCSArray<UCSInt> intArray (testNS -> get<UCSArray<UCSInt>> ("arrr"));

	intArray[4] = 5;
	cout << "intArray[4]: " << intArray[4] << endl;

	UCSTestStruct testStruct (testNS -> get<UCSTestStruct> ("testStruct"));

	testStruct.field = 8;
	cout << "testStruct.field: " << testStruct.field << endl;

	UCSTestStruct testStruct1 (testNS -> get<UCSTestStruct> ("testStruct1"));
	cout << "testStruct1.field: " << testStruct1.field << endl;

	UCSArray<UCSTestStruct> structArray (testNS -> get<UCSArray<UCSTestStruct>> ("structArrr"));

	for (int i = 0; i < 3; i++)
		structArray[i].field = i + 1;

	UCSArray<UCSTestStruct> structArray1 (testNS -> get<UCSArray<UCSTestStruct>> ("structArrr"));

	for (int i = 0; i < 3; i++) {
		cout << "structarray1[" << i << "].field = " << structArray1[i].field << endl;
	}

#if 0
	UCSInt testVar;
	UCSString testStr (testNS, "bound");
	UCSString testStr2 (testNS, "bound");
	
	UCSInt testIntBound (testNS, "hey");
	UCSInt testInt2Bound (testNS, "hey");
	
	cout << "hi 2" << endl;
	
	testVar = 5;
	testIntBound = 6;
	testStr = "bzzz";

	cout << "testVar:" << testVar <<  endl;
	cout << "testStr:" << (string) testStr <<  endl;
	cout << "testStr2:" << (string) testStr2 <<  endl;
	cout << "testIntBound:" << testIntBound <<  endl;
	cout << "testInt2Bound:" << testInt2Bound <<  endl;

	cout << "assign" << endl;
	testInt2Bound = 7;
	cout << "testIntBound:" << testIntBound <<  endl;
	cout << "testInt2Bound:" << testInt2Bound <<  endl;
	
	cout << "assign2" << endl;
	testInt2Bound = testVar;
	cout << "testIntBound:" << testIntBound <<  endl;
	cout << "testInt2Bound:" << testInt2Bound <<  endl;

	cout << endl;
	cout << "listener test" << endl;
	
	cout << "assign3" << endl;
	//UCSInt::ListenerHandle handle = testInt2Bound.addListener ([](int value) { cout << "testInt2BoundChanged: " << value << endl; });
	
	testIntBound = 4;
	cout << "testIntBound:" << testIntBound <<  endl;
	cout << "testInt2Bound:" << testInt2Bound <<  endl;
	
	// testInt2Bound.removeListener (handle);

	testIntBound = 5;
	cout << "testIntBound:" << testIntBound <<  endl;
	cout << "testInt2Bound:" << testInt2Bound <<  endl;

	cout << endl;
	
	cout << "---- creating struct ---- " << endl;

	UCSTestStruct myStruct (testNS, "derStruct");

	cout << "---- done creating struct ---- " << endl;
	
	myStruct.field = 18;
	myStruct.field1 = 19;
	myStruct.strField = "str!";
	myStruct.arrayField[3] = 14;
	
	cout << "myStruct.field:" << myStruct.field << endl;
	cout << "myStruct.field1:" << myStruct.field1 << endl;
	cout << "myStruct.strField:" << (string) myStruct.strField << endl;
	
	for (int i = 0; i < 4; i++)
		cout << "myStruct.arrayField[" << i << "]:" << (myStruct.arrayField[i]) << endl;
	
	UCSTestStruct myStruct1 (testNS, "derStruct");
	
	cout << "myStruct1.field:" << myStruct1.field << endl;
	cout << "myStruct1.field1:" << myStruct1.field1 << endl;
	cout << "myStruct1.strField:" << (string) myStruct1.strField << endl;

	for (int i = 0; i < 4; i++)
		cout << "myStruct1.arrayField[" << i << "]:" << (myStruct1.arrayField[i]) << endl;
	
	UCSArray<UCSInt> intArray (testNS, "testArr");
	UCSArray<UCSInt> intArray1 (testNS, "testArr");

	/* for (int i = 0; i < 3; i++) {
		stringstream ss;
		ss << "arr item[" << i << "] changed: ";
		intArray1[i].addListener (shared_ptr<UCSValueListener<int>> (new UCSPrintingListener (ss.str())));
	} */
	
	for (int i = 0; i < 5; i++)
		intArray[i] = i + 1;
	
	for (int i = 0; i < 5; i++)
		cout << "array[" << i << "]: " << (intArray[i]) << endl;

	for (int i = 0; i < 5; i++)
		cout << "array1[" << i << "]: " << (intArray1[i]) << endl;
	
	
	cout << "----------- array of structs, make it funky! ----------------" << endl;
	
	UCSArray<UCSTestStruct> structArray (testNS, "structArr");
	
	for (int i = 0; i < 4; i++) {
		structArray[i].field = i + 10;
		structArray[i].arrayField[2] = i + 1;
	}
	
	for (int i = 0; i < structArray.size (); i++) {
		cout << "structArray[" << i << "].field = " << structArray[i].field << endl;
		cout << "structArray[" << i << "].arrayfield = ";
		for (int j = 0; j < structArray[i].arrayField.size(); j++)
			cout << structArray[i].arrayField[j] << " ";
		cout << endl;
	}

	UCSArray<UCSTestStruct> structArray2 (testNS, "structArr");

	for (int i = 0; i < structArray2.size (); i++) {
		cout << "structArray2[" << i << "].field = " << structArray2[i].field << endl;
		cout << "structArray[" << i << "].arrayfield = ";
		for (int j = 0; j < structArray2[i].arrayField.size(); j++)
			cout << structArray2[i].arrayField[j] << " ";
		cout << endl;
	}

#endif
	return 0;

	
}


