/********************************************************************************
** Form generated from reading UI file 'ExpressionBuilder.ui'
**
** Created by: Qt User Interface Compiler version 6.4.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_EXPRESSIONBUILDER_H
#define UI_EXPRESSIONBUILDER_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ExpressionBuilder
{
public:
    QVBoxLayout *verticalLayout;
    QSplitter *splitter;
    QTreeWidget *treeCategories;
    QListWidget *listExpressions;
    QWidget *rightPanel;
    QVBoxLayout *verticalLayout_2;
    QLabel *labelParameters;
    QWidget *parameterWidget;
    QLabel *expressionPreview;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer;
    QPushButton *btnCopyToClipboard;
    QPushButton *btnClose;

    void setupUi(QDialog *ExpressionBuilder)
    {
        if (ExpressionBuilder->objectName().isEmpty())
            ExpressionBuilder->setObjectName("ExpressionBuilder");
        ExpressionBuilder->resize(800, 500);
        verticalLayout = new QVBoxLayout(ExpressionBuilder);
        verticalLayout->setObjectName("verticalLayout");
        splitter = new QSplitter(ExpressionBuilder);
        splitter->setObjectName("splitter");
        splitter->setOrientation(Qt::Horizontal);
        treeCategories = new QTreeWidget(splitter);
        treeCategories->setObjectName("treeCategories");
        treeCategories->setMinimumSize(QSize(150, 0));
        treeCategories->setMaximumSize(QSize(250, 16777215));
        splitter->addWidget(treeCategories);
        listExpressions = new QListWidget(splitter);
        listExpressions->setObjectName("listExpressions");
        listExpressions->setMinimumSize(QSize(200, 0));
        splitter->addWidget(listExpressions);
        rightPanel = new QWidget(splitter);
        rightPanel->setObjectName("rightPanel");
        verticalLayout_2 = new QVBoxLayout(rightPanel);
        verticalLayout_2->setObjectName("verticalLayout_2");
        labelParameters = new QLabel(rightPanel);
        labelParameters->setObjectName("labelParameters");

        verticalLayout_2->addWidget(labelParameters);

        parameterWidget = new QWidget(rightPanel);
        parameterWidget->setObjectName("parameterWidget");
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(parameterWidget->sizePolicy().hasHeightForWidth());
        parameterWidget->setSizePolicy(sizePolicy);

        verticalLayout_2->addWidget(parameterWidget);

        expressionPreview = new QLabel(rightPanel);
        expressionPreview->setObjectName("expressionPreview");
        expressionPreview->setMinimumSize(QSize(0, 60));
        QFont font;
        font.setPointSize(12);
        expressionPreview->setFont(font);
        expressionPreview->setWordWrap(true);
        expressionPreview->setMargin(10);
        expressionPreview->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);

        verticalLayout_2->addWidget(expressionPreview);

        splitter->addWidget(rightPanel);

        verticalLayout->addWidget(splitter);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName("horizontalLayout");
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        btnCopyToClipboard = new QPushButton(ExpressionBuilder);
        btnCopyToClipboard->setObjectName("btnCopyToClipboard");

        horizontalLayout->addWidget(btnCopyToClipboard);

        btnClose = new QPushButton(ExpressionBuilder);
        btnClose->setObjectName("btnClose");

        horizontalLayout->addWidget(btnClose);


        verticalLayout->addLayout(horizontalLayout);


        retranslateUi(ExpressionBuilder);

        QMetaObject::connectSlotsByName(ExpressionBuilder);
    } // setupUi

    void retranslateUi(QDialog *ExpressionBuilder)
    {
        ExpressionBuilder->setWindowTitle(QCoreApplication::translate("ExpressionBuilder", "Expression Builder", nullptr));
        QTreeWidgetItem *___qtreewidgetitem = treeCategories->headerItem();
        ___qtreewidgetitem->setText(0, QCoreApplication::translate("ExpressionBuilder", "Categories", nullptr));
        labelParameters->setText(QCoreApplication::translate("ExpressionBuilder", "<b>Parameters:</b>", nullptr));
        expressionPreview->setText(QCoreApplication::translate("ExpressionBuilder", "<b>Expression:</b> ", nullptr));
        btnCopyToClipboard->setText(QCoreApplication::translate("ExpressionBuilder", "Copy to Clipboard", nullptr));
        btnClose->setText(QCoreApplication::translate("ExpressionBuilder", "Close", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ExpressionBuilder: public Ui_ExpressionBuilder {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_EXPRESSIONBUILDER_H
