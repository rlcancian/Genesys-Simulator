#include "ExpressionBuilder.h"
#include "ui_ExpressionBuilder.h"
#include "ParserFunctionCatalog.h"
#include "parser/Genesys++-driver.h"
#include <QStringList>
#include <QTextCursor>
#include <algorithm>
#include <iterator>
#include <QMessageBox>

ExpressionBuilder::ExpressionBuilder(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ExpressionBuilder),
    currentExpression(nullptr)
{
    ui->setupUi(this);
    
    // Get references to UI components from the UI file
    treeCategories = ui->treeCategories;
    listExpressions = ui->listExpressions;
    parameterWidget = ui->parameterWidget;
    expressionPreview = ui->expressionPreview;
    btnCopyToClipboard = ui->btnCopyToClipboard;
    btnClose = ui->btnClose;
    
    // Initialize parameter layout
    parameterLayout = new QFormLayout();
    parameterWidget->setLayout(parameterLayout);
    
    // Connect signals
    // Respond to keyboard and mouse selection, not just itemClicked.
    connect(treeCategories, &QTreeWidget::currentItemChanged, this,
            [this](QTreeWidgetItem* current, QTreeWidgetItem*) {
                on_categorySelected(current, 0);
            });
    connect(listExpressions, &QListWidget::currentItemChanged, this,
            [this](QListWidgetItem* current, QListWidgetItem*) {
                on_expressionSelected(current);
            });
    connect(btnCopyToClipboard, &QPushButton::clicked, this, &ExpressionBuilder::on_copyToClipboard_clicked);
    connect(btnClose, &QPushButton::clicked, this, &ExpressionBuilder::on_close_clicked);
    connect(ui->btnInsert, &QPushButton::clicked, this, &ExpressionBuilder::insertSelectedExpression);
    connect(ui->btnValidate, &QPushButton::clicked, this, [this]() {
        (void)validateCurrentExpression();
    });
    connect(ui->btnApply, &QPushButton::clicked, this, [this]() {
        if (validateCurrentExpression()) accept();
    });
    connect(listExpressions, &QListWidget::itemDoubleClicked, this,
            [this](QListWidgetItem*) { insertSelectedExpression(); });
    connect(ui->expressionEditor, &QPlainTextEdit::textChanged, this,
            [this]() { ui->validationStatus->setText(tr("Not validated")); });
    
    // Initialize expression categories
    initializeCategories();
    
    // Populate tree with categories
    for (const auto &category : categories) {
        QTreeWidgetItem *item = new QTreeWidgetItem(treeCategories);
        item->setText(0, category.name);
        item->setData(0, Qt::UserRole, QVariant::fromValue(category.name));
    }
    
    treeCategories->expandAll();
    if (treeCategories->topLevelItemCount() > 0) {
        QTreeWidgetItem* firstCategory = treeCategories->topLevelItem(0);
        treeCategories->setCurrentItem(firstCategory);
    }
}

ExpressionBuilder::~ExpressionBuilder()
{
    delete ui;
}

void ExpressionBuilder::initializeCategories()
{
    // The parser's function reference and this editor share one catalog.
    // Exclude token-only/stub functions at its source, not individually here.
    for (const ParserFunctionInfo& info : parserFunctionCatalog()) {
        if (info.name == QStringLiteral("operators")) {
            continue;
        }

        ExpressionTemplate expression;
        expression.name = info.syntax;
        expression.description = info.description;
        const int opening = info.syntax.indexOf(QLatin1Char('('));
        const int closing = info.syntax.lastIndexOf(QLatin1Char(')'));
        if (opening < 0 || closing < opening) {
            expression.pattern = info.syntax;
        } else {
            const QStringList parameters = info.syntax.mid(opening + 1, closing - opening - 1)
                                               .split(QLatin1Char(','), Qt::SkipEmptyParts);
            // Use the catalog's tested examples as editable argument defaults.
            // A nested expression such as ln(exp(1)) must remain one argument.
            QStringList exampleArguments;
            const int exampleOpening = info.example.indexOf(QLatin1Char('('));
            const int exampleClosing = info.example.lastIndexOf(QLatin1Char(')'));
            if (exampleOpening >= 0 && exampleClosing > exampleOpening) {
                const QString contents = info.example.mid(exampleOpening + 1,
                    exampleClosing - exampleOpening - 1);
                QString argument;
                int nesting = 0;
                for (const QChar character : contents) {
                    if (character == QLatin1Char('(')) ++nesting;
                    else if (character == QLatin1Char(')')) --nesting;
                    if (character == QLatin1Char(',') && nesting == 0) {
                        exampleArguments.append(argument.trimmed());
                        argument.clear();
                    } else {
                        argument.append(character);
                    }
                }
                exampleArguments.append(argument.trimmed());
            }

            QStringList placeholders;
            for (int index = 0; index < parameters.size(); ++index) {
                const QString key = QStringLiteral("arg%1").arg(index + 1);
                const QString meaning = parameters.at(index).trimmed();
                const QString defaultValue = exampleArguments.size() == parameters.size()
                    ? exampleArguments.at(index) : QStringLiteral("1");
                expression.parameters.append({key, defaultValue, meaning});
                placeholders.append(QStringLiteral("{%1}").arg(key));
            }
            expression.pattern = info.name + QLatin1Char('(')
                + placeholders.join(QStringLiteral(", ")) + QLatin1Char(')');
        }

        auto it = std::find_if(categories.begin(), categories.end(),
            [&info](const ExpressionCategory& category) { return category.name == info.category; });
        if (it == categories.end()) {
            categories.append({info.category, {}});
            it = std::prev(categories.end());
        }
        it->expressions.append(expression);
    }

    ExpressionCategory operators;
    operators.name = tr("Operators");
    for (const QString& symbol : {QStringLiteral("+"), QStringLiteral("-"),
            QStringLiteral("*"), QStringLiteral("/"), QStringLiteral("^"),
            QStringLiteral("<"), QStringLiteral(">"), QStringLiteral("<="),
            QStringLiteral(">="), QStringLiteral("=="), QStringLiteral("!="),
            QStringLiteral("and"), QStringLiteral("or"), QStringLiteral("nand"),
            QStringLiteral("xor"), QStringLiteral("not"), QStringLiteral("="),
            QStringLiteral("<>"), QStringLiteral("&&"), QStringLiteral("||"),
            QStringLiteral("!"), QStringLiteral("("), QStringLiteral(")"),
            QStringLiteral("["), QStringLiteral("]"), QStringLiteral(",")}) {
        const bool delimiter = symbol == QLatin1String("(") || symbol == QLatin1String(")")
            || symbol == QLatin1String("[") || symbol == QLatin1String("]")
            || symbol == QLatin1String(",");
        const QString insertedText = delimiter ? symbol : QStringLiteral(" ") + symbol + QStringLiteral(" ");
        operators.expressions.append({symbol, insertedText, tr("Insert a grammar operator"), {}});
    }
    categories.append(operators);
}

void ExpressionBuilder::on_categorySelected(QTreeWidgetItem *item, int column)
{
    if (!item) return;
    
    currentExpression = nullptr;
    updateParameterForm();
    expressionPreview->clear();
    QString categoryName = item->data(0, Qt::UserRole).toString();
    
    // Find the category
    ExpressionCategory *selectedCategory = nullptr;
    for (auto &category : categories) {
        if (category.name == categoryName) {
            selectedCategory = &category;
            break;
        }
    }
    
    if (!selectedCategory) return;
    
    // Populate expressions list
    listExpressions->clear();
    for (const auto &expression : selectedCategory->expressions) {
        QListWidgetItem *item = new QListWidgetItem(expression.name);
        item->setData(Qt::UserRole, QVariant::fromValue(expression.name));
        item->setToolTip(expression.description);
        listExpressions->addItem(item);
    }
}

void ExpressionBuilder::on_expressionSelected(QListWidgetItem *item)
{
    currentExpression = nullptr;
    if (item == nullptr || treeCategories->currentItem() == nullptr) return;
    const QString categoryName = treeCategories->currentItem()->data(0, Qt::UserRole).toString();
    const int row = listExpressions->row(item);
    for (ExpressionCategory& category : categories) {
        if (category.name == categoryName && row >= 0 && row < category.expressions.size()) {
            currentExpression = &category.expressions[row];
            break;
        }
    }
    updateParameterForm();
    updateExpressionPreview();
}

void ExpressionBuilder::updateParameterForm()
{
    // Clear existing parameter inputs
    QLayoutItem *item;
    while ((item = parameterLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }
    parameterInputs.clear();
    
    if (!currentExpression) return;
    
    // Add parameter inputs
    for (const auto &param : currentExpression->parameters) {
        QLineEdit *input = new QLineEdit();
        input->setText(param.defaultValue);
        input->setPlaceholderText(param.description);
        
        // Connect to update preview on change
        connect(input, &QLineEdit::textChanged, this, &ExpressionBuilder::updateExpressionPreview);
        
        parameterInputs[param.name] = input;
        parameterLayout->addRow(param.description + ":", input);
    }
}

void ExpressionBuilder::updateExpressionPreview()
{
    QString expression = buildExpression();
    expressionPreview->setText("<b>Expression:</b> " + expression.toHtmlEscaped());
}

QString ExpressionBuilder::buildExpression()
{
    if (!currentExpression) return "";
    
    QString result = currentExpression->pattern;
    
    // Replace placeholders with actual values
    for (const auto &param : currentExpression->parameters) {
        QString placeholder = "{" + param.name + "}";
        QString value = parameterInputs.contains(param.name) ? 
                        parameterInputs[param.name]->text() : 
                        param.defaultValue;
        result.replace(placeholder, value);
    }
    
    return result;
}

void ExpressionBuilder::on_copyToClipboard_clicked()
{
    // Preserve the historical standalone function-copy workflow when no full
    // expression has been entered in the new editor.
    const QString expression = expressionText().isEmpty() ? buildExpression() : expressionText();
    if (expression.isEmpty()) {
        QMessageBox::warning(this, tr("Expression Builder"), tr("The expression is empty."));
        return;
    }
    QApplication::clipboard()->setText(expression);
}

void ExpressionBuilder::on_close_clicked()
{
    reject();
}

void ExpressionBuilder::setExpressionContext(Model* model, const QString& currentExpression)
{
    _model = model;
    ui->expressionEditor->setPlainText(currentExpression);
    ui->btnClose->setText(tr("Cancel"));
    ui->validationStatus->setText(tr("Not validated"));
}

QString ExpressionBuilder::expressionText() const
{
    return ui->expressionEditor->toPlainText();
}

bool ExpressionBuilder::validateCurrentExpression()
{
    try {
        genesyspp_driver driver(_model, nullptr, false);
        const ExpressionSyntaxValidation result = driver.validateSyntax(expressionText().toStdString());
        if (result.isValid()) {
            ui->validationStatus->setText(tr("Syntax valid (expression not evaluated)."));
            return true;
        }
        if (result.status == ExpressionSyntaxValidation::Status::UnresolvedIdentifiers) {
            ui->validationStatus->setText(tr("Unresolved model identifier: %1")
                                          .arg(QString::fromStdString(result.errorMessage)));
        } else {
            ui->validationStatus->setText(tr("Invalid expression: %1")
                                          .arg(QString::fromStdString(result.errorMessage)));
        }
    } catch (const std::exception& error) {
        ui->validationStatus->setText(tr("Validation failed: %1").arg(QString::fromUtf8(error.what())));
    } catch (const std::string& error) {
        ui->validationStatus->setText(tr("Validation failed: %1").arg(QString::fromStdString(error)));
    }
    return false;
}

void ExpressionBuilder::insertSelectedExpression()
{
    if (currentExpression == nullptr) {
        ui->validationStatus->setText(tr("Select a function or operator to insert."));
        return;
    }
    const QString snippet = buildExpression();
    QTextCursor cursor = ui->expressionEditor->textCursor();
    cursor.insertText(snippet);
    ui->expressionEditor->setTextCursor(cursor);
    ui->expressionEditor->setFocus();
}

void ExpressionBuilder::registerCategory(const ExpressionCategory &category)
{
    categories.append(category);
    
    // Add to tree widget
    QTreeWidgetItem *item = new QTreeWidgetItem(treeCategories);
    item->setText(0, category.name);
    item->setData(0, Qt::UserRole, QVariant::fromValue(category.name));
}

void ExpressionBuilder::registerExpression(const QString &categoryName, const ExpressionTemplate &expression)
{
    // Find the category
    for (auto &category : categories) {
        if (category.name == categoryName) {
            category.expressions.append(expression);
            break;
        }
    }
}
