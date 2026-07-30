#include "SettingsMenu.h"
#include "qboxlayout.h"
#include "qlabel.h"
#include "qlist.h"
#include "qlineedit.h"
#include "qheaderview.h"
#include "Dock.h"
#include "qwindow.h"


/*
I am THIS CLOSE to copying obs-multi-rtmp's method 
of wrapping configuration files in nlohmann's JSON library
instead of dealing with... whatever this is 
*/
void SettingsMenu::LoadSources()
{
	obs_data_array_t* verseSources = obs_data_get_array(data, "verseSources");
	obs_data_array_t *versionSources = obs_data_get_array(data, "versionSources");
	int verseSize = static_cast<int>(obs_data_array_count(verseSources)); //Shut up, MSVC. I know what I'm doing.
	int versionSize = static_cast<int>(obs_data_array_count(versionSources)); //(And, frankly, if you have more than even a million sources in OBS, that's on you.)
	verseOutputTable.setRowCount(verseSize);
	versionOutputTable.setRowCount(versionSize);
	for (int i = 0; i < verseSize; i++) { //If there's a method of storing primitives in arrays provided by OBS,
		obs_data_t *source = obs_data_array_item(verseSources, i); //I'm gonna feel so stupid.
		QTableWidgetItem *item = new QTableWidgetItem();
		item->setText(obs_data_get_string(source, "Name"));
		verseOutputTable.setItem(i, 0, item);
		obs_data_release(source);
	}

	for (int i = 0; i < versionSize; i++) {
		obs_data_t *source = obs_data_array_item(versionSources, i);
		QTableWidgetItem *item = new QTableWidgetItem();
		item->setText(obs_data_get_string(source, "Name"));
		versionOutputTable.setItem(i, 0, item);
		obs_data_release(source);
	}
	obs_data_array_release(verseSources);
	obs_data_array_release(versionSources);
	settingsLoaded = true;

}

void SettingsMenu::SaveSourceName(SourceType type, int idx, const char *newName) {
	if (!settingsLoaded) {return;}
	obs_data_array_t *arr = obs_data_get_array(data, type == VERSE_SOURCE_TYPE ? "verseSources" : "versionSources");
	int arrSize = static_cast<int>(obs_data_array_count(arr));
	obs_data_t *newData;
	if (arrSize - 1 < idx) { //If we're adding onto the end
		newData = obs_data_create();
		obs_data_array_push_back(arr, newData);
	} else { //If we're modifying in place
		newData = obs_data_array_item(arr, idx);
	}
	obs_data_set_string(newData, "Name", newName);
	obs_data_array_release(arr);
	obs_data_release(newData);
}

QGridLayout *SettingsMenu::setupSettings()
{
	QGridLayout* layout = new QGridLayout();
	
	QLabel *descLabel;
	QLabel *startupHeader = new QLabel("Startup Behaviour");
	QFont font = startupHeader->font();
	font.setBold(true);
	startupHeader->setFont(font);
	layout->addWidget(startupHeader, 0,0);

	descLabel = new QLabel("Default Bible: ");
	layout->addWidget(descLabel, 1, 0);
	layout->addWidget(&defaultBible, 1, 1);
	for (std::string bible : getBackendInfo()->availableBibles) {
		toUpper(bible);
		defaultBible.addItem(QString::fromStdString(bible));
	}
	connect(&defaultBible, &QComboBox::currentIndexChanged, this, [this](int idx) { 
		obs_data_set_string(data, "bibleToPreload", defaultBible.itemText(idx).toUtf8());
	});


	//Revisit later!
	//descLabel = new QLabel("Preload Other Bibles:");
	//descLabel->
	//	setToolTip("Enables preloading multiple bible versions at startup.\n<font color=\"red\">Will increase memory footprint and OBS startup times!</font>");
	//preloadBibles.setChecked(obs_data_get_bool(data, "preloadMultipleBibles"));


	QLabel *generalHeader = new QLabel("General");
	generalHeader->setFont(font);
	layout->addWidget(generalHeader, 2, 0);

	descLabel = new QLabel("Deliver multiple verses");
	descLabel->setToolTip("Enables multiple verses to be displayed at one time.");
	deliverMultipleReferences.setChecked(obs_data_get_bool(data, "deliverCompoundReferences"));
	connect(&deliverMultipleReferences, &QCheckBox::checkStateChanged, this,
		[this]() { 
			multiRefSize.setEnabled(deliverMultipleReferences.isChecked());
			obs_data_set_bool(data, "deliverCompoundReferences", true); //fix the structure later!!
		});
	layout->addWidget(descLabel, 3, 0);
	layout->addWidget(&deliverMultipleReferences, 3, 1);

	descLabel = new QLabel("Multiple-verse character limit");
	multiRefSize.setEnabled(deliverMultipleReferences.isChecked());
	layout->addWidget(descLabel, 4, 0);
	layout->addWidget(&multiRefSize, 4, 1);

	return layout;
}



SettingsMenu::SettingsMenu(obs_data_t *settings, QWidget *parent)
{
	dock = parent;
	data = settings;
	obs_data_addref(data);
	setWindowTitle("Kuwagata Settings");
	QHBoxLayout* hbox = new QHBoxLayout();
	QVBoxLayout *outputSources = new QVBoxLayout();

	QLabel *settingsLabel = new QLabel("Settings");
	QLabel *outputSourcesLabel = new QLabel("Output Sources");
	QLabel *verseLabel = new QLabel("Verse Outputs");
	QLabel *versionLabel = new QLabel("Reference/Version Outputs");
	outputSources->addWidget(verseLabel);

	QHeaderView *hv, *vv;

	hv = verseOutputTable.horizontalHeader();
	vv = verseOutputTable.verticalHeader();
	hv->hide();
	hv->setSectionResizeMode(QHeaderView::Stretch);
	vv->hide();

	hv = versionOutputTable.horizontalHeader();
	vv = versionOutputTable.verticalHeader();
	hv->hide();
	hv->setSectionResizeMode(QHeaderView::Stretch);
	vv->hide();

	QHBoxLayout *addRemoveBtnVerses = new QHBoxLayout();
	QHBoxLayout *addRemoveBtnVersions = new QHBoxLayout();

	addSourceVerses.setText("Add Verse Source");
	removeSourceVerses.setText("Remove Verse Source");

	QPushButton *addSourceVersions = new QPushButton("Add Version Source");
	QPushButton *removeSourceVersions = new QPushButton("Remove Version Source");

	addRemoveBtnVerses->addWidget(&addSourceVerses);
	addRemoveBtnVerses->addWidget(&removeSourceVerses);

	addRemoveBtnVersions->addWidget(addSourceVersions);
	addRemoveBtnVersions->addWidget(removeSourceVersions);

	verseOutputTable.setColumnCount(1);
	versionOutputTable.setColumnCount(1);

	outputSources->addWidget(verseLabel);
	outputSources->addWidget(&verseOutputTable);
	outputSources->addLayout(addRemoveBtnVerses);

	outputSources->addWidget(versionLabel);
	outputSources->addWidget(&versionOutputTable);
	outputSources->addLayout(addRemoveBtnVersions);

	hbox->addLayout(outputSources);
	hbox->addLayout(setupSettings());
	setLayout(hbox);

	

	connect(&removeSourceVerses, &QPushButton::clicked, this, &SettingsMenu::RemoveSources);
	connect(removeSourceVersions, &QPushButton::clicked, this, &SettingsMenu::RemoveSources);
			
	connect(&addSourceVerses, &QPushButton::clicked, this, &SettingsMenu::AddSource);
	connect(addSourceVersions, &QPushButton::clicked, this, &SettingsMenu::AddSource);
			
	connect(&verseOutputTable, &QTableWidget::cellChanged, this, &SettingsMenu::OnSourceTableChange);
	connect(&versionOutputTable, &QTableWidget::cellChanged, this, &SettingsMenu::OnSourceTableChange);

	LoadSources();
}

/*
* Signal that your source has been validated/invalidated
* @param type: the type of source
* @param sourceIndex: the idx of the source in its container
* @param validationStatus: the validation status of the source
* 
*/
void SettingsMenu::SetSourceValidated(SourceType type, int sourceIndex, bool validationStatus) {
	
	QBrush invalidationBrush(Qt::GlobalColor::red);
	QBrush validationBrush;
	QTableWidget *table = type == VERSE_SOURCE_TYPE ? &verseOutputTable : &versionOutputTable;
	if (table->rowCount() - 1 > sourceIndex) {
		return;
	}
	table->blockSignals(true);
	table->item(sourceIndex, 0)->setBackground(validationStatus ? validationBrush : invalidationBrush);
	table->blockSignals(false);

	if (validationStatus) {
		SaveSourceName(type, sourceIndex, table->item(sourceIndex, 0)->text().toUtf8());
	}
}

/*
* Modify the name of a source in the settings menu
* @param type: the source type
* @param idx: the index of the source in its type container
* @param newName: the new name of the source
*/
void SettingsMenu::ModifySourceName(SourceType type, int idx, const char* newName) {
	QTableWidgetItem *item = (type == VERSE_SOURCE_TYPE ? verseOutputTable : versionOutputTable).item(idx, 0);
	item->setText(QString(newName));

	SaveSourceName(type, idx, newName);
}

/*
	Remove a source with a type and index
	@param type: the source type
	@param idx: the index of the source in its container
*/
void SettingsMenu::RemoveSourceWithIndex(SourceType type, int idx) {
	QTableWidget *table = (type == VERSE_SOURCE_TYPE ? &verseOutputTable : &versionOutputTable);
	table->removeRow(idx);
	((KuwagataDock *)dock)->UpdateCaptiveSources(type, idx, "");
}

SettingsMenu::~SettingsMenu() {
	obs_data_release(data);
}


void SettingsMenu::OnSourceTableChange(int row, int col)
{
	QTableWidget *table = (QTableWidget*)sender();
	SourceType tableType = table == &verseOutputTable ? VERSE_SOURCE_TYPE : VERSION_SOURCE_TYPE;
	QString text = table->item(row, col)->text();
	if (text == "") {
		table->removeRow(row);
	}
	((KuwagataDock *)dock)->UpdateCaptiveSources(tableType, row, text);
}


void SettingsMenu::RemoveSources() {
	QPushButton *emitter = (QPushButton*)sender();
	QList<QTableWidgetItem *> selectedItems;

	selectedItems = (emitter == &removeSourceVerses ? verseOutputTable : versionOutputTable).selectedItems();
	if (selectedItems.size() == 0) {
		return;
	}
	
	int row;
	SourceType type = (emitter == &removeSourceVerses) ? VERSE_SOURCE_TYPE : VERSION_SOURCE_TYPE;
	for (QTableWidgetItem *item : selectedItems) {
		row = item->row();
		verseOutputTable.removeRow(row);
		((KuwagataDock *)dock)->UpdateCaptiveSources(type, row, "");
	}
}

void SettingsMenu::AddSource() {
	QPushButton *emitter = (QPushButton *)sender();
	QTableWidget* currentAddingTable = (emitter == &addSourceVerses) ? &verseOutputTable : &versionOutputTable;
	QTableWidgetItem *newSource = new QTableWidgetItem();
	int rowCount = currentAddingTable->rowCount();
	currentAddingTable->blockSignals(true);
	currentAddingTable->setRowCount(rowCount + 1);
	currentAddingTable->setItem(rowCount, 0, newSource);
	currentAddingTable->editItem(newSource);
	currentAddingTable->blockSignals(false);
}



