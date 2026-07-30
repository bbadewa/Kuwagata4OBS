#include"Backend.h"
#include"Kuwagata.h"
#include"UserException.h"
#include"obs-module.h"
#include <algorithm> 
#include<util/platform.h>


struct backendInfo info;
obs_data_t *configData;
using namespace KuwagataDLL;


void InitializeData() {
	const char *bibleToPreload;
	if (info.availableBibles.size() > 0) {
		bibleToPreload = info.availableBibles[0].c_str();
	} else {
		bibleToPreload = "";
	}
	obs_data_array_t *otherBibles = obs_data_array_create();
	obs_data_array_t *verseSources = obs_data_array_create();
	obs_data_array_t *versionSources = obs_data_array_create();
	obs_data_set_string(configData, "bibleToPreload", bibleToPreload);
	obs_data_set_bool(configData, "preloadMultipleBibles", false);
	obs_data_set_array(configData, "otherBiblesToPreload", otherBibles); 
	obs_data_set_bool(configData, "deliverCompoundReferences", false); //TODO something with this
	obs_data_set_int(configData, "compoundReferenceCharacterLimit", 256);
	obs_data_set_array(configData, "verseSources", verseSources);
	obs_data_set_array(configData, "versionSources", versionSources);

	obs_data_array_release(otherBibles);
	obs_data_array_release(verseSources);
	obs_data_array_release(versionSources);
}

/*
Initializes the backend info structure and wakes up KuwagataDLL.
*/
bool BackendInit() {
	
	bool shouldInitData = false;
	//Create config dir
	char *kuwaConfig = obs_module_config_path("Kuwagata4OBS.json");
	char *configDir = (char*)malloc(512);
	memset(configDir, 0, 512);
	strncpy(configDir, kuwaConfig, strlen(kuwaConfig) - strlen("Kuwagata4OBS.json"));
	if (!os_file_exists(configDir)) {
		os_mkdir(configDir);
	}
	info.configDir = configDir;

	//Write config info
	if (!os_file_exists(kuwaConfig)) {
		os_quick_write_utf8_file(kuwaConfig, "{}", 2, false);
		shouldInitData = true;
	}
	configData = obs_data_create_from_json_file(kuwaConfig);
	
	//Scan for bibles
	char biblePath[512];
	snprintf(biblePath, sizeof(biblePath), "%sbibles", configDir);
	if (!os_file_exists(biblePath)) {
		os_mkdir(biblePath);
	}
	os_dir_t *bibleDir = os_opendir(biblePath);
	struct os_dirent *bibles = os_readdir(bibleDir);
	while (bibles != NULL) {
		if (bibles->directory && strlen(bibles->d_name) >= 3) {
			info.availableBibles.push_back(std::string(bibles->d_name));
		}
		bibles = os_readdir(bibleDir);
	}
	
	//Load the first available bible by default
	//TODO: we gotta derive this from the config file instead of just loading the first one
	if (info.availableBibles.size() != 0) {
		std::string defaultBiblePath(configDir);
		defaultBiblePath += "/bibles/" + info.availableBibles.at(0);
		Kuwagata::Initialize(defaultBiblePath);
	}

	os_closedir(bibleDir);

	//Lastly, free the config path
	bfree(kuwaConfig);

	if (shouldInitData) {
		InitializeData();
	}

	return true;
}

void BackendFree() {
	Kuwagata::Release();
	obs_data_save_json(configData, obs_module_config_path("Kuwagata4OBS.json"));
	obs_data_release(configData);
	free(info.configDir);
}

void toUpper(std::string& in) {
	for (int i = 0; i < in.length(); i++) {
		in[i] = std::toupper(in[i]);
	}
}

/*
* @return The Configuration data for this plugin.
* Please don't forget to call BackendInit() first before invoking!
*/
obs_data_t* getConfigData() {
	return configData;
}	

bool isAvailableBible(std::string version) {
	for (std::string bible : info.availableBibles) {
		if (version == bible) {
			return true;
		}
	}
	return false;
}

/*
Query KuwagataDLL for a new request.
@param reference: The reference to pass along to the DLL.
@param version: The version from which to extract the reference
@return A pointer to a Request structure containing the string content
of the original reference, the corresponding verses, their prettified references,
and any exceptions generated while generating this request.
*/
struct Request StartRequest(std::string reference, std::string version)
{
	if (version != "" && version != Kuwagata::GetCurrentVersion() && isAvailableBible(version)) {
		std::string biblePath(info.configDir);
		biblePath += ("bibles/" + version);
		Kuwagata::ChangeOSISPath(biblePath);
	}

	Kuwagata::StartNewRequest(reference);
	struct Request request;

	toUpper(version);
	
	request.reference = reference + ", " + version;
	//Y'know, I'm starting to think that nuking previous
	//references to get rid of the new ones was a.. blunder, so to speak.
	request.verses = Kuwagata::GetVerses();
	request.individualRefs = Kuwagata::GetReferences();
	request.exceptions =  std::vector<std::string>();

	for (UserException exception : Kuwagata::GetRaisedExceptions()) {
		request.exceptions.push_back(exception.asString());
	}

	return request;

}


void DisposeRequest(struct Request *request) {
	if (request == nullptr) {
		return;
	}
	free(request);
}

struct backendInfo *getBackendInfo() {
	return &info;
}
