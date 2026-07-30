#pragma once

#include "qobject.h"
#include"qwidget.h"
#include"qtablewidget.h"
#include "qpushbutton.h"
#include "qcombobox.h"
#include "QSpinBox.h"
#include "qgridlayout.h"
#include "qcheckbox.h"
#include "obs.h"

#include<vector>
#include<string>

enum SourceType { VERSE_SOURCE_TYPE, VERSION_SOURCE_TYPE };


class SettingsMenu : public QWidget {

public:
	
	SettingsMenu(obs_data_t *settings, QWidget *parent);
	void SetSourceValidated(SourceType type, int sourceIndex, bool validationStatus);
	void ModifySourceName(SourceType type, int idx, const char* newName);
	void RemoveSourceWithIndex(SourceType type, int idx);
	~SettingsMenu();

private:
	QTableWidget verseOutputTable;
	QTableWidget versionOutputTable;
	QPushButton addSourceVerses;
	QPushButton removeSourceVerses;
	
	QComboBox defaultBible;
	QCheckBox preloadBibles;
	QCheckBox deliverMultipleReferences;
	QSpinBox multiRefSize;
	void LoadSources();
	void SaveSourceName(SourceType type, int idx, const char *newName);
	QGridLayout *setupSettings();
	void OnSourceTableChange(int row, int col);
	void RemoveSources();
	void AddSource();
	obs_data_t *data;
	bool settingsLoaded = false;
	QWidget *dock;
};
