#ifndef DIALOGMODELINFORMATION_H
#define DIALOGMODELINFORMATION_H

#include "../../../../../kernel/simulator/ModelInfo.h"

#include <QDialog>
#include <string>

namespace Ui {
	class DialogModelInformation;
}

class DialogModelInformation : public QDialog
{
	Q_OBJECT

public:
	explicit DialogModelInformation(QWidget *parent = nullptr);
	~DialogModelInformation();
	void setModelInfo(ModelInfo* modelInfo);

public slots:
	void accept() override;
	void reject() override;

private:
	void _loadModelInfo();
	bool _hasPendingChanges() const;
	std::string _projectTitleFromUi() const;
	std::string _nameFromUi() const;
	std::string _versionFromUi() const;
	std::string _descriptionFromUi() const;
	std::string _analystNameFromUi() const;

	Ui::DialogModelInformation *ui;
	ModelInfo* _modelInfo = nullptr;
	std::string _originalProjectTitle;
	std::string _originalName;
	std::string _originalVersion;
	std::string _originalDescription;
	std::string _originalAnalystName;
};

#endif // DIALOGMODELINFORMATION_H
