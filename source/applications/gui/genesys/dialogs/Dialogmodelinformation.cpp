#include "Dialogmodelinformation.h"
#include "ui_Dialogmodelinformation.h"

#include <QMessageBox>

DialogModelInformation::DialogModelInformation(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::DialogModelInformation)
{
	ui->setupUi(this);
}

DialogModelInformation::~DialogModelInformation()
{
	delete ui;
}

void DialogModelInformation::setModelInfo(ModelInfo* modelInfo)
{
	_modelInfo = modelInfo;
	_loadModelInfo();
}

void DialogModelInformation::accept()
{
	if (_modelInfo != nullptr) {
		const std::string projectTitle = _projectTitleFromUi();
		const std::string name = _nameFromUi();
		const std::string version = _versionFromUi();
		const std::string description = _descriptionFromUi();
		const std::string analystName = _analystNameFromUi();

		// Apply only real changes so opening the dialog and pressing OK does not mark the model as dirty.
		if (projectTitle != _originalProjectTitle) {
			_modelInfo->setProjectTitle(projectTitle);
		}
		if (name != _originalName) {
			_modelInfo->setName(name);
		}
		if (version != _originalVersion) {
			_modelInfo->setVersion(version);
		}
		if (description != _originalDescription) {
			_modelInfo->setDescription(description);
		}
		if (analystName != _originalAnalystName) {
			_modelInfo->setAnalystName(analystName);
		}
	}

	QDialog::accept();
}

void DialogModelInformation::reject()
{
	if (_hasPendingChanges()) {
		const QMessageBox::StandardButton answer = QMessageBox::question(
			this,
			tr("Discard changes?"),
			tr("The model information has unsaved changes. Do you want to discard them?"),
			QMessageBox::Discard | QMessageBox::Cancel,
			QMessageBox::Cancel
		);
		if (answer != QMessageBox::Discard) {
			return;
		}
	}

	QDialog::reject();
}

void DialogModelInformation::_loadModelInfo()
{
	if (_modelInfo == nullptr) {
		return;
	}

	_originalProjectTitle = _modelInfo->getProjectTitle();
	_originalName = _modelInfo->getName();
	_originalVersion = _modelInfo->getVersion();
	_originalDescription = _modelInfo->getDescription();
	_originalAnalystName = _modelInfo->getAnalystName();

	// Keep the widgets as a direct view of the editable ModelInfo attributes.
	ui->plainTextProjectName->setPlainText(QString::fromStdString(_originalProjectTitle));
	ui->plainTextModelName->setPlainText(QString::fromStdString(_originalName));
	ui->plainTextModelVersion->setPlainText(QString::fromStdString(_originalVersion));
	ui->plainTextModelDescription->setPlainText(QString::fromStdString(_originalDescription));
	ui->plainTextAnalystName->setPlainText(QString::fromStdString(_originalAnalystName));
}

bool DialogModelInformation::_hasPendingChanges() const
{
	return _modelInfo != nullptr
		&& (_projectTitleFromUi() != _originalProjectTitle
			|| _nameFromUi() != _originalName
			|| _versionFromUi() != _originalVersion
			|| _descriptionFromUi() != _originalDescription
			|| _analystNameFromUi() != _originalAnalystName);
}

std::string DialogModelInformation::_projectTitleFromUi() const
{
	return ui->plainTextProjectName->toPlainText().toStdString();
}

std::string DialogModelInformation::_nameFromUi() const
{
	return ui->plainTextModelName->toPlainText().toStdString();
}

std::string DialogModelInformation::_versionFromUi() const
{
	return ui->plainTextModelVersion->toPlainText().toStdString();
}

std::string DialogModelInformation::_descriptionFromUi() const
{
	return ui->plainTextModelDescription->toPlainText().toStdString();
}

std::string DialogModelInformation::_analystNameFromUi() const
{
	return ui->plainTextAnalystName->toPlainText().toStdString();
}
