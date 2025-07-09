#include "PropertyEditor.h"
#include "../../../../kernel/simulator/ModelDataDefinition.h"
#include <QLineEdit>

PropertyEditor::PropertyEditor(QWidget *parent)
: QTreeWidget(parent) {
	setColumnCount(2);
	setAlternatingRowColors(true);
	setAnimated(true);
	setContentsMargins(1, 1, 1, 1);
	setAutoExpandDelay(10);
	//setExpanded(0,true);
	setIndentation(1);
	QTreeWidgetItem *header = headerItem();
	header->setText(0, "Name");
	header->setText(1, "Value");
	header->setExpanded(true);
}

void PropertyEditor::setModelBlock(ModelDataDefinition* modelblock) {
	this->clear();
	QTreeWidgetItem *treeRootItem;
	QString category;
	QBrush brush;
	//PropertyT<double>* propDouble;
	//PropertyT<unsigned int>* propUInt;
	//PropertyT<bool>* propBool;
	//PropertyT<std::string>* propStr;
	//PropertyT<Util::TimeUnit>* propTUnit;
	for (auto prop : *modelblock->getProperties()->list()) {
		//propDouble = dynamic_cast<PropertyT<double>*>(prop);
		//propUInt = dynamic_cast<PropertyT<unsigned int>*>(prop);
		//propBool = dynamic_cast<PropertyT<bool>*>(prop);
		//propStr = dynamic_cast<PropertyT<std::string>*>(prop);
		//propTUnit = dynamic_cast<PropertyT<Util::TimeUnit>*>(prop);
		category = QString::fromStdString(prop->getName()); ///!@TODO Era para ser getClassName());
		QList<QTreeWidgetItem*> founds = findItems(category, Qt::MatchContains);
		if (founds.size() == 0) {
			treeRootItem = new QTreeWidgetItem(this);
			treeRootItem->setText(0, category);
			QBrush bforeground(Qt::white);
			QFont font("Nimbus Sans", 12, QFont::Bold);
			QBrush bbackground(Qt::black);
			treeRootItem->setForeground(0, bforeground);
			if (category == "ModelDataDefinition") {
				bbackground.setColor(Qt::darkRed);
			} else if (category == "ModelComponent") {
				bbackground.setColor(Qt::darkGreen);
			} else if (category == "ModelSourceComponent") {
				bbackground.setColor(Qt::darkYellow);
			} else if (category == "ModelSinkComponent") {
				bbackground.setColor(Qt::magenta);
			} else {
				bbackground.setColor(Qt::darkBlue);
			}
			treeRootItem->setFirstColumnSpanned(true);
			treeRootItem->setBackground(0, bbackground);
			treeRootItem->setFont(0, font);
			treeRootItem->setExpanded(true);
			treeRootItem->sortChildren(0, Qt::AscendingOrder);
		} else {
			treeRootItem = *founds.begin();
		}
		QTreeWidgetItem *treeItemChild = new QTreeWidgetItem();
		//treeItemChild->setWhatsThis(0, QString::fromStdString(plugin->getPluginInfo()->getPluginTypename()));
		//treeItemChild->setTextColor(0, treeRootItem->backgroundColor(0));
		treeItemChild->setText(0, QString::fromStdString(prop->getName()));
		//treeItemChild->setText(1, QString::fromStdString(std::to_string(prop->getValue())));
		//treeItemChild->setToolTip(0, QString::fromStdString(plugtextAdds));
		//treeItemChild->setStatusTip(0, QString::fromStdString(plugin->getPluginInfo()->getLanguageTemplate()));
		//treeItemChild->setFlags(Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemNeverHasChildren);
		treeRootItem->addChild(treeItemChild);
		QWidget* lineEdit = new QLineEdit(QString::fromStdString(prop->getValue()));
		this->setItemWidget(treeItemChild, 1, lineEdit);
	}
	resizeColumnToContents(0);
}
