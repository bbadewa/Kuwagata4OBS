#pragma once
#include<obs-frontend-api.h>
#include "obs-source.h"
#include"qdockwidget.h"
#include "qlabel.h"
#include "qlineedit.h"
#include "qtablewidget.h"
#include "qcheckbox.h"
#include "qpushbutton.h"
#include "qcombobox.h"
#include "SettingsMenu.h"
#include "Backend.h"
#define PREV_REQUESTS_SIZE 10

void SetupFrontend();
void DockCallback(obs_frontend_event e, void *data);

struct SourceInfo {
	obs_source_t *source = nullptr;
	std::string name;
	int sourceIdx = 0;
};

class KuwagataDock : public QDockWidget {
public: 
	KuwagataDock();
	std::string GetVerse();
	std::string GetVersion();
	void SetVerseTable(std::vector<std::string> verses, std::vector<std::string> references);
	void AdjustRequestsTable(struct Request r);
	void UpdateCaptiveSources(SourceType type, int idx, QString newSourceName);
	void PropagateToSources(std::string verse, std::string version);
	SettingsMenu* GetSettingsMenu();
	~KuwagataDock();

private:
	QLineEdit	verses;
	QComboBox versionBox;
	QPushButton settingsBtn;
	QPushButton retrieveBtn;
	QTableWidget verseTable;
	QTableWidget previousReferences;
	QCheckBox isAutomatic;
	SettingsMenu settingsMenu;
	
	struct Request previousRequests[PREV_REQUESTS_SIZE]; 
	int requestsSize;
};