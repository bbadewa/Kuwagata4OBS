//Originally, this was JUST going to contain the dock stuff but there's just not enough
//of the other functionality to justify putting in another .cpp file, so it's mostly
//frontend stuff.

#include "Dock.h"
#include "qmainwindow.h"
#include "qlayout.h"
#include "obs-frontend-api.h"

#include "Backend.h"
#include "qheaderview.h"
#include "callback/signal.h"
#include "obs-hotkey.h"

KuwagataDock* kMenu;
std::vector<struct SourceInfo> verseSources;
std::vector<struct SourceInfo> versionSources;
int verseIdx = 0;
struct Request *currentRequest = nullptr;

void OnSourceNameChange(void* info, calldata_t* data) {

	const char *newName = calldata_string(data, "new_name");
	const char *oldName = calldata_string(data, "prev_name");
	//Anything's a pointer if you're brave enough.
	SourceType s = static_cast<SourceType>((long long)info >> 32);
	int idx = static_cast<int>(0x00000000FFFFFFFF & (long long)info);
	struct SourceInfo *infoStruct;
	if (s == VERSE_SOURCE_TYPE) {
		infoStruct = &verseSources.at(idx);
	} else {
		infoStruct = &versionSources.at(idx);
	}
	
	if (strcmp(newName, oldName)) {
		kMenu->GetSettingsMenu()->ModifySourceName(s, idx, newName);
	}
	infoStruct->name = newName;

}

void TransformQueue(void *data, obs_hotkey_id id, obs_hotkey_t *hotkey, bool pressed) {
	if (!pressed || currentRequest == nullptr) { return;}
	int direction = (long long)data;
	if (verseIdx + direction >= currentRequest->verses.size() || verseIdx + direction < 0) {return;}
	verseIdx += direction;
	kMenu->PropagateToSources(currentRequest->verses[verseIdx], currentRequest->individualRefs[verseIdx]);
}

void OnSourceRemoved(void* info, calldata_t* data) {

	SourceType s = static_cast<SourceType>((long long)info >> 32);
	int idx = static_cast<int>(0x00000000FFFFFFFF & (long long)info);
	kMenu->GetSettingsMenu()->RemoveSourceWithIndex(s, idx);
}

void SetupFrontend() {
	kMenu = new KuwagataDock();
	obs_frontend_add_dock_by_id("kuwagata4obs", "Kuwagata", (QWidget*)kMenu);
	obs_hotkey_register_frontend("nextVerse", 
		"Kuwagata: Next Verse", TransformQueue, (void*)1);
	obs_hotkey_register_frontend("prevVerse", 
		"Kuwagata: Previous Verse", TransformQueue, (void *)-1);
}

void DockCallback(obs_frontend_event e, void *data) {
	if (e == OBS_FRONTEND_EVENT_FINISHED_LOADING) {
		SetupFrontend();
	} else if (e == OBS_FRONTEND_EVENT_EXIT) {
		delete kMenu;	
	}
}

struct SourceInfo getSourceInfo(const char *sourceName, SourceType type, int idx)
{

	struct SourceInfo info;
	info.source = nullptr;
	obs_source_t *newSource = obs_get_source_by_name(sourceName);

	if (!newSource) {
		return info;
	}

	if (!strstr(obs_source_get_id(newSource), "text")) {
		return info;
	}

	info.source = newSource;
	info.name = obs_source_get_name(newSource);
	info.sourceIdx = idx;
	//We do a little trolling (shove both the type and idx into a "pointer")
	long long controlBlock = ((long long)type << 32) | ((long long)idx);
	signal_handler_t * s = obs_source_get_signal_handler(info.source);
	signal_handler_connect(s, "rename", OnSourceNameChange, (void *)controlBlock);
	signal_handler_connect(s, "remove", OnSourceRemoved, (void *)controlBlock);
	return info;
}



void KuwagataDock::AdjustRequestsTable(struct Request r) {
	if (requestsSize < PREV_REQUESTS_SIZE) {
		requestsSize++;
	}

	for (int i = PREV_REQUESTS_SIZE - 2; i >= 0; i--) {
		previousReferences.item(i + 1, 0)->setText(previousReferences.item(i, 0)->text());
		previousRequests[i + 1] = previousRequests[i];
	}
	previousRequests[0] = r;
	currentRequest = previousRequests;
	verseIdx = 0;
	previousReferences.item(0, 0)->setText(QString::fromStdString(r.reference));

}

void KuwagataDock::UpdateCaptiveSources(SourceType type, int idx, QString newSourceName) {
	std::vector<struct SourceInfo>* captiveSources = type == VERSE_SOURCE_TYPE ? &verseSources : &versionSources;

	if (newSourceName == "" && captiveSources->size() > idx) { //Shorthand for "This source no longer exists"
		if (captiveSources->at(idx).source != nullptr) {
			obs_source_release(captiveSources->at(idx).source);
		}
		captiveSources->erase(captiveSources->begin() + idx);
		return;
	}

	SourceInfo info = getSourceInfo(newSourceName.toUtf8(), type, idx);
	bool isTextSource = false;
	if (info.source != nullptr) {
		isTextSource = strstr(obs_source_get_id(info.source), "text");
	}
	settingsMenu.SetSourceValidated(type, idx, info.source != nullptr && isTextSource);
	if (captiveSources->size() <= idx) {
		captiveSources->push_back(info);
	} else {
		captiveSources->at(idx) = info;

	}
}

void KuwagataDock::PropagateToSources(std::string verse, std::string version) {
	for (struct SourceInfo info : verseSources) {
		if (info.source != nullptr) {
			obs_data_t *data = obs_source_get_settings(info.source);
			obs_data_set_string(data, "text", verse.c_str());
			obs_source_update(info.source, data);
			obs_data_release(data);
		}
	}

	for (struct SourceInfo info : versionSources) {
		if (info.source != nullptr) {
			obs_data_t* data = obs_source_get_settings(info.source);
			obs_data_set_string(data, "text", version.c_str());
			obs_source_update(info.source, data);
			obs_data_release(data);
		}
	}
}

SettingsMenu *KuwagataDock::GetSettingsMenu()
{
	return &settingsMenu;
}

KuwagataDock::~KuwagataDock() {
	//Let go of all our sources
	for (SourceInfo s : verseSources) {
		if (s.source != nullptr)
		{
			obs_source_release(s.source);
		}
	}
	for (SourceInfo s : versionSources) {
		if (s.source != nullptr) {
			obs_source_release(s.source);
		}
	}
	//Let go of the backend
	BackendFree();
}

void OnRetrieve() {
	KuwagataDock *dock = (KuwagataDock *)kMenu;
	std::string reference = dock->GetVerse();
	std::string version = dock->GetVersion();
	struct Request r = StartRequest(reference, version);
	dock->SetVerseTable(r.verses, r.individualRefs);
	dock->AdjustRequestsTable(r);
	dock->PropagateToSources(r.verses[0], r.individualRefs[0]);
}

void SetupVersionBox(QComboBox *v)
{
	backendInfo *info = getBackendInfo();
	for (std::string bible : info->availableBibles) {
		v->addItem(QString::fromStdString(bible));
	}
	v->setCurrentIndex(0);
}

KuwagataDock::KuwagataDock() : settingsMenu(getConfigData(), this) {
	setTitleBarWidget(new QWidget());
	setWindowTitle("Kuwagata");

	QWidget *containerWidget = new QWidget();
	QVBoxLayout* main = new QVBoxLayout();
	QHBoxLayout* topHalf = new QHBoxLayout();
	QVBoxLayout* commands = new QVBoxLayout();
	QVBoxLayout* selections = new QVBoxLayout();
	QHBoxLayout* autoBox = new QHBoxLayout();

	autoBox->setAlignment(Qt::AlignHCenter);

	QLabel* autoLabel = new QLabel("Auto: ");
	autoBox->addWidget(autoLabel);
	autoBox->addWidget(&isAutomatic);

	QLabel* verseLabel = new QLabel("Verse");
	QLabel* versionLabel = new QLabel("Version");
	versionLabel->setAlignment(Qt::AlignHCenter);
	verseLabel->setAlignment(Qt::AlignHCenter);



	previousReferences.setColumnCount(1);
	previousReferences.setRowCount(PREV_REQUESTS_SIZE);
	previousReferences.setEditTriggers(QAbstractItemView::NoEditTriggers);
	for (int i = 0; i < PREV_REQUESTS_SIZE; i++) {
		previousReferences.setItem(i, 0, new QTableWidgetItem(""));
	}

	QHeaderView *horizontalHeader = previousReferences.horizontalHeader();
	QHeaderView *verticalHeader = previousReferences.verticalHeader();

	horizontalHeader->hide();
	horizontalHeader->setSectionResizeMode(QHeaderView::Stretch);
	verticalHeader->hide();
	verticalHeader->setSectionResizeMode(QHeaderView::Stretch);

	commands->addWidget(verseLabel);
	commands->addWidget(&verses);
	commands->addWidget(versionLabel);
	commands->addWidget(&versionBox);
	commands->addLayout(autoBox);
	commands->addWidget(&previousReferences);
	SetupVersionBox(&versionBox);

	QLabel *selectionsLabel = new QLabel("Selection");
	verseTable.setColumnCount(2);
	verseTable.setEditTriggers(QAbstractItemView::NoEditTriggers);
	horizontalHeader = verseTable.horizontalHeader();
	verticalHeader = verseTable.verticalHeader();
	horizontalHeader->hide();
	verticalHeader->hide();
	horizontalHeader->setSectionResizeMode(QHeaderView::Stretch);
	verticalHeader->setSectionResizeMode(QHeaderView::Stretch);
	retrieveBtn.setText("Retrieve");
	selections->addWidget(selectionsLabel);
	selections->addWidget(&verseTable);
	selections->addWidget(&retrieveBtn);
	settingsBtn.setText("Settings");

	topHalf->addLayout(commands);
	topHalf->addLayout(selections);
	main->addLayout(topHalf);
	main->addWidget(&settingsBtn);
	
	containerWidget->setLayout(main);
	setWidget(containerWidget);

	requestsSize = 0;
	
	connect(&retrieveBtn, &QPushButton::clicked, this, &OnRetrieve);
	connect(&settingsBtn, &QPushButton::clicked, this, [this]() { 
		if (!settingsMenu.isVisible()) {
			settingsMenu.show();
		}
	});
	connect(&previousReferences, &QTableWidget::cellDoubleClicked, this, [this](int row, int col) {
		if (row >= requestsSize) {return;}
		struct Request targetRequest = previousRequests[row];
		if (targetRequest.verses.size() != 0) {
			SetVerseTable(targetRequest.verses, targetRequest.individualRefs);
			PropagateToSources(targetRequest.verses[0], targetRequest.individualRefs[0]);
			verseIdx = 0;
			currentRequest = &previousRequests[row];
		}
	});
	connect(&verseTable, &QTableWidget::cellDoubleClicked, this, [this](int row, int col) { 
		//This is ugly and I regret having done this as soon as I did it
		PropagateToSources(verseTable.item(row, 1)->text().toStdString(),
				   verseTable.item(row, 0)->text().toStdString()); 
		verseIdx = row;
	});

}

std::string KuwagataDock::GetVerse()
{
	return verses.displayText().toStdString();
}

std::string KuwagataDock::GetVersion()
{
	return versionBox.currentText().toStdString();
}

void KuwagataDock::SetVerseTable(std::vector<std::string> verses, std::vector<std::string> references) {
	verseTable.setRowCount((int)verses.size());
	for (int i = 0; i < verses.size(); i++) {
		QTableWidgetItem *verseItem = new QTableWidgetItem(QString::fromStdString(verses.at(i)));
		QTableWidgetItem *refItem = new QTableWidgetItem(QString::fromStdString(references.at(i)));
		verseTable.setItem(i, 0, refItem);
		verseTable.setItem(i, 1, verseItem);
	}
}
