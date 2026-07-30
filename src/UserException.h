#ifndef USER_EXCEPTION
#define USER_EXCEPTION
#include<string>
/*
	A major design flaw of the first iteration of Kuwagata was that if you messed up any part of a reference,
	you were basically stuck with an unhelpful error message that left you no clue what part you messed up,
	and messing up any part of a reference halted interpretation immediately, leaving you with 0 verses to display
	even if you only messed up the verse on the end. This is cumbersome, especially for long and large references that
	require scrolling back and forth in order to understand where you messed up.
	Hopefully, implementing a class like this will help mitigate both of these issues. 
*/

#pragma warning(disable: 4251) //offendingUserInput will ideally, Never Ever be used by a client program itself. Safe to ignore.

#ifndef KUWAGATA_DLL_EXPORTS
#define KUWAGATA_DLL __declspec(dllexport)
#else 
#define KUWAGATA_DLL __declspec(dllimport)
#endif

namespace KuwagataDLL {

	
	enum ExceptionType {
		UNKNOWN_BOOK, //You referenced a book that we don't have.
		CHAPTER_OUT_OF_RANGE, //Your chapter is out of range for the book we have.
		VERSE_OUT_OF_RANGE, //Your verse is out of range for the chapter we have.
	};

	/*
	The UserException class, designed to allow users to understand errors returned by KuwagataDLL more comprehensively
	and to increase program fault-tolerance. 
	*/
	class KUWAGATA_DLL UserException {
		public:
			
			UserException(ExceptionType eType, int reference);
			UserException(ExceptionType eType, std::string userInput);
			std::string asString();
			ExceptionType getExceptionType();
		private:
			ExceptionType type;
			int exceptionReference;
			std::string  offendingUserInput;

	};
}

#endif