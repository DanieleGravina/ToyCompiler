/* 
* File:   main.cpp
* Author: daniele
*
* Created on 28 aprile 2014, 11.11
*/

#include <string>
#include <iostream>
#include <cstdio>
#include <fstream>
#include "lexer.h"
#include "frontend.h"

#define ARGS 1

int main(int argc, char **argv) {

	using namespace std;

	/*if (argc != ARGS + 1) {
	fprintf(stderr, "Must use %d arguments", ARGS);
	fputs("\n", stderr);
	exit(1);
	}

	ifstream in(argv[1]);

	if(!in) {
	fprintf(stderr, "Could not open file %s\n", argv[1]);
	exit(1);
	}*/

	const char* s_punt;

	string text;
	stringstream sstr;
	string path = __FILE__; //gets source code path, include file name
	path = path.substr(0,1+path.find_last_of('\\')); //removes file name
	path+= "example.txt"; //adds input file to path

	ifstream myfile (path, ios::in);
	if (myfile.is_open())
	{
		stringstream sstr;
		sstr << myfile.rdbuf();
		text = sstr.str();
		myfile.close();
	}

	s_punt = text.c_str();

	/*const char* s_punt = 
		" VAR x , squ;" 

		" PROCEDURE square z ; "
		" ARRAY y [10]; "
		" BEGIN "
		"  squ := z * z ;"
		"  y[2] := 5 "
		" END; "

		" BEGIN "
		" x := 1; "
		" WHILE x <= 10 DO "
		"  BEGIN "
		"  CALL square x ; "
		"  !squ; "
		"  x := x + 1 "
		"  END "
		" END. ";*/


	lexer l(s_punt);

	frontend f(l);

	f.program();
}

