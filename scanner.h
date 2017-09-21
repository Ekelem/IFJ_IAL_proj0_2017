/*
 *	IFJ/IAL project 2017
 *	author/s: Erik Kelemen
*/

#ifndef SCANNER_H
#define SCANNER_H 42

//LEXICAL ANALYSIS

typedef enum token
{
	identifier = 0,
	As,
	Asc,
	Declare,
	Dim,
	Do,
	Double,
	Else,
	End,
	Chr,
	Function,
	If,
	Input,
	Integer,
	Length,
	Loop,
	Print,
	Return,
	Scope,
	String,
	SubStr,
	Then,
	While,

	And,
	Boole,
	Continue,
	Elseif,
	Exit,
	False,
	For,
	Next,
	Not,
	Or,
	Shared,
	Static,
	True

	//TODO: Doplnit


}T_token;
#endif