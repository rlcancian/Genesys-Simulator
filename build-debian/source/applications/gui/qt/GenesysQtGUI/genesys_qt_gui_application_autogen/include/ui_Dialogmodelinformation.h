/********************************************************************************
** Form generated from reading UI file 'Dialogmodelinformation.ui'
**
** Created by: Qt User Interface Compiler version 6.4.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DIALOGMODELINFORMATION_H
#define UI_DIALOGMODELINFORMATION_H

#include <QtCore/QVariant>
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_DialogModelInformation
{
public:
    QVBoxLayout *verticalLayout;
    QFormLayout *formLayout_2;
    QLabel *label;
    QPlainTextEdit *plainTextProjectName;
    QLabel *label_2;
    QPlainTextEdit *plainTextModelName;
    QLabel *label_3;
    QPlainTextEdit *plainTextModelDescription;
    QLabel *label_4;
    QPlainTextEdit *plainTextAnalystName;
    QPlainTextEdit *plainTextModelVersion;
    QLabel *label_5;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *DialogModelInformation)
    {
        if (DialogModelInformation->objectName().isEmpty())
            DialogModelInformation->setObjectName("DialogModelInformation");
        DialogModelInformation->setWindowModality(Qt::WindowModal);
        DialogModelInformation->resize(554, 351);
        verticalLayout = new QVBoxLayout(DialogModelInformation);
        verticalLayout->setObjectName("verticalLayout");
        formLayout_2 = new QFormLayout();
        formLayout_2->setObjectName("formLayout_2");
        label = new QLabel(DialogModelInformation);
        label->setObjectName("label");

        formLayout_2->setWidget(0, QFormLayout::LabelRole, label);

        plainTextProjectName = new QPlainTextEdit(DialogModelInformation);
        plainTextProjectName->setObjectName("plainTextProjectName");
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(plainTextProjectName->sizePolicy().hasHeightForWidth());
        plainTextProjectName->setSizePolicy(sizePolicy);
        plainTextProjectName->setMaximumSize(QSize(16777215, 32));

        formLayout_2->setWidget(0, QFormLayout::FieldRole, plainTextProjectName);

        label_2 = new QLabel(DialogModelInformation);
        label_2->setObjectName("label_2");

        formLayout_2->setWidget(1, QFormLayout::LabelRole, label_2);

        plainTextModelName = new QPlainTextEdit(DialogModelInformation);
        plainTextModelName->setObjectName("plainTextModelName");
        sizePolicy.setHeightForWidth(plainTextModelName->sizePolicy().hasHeightForWidth());
        plainTextModelName->setSizePolicy(sizePolicy);
        plainTextModelName->setMaximumSize(QSize(16777215, 32));

        formLayout_2->setWidget(1, QFormLayout::FieldRole, plainTextModelName);

        label_3 = new QLabel(DialogModelInformation);
        label_3->setObjectName("label_3");

        formLayout_2->setWidget(3, QFormLayout::LabelRole, label_3);

        plainTextModelDescription = new QPlainTextEdit(DialogModelInformation);
        plainTextModelDescription->setObjectName("plainTextModelDescription");

        formLayout_2->setWidget(3, QFormLayout::FieldRole, plainTextModelDescription);

        label_4 = new QLabel(DialogModelInformation);
        label_4->setObjectName("label_4");

        formLayout_2->setWidget(4, QFormLayout::LabelRole, label_4);

        plainTextAnalystName = new QPlainTextEdit(DialogModelInformation);
        plainTextAnalystName->setObjectName("plainTextAnalystName");
        sizePolicy.setHeightForWidth(plainTextAnalystName->sizePolicy().hasHeightForWidth());
        plainTextAnalystName->setSizePolicy(sizePolicy);
        plainTextAnalystName->setMaximumSize(QSize(16777215, 32));

        formLayout_2->setWidget(4, QFormLayout::FieldRole, plainTextAnalystName);

        plainTextModelVersion = new QPlainTextEdit(DialogModelInformation);
        plainTextModelVersion->setObjectName("plainTextModelVersion");
        sizePolicy.setHeightForWidth(plainTextModelVersion->sizePolicy().hasHeightForWidth());
        plainTextModelVersion->setSizePolicy(sizePolicy);
        plainTextModelVersion->setMaximumSize(QSize(16777215, 32));

        formLayout_2->setWidget(2, QFormLayout::FieldRole, plainTextModelVersion);

        label_5 = new QLabel(DialogModelInformation);
        label_5->setObjectName("label_5");

        formLayout_2->setWidget(2, QFormLayout::LabelRole, label_5);


        verticalLayout->addLayout(formLayout_2);

        buttonBox = new QDialogButtonBox(DialogModelInformation);
        buttonBox->setObjectName("buttonBox");
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout->addWidget(buttonBox);

#if QT_CONFIG(shortcut)
        label->setBuddy(plainTextProjectName);
        label_2->setBuddy(plainTextModelName);
        label_3->setBuddy(plainTextModelDescription);
        label_4->setBuddy(plainTextAnalystName);
        label_5->setBuddy(plainTextModelVersion);
#endif // QT_CONFIG(shortcut)
        QWidget::setTabOrder(plainTextProjectName, plainTextModelName);
        QWidget::setTabOrder(plainTextModelName, plainTextModelVersion);
        QWidget::setTabOrder(plainTextModelVersion, plainTextModelDescription);
        QWidget::setTabOrder(plainTextModelDescription, plainTextAnalystName);

        retranslateUi(DialogModelInformation);
        QObject::connect(buttonBox, &QDialogButtonBox::accepted, DialogModelInformation, qOverload<>(&QDialog::accept));
        QObject::connect(buttonBox, &QDialogButtonBox::rejected, DialogModelInformation, qOverload<>(&QDialog::reject));

        QMetaObject::connectSlotsByName(DialogModelInformation);
    } // setupUi

    void retranslateUi(QDialog *DialogModelInformation)
    {
        DialogModelInformation->setWindowTitle(QCoreApplication::translate("DialogModelInformation", "Model Information", nullptr));
        label->setText(QCoreApplication::translate("DialogModelInformation", "Project Title:", nullptr));
        label_2->setText(QCoreApplication::translate("DialogModelInformation", "Model Name:", nullptr));
        label_3->setText(QCoreApplication::translate("DialogModelInformation", "Model Description:", nullptr));
        label_4->setText(QCoreApplication::translate("DialogModelInformation", "Analyst Name:", nullptr));
        label_5->setText(QCoreApplication::translate("DialogModelInformation", "Model Version:", nullptr));
    } // retranslateUi

};

namespace Ui {
    class DialogModelInformation: public Ui_DialogModelInformation {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DIALOGMODELINFORMATION_H
