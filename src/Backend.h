//Interposer between KuwagataDLL-C++ and the plugin
#pragma once
#include <string>
#include <vector>
#include <obs.h>


struct backendInfo {
	char* configDir;
	std::vector<std::string> availableBibles;
};

struct Request {
	std::string reference;
	std::vector<std::string> verses;
	std::vector<std::string> individualRefs;
	std::vector<std::string> exceptions;
};
void DisposeRequest(struct Request *request);
void toUpper(std::string &in);
bool BackendInit();
void BackendFree();
obs_data_t *getConfigData();
struct Request StartRequest(std::string reference, std::string version);
struct backendInfo *getBackendInfo();



