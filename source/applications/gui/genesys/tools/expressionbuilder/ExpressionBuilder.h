#ifndef EXPRESSIONBUILDER_H
#define EXPRESSIONBUILDER_H

#include <QDialog>
#include <QTreeWidget>
#include <QListWidget>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QClipboard>
#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QString>
#include <QMap>
#include <QList>

namespace Ui {
class ExpressionBuilder;
}

struct ExpressionParameter {
    QString name;
    QString defaultValue;
    QString description;
};

struct ExpressionTemplate {
    QString name;
    QString pattern;
    QString description;
    QList<ExpressionParameter> parameters;
};

struct ExpressionCategory {
    QString name;
    QList<ExpressionTemplate> expressions;
};

class ExpressionBuilder : public QDialog
{
    Q_OBJECT

public:
    explicit ExpressionBuilder(QWidget *parent = nullptr);
    ~ExpressionBuilder();

private slots:
    void on_categorySelected(QTreeWidgetItem *item, int column);
    void on_expressionSelected(QListWidgetItem *item);
    void on_copyToClipboard_clicked();
    void on_close_clicked();

private:
    Ui::ExpressionBuilder *ui;
    
    // Expression data
    QList<ExpressionCategory> categories;
    
    // UI Components
    QTreeWidget *treeCategories;
    QListWidget *listExpressions;
    QWidget *parameterWidget;
    QFormLayout *parameterLayout;
    QLabel *expressionPreview;
    QPushButton *btnCopyToClipboard;
    QPushButton *btnClose;
    
    // Parameter inputs
    QMap<QString, QLineEdit*> parameterInputs;
    
    // Current selection
    ExpressionTemplate *currentExpression;
    
    // Initialize expression categories
    void initializeCategories();
    void updateParameterForm();
    void updateExpressionPreview();
    QString buildExpression();
    
    // Extensibility: Register new categories and expressions
    void registerCategory(const ExpressionCategory &category);
    void registerExpression(const QString &categoryName, const ExpressionTemplate &expression);
};

#endif // EXPRESSIONBUILDER_H
