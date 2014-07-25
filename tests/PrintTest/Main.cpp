// Main.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "arduino.h"
#include "Print.h"

class PrintableTester : public Printable
{
public:
	using Printable::printTo;
	inline size_t PrintableTester::printTo(Print &p)
	{
		p.print("It's a printable");
		return sizeof("It's a printable");
	}
};

class PrintTester : public Print
{
public:
	using Print::write;
	inline size_t PrintTester::write(uint8_t value)
	{
		Log(L"%c", value);
		return 1; // assume success
	}
};

int _tmain(int argc, _TCHAR* argv[])
{
	return RunArduinoSketch();
}

void setup()
{
	PrintTester *p = new PrintTester();
	
	// All println functions call print inside them
	
	// print character
	p->println('c');
	
	// print String
	String st = String("String");
	p->println(st);
	
	// print char *
	char *temp = "char *";
	p->println("char *");

	Log(L"Printing Integers:\n");
	// print integer in DEC
	int n = 10;
	p->println(n);
	
	// print integer in OCT
	p->println(n, OCT);
	
	// print integer in HEX
	p->println(n, HEX);
	
	// print integer in BIN
	p->println(n, BIN);
	
	Log(L"Printing Unsigned Integers:\n");
	// print unsigned integer
	unsigned int un = 10;
	p->println(un);

	// print unsigned integer in OCT
	p->println(un, OCT);
	
	// print unsigned integer in HEX
	p->println(un, HEX);
	
	// print unsigned integer in BIN
	p->println(un, BIN);

	Log(L"Printing Longs:\n");
	// print long
	long l = 10;
	p->println(l);
	
	// print long in OCT
	p->println(l, OCT);
	
	// print long in HEX
	p->println(l, HEX);
	
	// print long in BIN
	p->println(l, BIN);
	
	Log(L"Printing Unsigned Longs:\n");
	// print unsigned long
	unsigned long ul= 10;
	p->println(ul);
	
	// print unsigned long in OCT
	p->println(ul, OCT);
	
	// print unsigned long in HEX
	p->println(ul, HEX);

	// print unsigned long in BIN
	p->println(ul, BIN);

	// print double with default digits
	p->println(3.14159);
	
	// print double with 4 digits
	p->println(3.14159, 4);
	
	// print Printable
	PrintableTester printableTester;
	p->println(printableTester);
}

void loop()
{
}


