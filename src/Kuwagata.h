#ifndef KUWAGATA_DLL_MAIN
#define KUWAGATA_DLL_MAIN


#ifndef KUWAGATA_DLL_EXPORTS
#define KUWAGATA_DLL __declspec(dllexport)
#else 
#define KUWAGATA_DLL __declspec(dllimport)
#endif

#include<string>
#include<vector>
#include"UserException.h"

namespace KuwagataDLL {
	class Kuwagata {
	public:
		KUWAGATA_DLL static void Initialize(std::string OSISpath);
		KUWAGATA_DLL static void ChangeOSISPath(std::string newOSISPath);
		KUWAGATA_DLL static void StartNewRequest(std::string Verse);
		KUWAGATA_DLL static std::string GetCurrentVersion();
		KUWAGATA_DLL static std::vector<UserException> GetRaisedExceptions();
		KUWAGATA_DLL static std::vector<std::string> GetVerses();
		KUWAGATA_DLL static std::vector<std::string> GetReferences();
		KUWAGATA_DLL static std::vector<int> GetVerseIDs();
		KUWAGATA_DLL static void Release();
	};
}

#endif;