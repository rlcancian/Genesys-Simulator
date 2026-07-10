#include "DialogFind.h"

DialogFind::DialogFind(QWidget *parent, ModelGraphicsScene * scene)
    : QDialog(parent), scene(scene)
{
    counter = 0;
    QLabel *findLabel = new QLabel(tr("Enter the name of a component:"));
    lineEdit = new FindLineEdit;

    previousButton = new QPushButton(tr("&previous"));
    nextButton = new QPushButton(tr("&next"));
    findButton = new QPushButton(tr("&find"));


    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(findLabel);
    layout->addWidget(lineEdit);
    layout->addWidget(findButton);
    layout->addWidget(nextButton);
    layout->addWidget(previousButton);
    lineEdit->setPlaceholderText("Empty");

    setLayout(layout);
    setWindowTitle(tr("Find a component"));

    connect(findButton, &QPushButton::clicked,
            this, &DialogFind::findClicked);

    connect(nextButton, &QPushButton::clicked,
            this, &DialogFind::nextClicked);

    connect(previousButton, &QPushButton::clicked,
            this, &DialogFind::previousClicked);

    lineEdit->setFocus();
    previousButton->setEnabled(false);
    nextButton->setEnabled(false);


}

void DialogFind::findClicked()
{
    QString input = lineEdit->text().toLower();

    if (input.isEmpty()) {
        QMessageBox::information(this, tr("Empty Field"),
                                 tr("Please enter a component."));
        return;

    } else {

        finded->clear();
        QList<QGraphicsItem*>* copia = new QList<QGraphicsItem*>(*(scene->getGraphicalModelComponents()));

        int size = copia->size();

        //cria uma lista com todos os componentes que encontrar com aquela pesquisa
        for (int i = 0; i < size; i++) {

            GraphicalModelComponent * gmc = (GraphicalModelComponent * )copia->at(i);

            // Pega o nome de cada modelo
            QString componentName = QString::fromStdString(gmc->getComponent()->getName()).toLower();

            if (componentName.contains(input)){
                finded->append(copia->at(i));
            }
        }

        // Encontrou
        if (!finded->isEmpty()) {

            if (!finded->at(0)->isSelected()) {
                scene->clearSelection();
                finded->at(0)->setSelected(true);
                counter = 0;
            }

            if (finded->size() > 1){
                previousButton->setEnabled(true);
                nextButton->setEnabled(true);
                nextButton->setFocus();
            }

            // Não encontrou
        } else {
            lineEdit->setStyleSheet("color: red");
        }
    }
}

void DialogFind::nextClicked()
{
    if (finded->empty() || finded->size()== 1) return;

    counter++;
    // Se for o ultimo
    if (counter == finded->size() -1)
    {
        nextButton->setEnabled(false);
        previousButton->setFocus();

    }
    previousButton->setEnabled(true);
    // show
    GraphicalModelComponent * gmc = (GraphicalModelComponent *)finded->at(counter);

    scene->clearSelection();

    gmc->setSelected(true);

}

void DialogFind::previousClicked()
{
    if (finded->isEmpty() || finded->size()==1) return;

    counter--;
    // Se for o primeiro
    if (counter == 0)

    {
        previousButton->setEnabled(false);
        nextButton->setFocus();
    }

    nextButton->setEnabled(true);
    // show
    GraphicalModelComponent * gmc = (GraphicalModelComponent *) finded->at(counter);

    if (!gmc->isSelected()) {
        scene->clearSelection();
        gmc->setSelected(true);
    }


}




