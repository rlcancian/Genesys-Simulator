#ifndef DIALOGFIND_H
#define DIALOGFIND_H

#include <QDialog>
#include <QLabel>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>
#include "../graphicals/GraphicalModelComponent.h"
#include "graphicals/ModelGraphicsScene.h"
#include <QGraphicsItem>
#include <QList>


class QLineEdit;
class QPushButton;

class FindLineEdit : public QLineEdit {
    Q_OBJECT

public:
    FindLineEdit(QWidget *parent = nullptr) : QLineEdit(parent) {
        connect(this, &QLineEdit::textChanged, this, &FindLineEdit::onTextChanged);
    }

private slots:
    void onTextChanged(const QString &text) {
//        qDebug() << "Texto atual: " << text;

        if (text.length() < previousText.length()) {
            this->setStyleSheet("color: black");
        }

        previousText = text;
    }

private:
    QString previousText;
};


class DialogFind : public QDialog
{
    Q_OBJECT

public:
    DialogFind(QWidget *parent = nullptr, ModelGraphicsScene * scene = nullptr);
    QString getFindText();

public slots:
    void findClicked();
    void nextClicked();
    void previousClicked();

private:
    QPushButton *findButton;
    QPushButton *nextButton;
    QPushButton *previousButton;
    FindLineEdit *lineEdit;
    bool clicked;
    int counter;
    QList<QGraphicsItem*>* finded = new QList<QGraphicsItem*>();
    ModelGraphicsScene * scene;

};

#endif // DIALOGFIND_H


