#include "ExpressionBuilder.h"
#include "ui_ExpressionBuilder.h"
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
    connect(treeCategories, &QTreeWidget::itemClicked, this, &ExpressionBuilder::on_categorySelected);
    connect(listExpressions, &QListWidget::itemClicked, this, &ExpressionBuilder::on_expressionSelected);
    connect(btnCopyToClipboard, &QPushButton::clicked, this, &ExpressionBuilder::on_copyToClipboard_clicked);
    connect(btnClose, &QPushButton::clicked, this, &ExpressionBuilder::on_close_clicked);
    
    // Initialize expression categories
    initializeCategories();
    
    // Populate tree with categories
    for (const auto &category : categories) {
        QTreeWidgetItem *item = new QTreeWidgetItem(treeCategories);
        item->setText(0, category.name);
        item->setData(0, Qt::UserRole, QVariant::fromValue(category.name));
    }
    
    treeCategories->expandAll();
}

ExpressionBuilder::~ExpressionBuilder()
{
    delete ui;
}

void ExpressionBuilder::initializeCategories()
{
    // Math Category
    ExpressionCategory mathCategory;
    mathCategory.name = "Math";
    
    // abs(x)
    ExpressionTemplate absExpr;
    absExpr.name = "abs(x)";
    absExpr.pattern = "abs({x})";
    absExpr.description = "Absolute value of x";
    absExpr.parameters.append({"x", "0", "Value"});
    mathCategory.expressions.append(absExpr);
    
    // sqrt(x)
    ExpressionTemplate sqrtExpr;
    sqrtExpr.name = "sqrt(x)";
    sqrtExpr.pattern = "sqrt({x})";
    sqrtExpr.description = "Square root of x";
    sqrtExpr.parameters.append({"x", "1", "Value"});
    mathCategory.expressions.append(sqrtExpr);
    
    // pow(x, y)
    ExpressionTemplate powExpr;
    powExpr.name = "pow(x, y)";
    powExpr.pattern = "pow({x}, {y})";
    powExpr.description = "x raised to the power of y";
    powExpr.parameters.append({"x", "1", "Base"});
    powExpr.parameters.append({"y", "2", "Exponent"});
    mathCategory.expressions.append(powExpr);
    
    // min(x, y)
    ExpressionTemplate minExpr;
    minExpr.name = "min(x, y)";
    minExpr.pattern = "min({x}, {y})";
    minExpr.description = "Minimum of x and y";
    minExpr.parameters.append({"x", "0", "First value"});
    minExpr.parameters.append({"y", "0", "Second value"});
    mathCategory.expressions.append(minExpr);
    
    // max(x, y)
    ExpressionTemplate maxExpr;
    maxExpr.name = "max(x, y)";
    maxExpr.pattern = "max({x}, {y})";
    maxExpr.description = "Maximum of x and y";
    maxExpr.parameters.append({"x", "0", "First value"});
    maxExpr.parameters.append({"y", "0", "Second value"});
    mathCategory.expressions.append(maxExpr);
    
    // sin(x)
    ExpressionTemplate sinExpr;
    sinExpr.name = "sin(x)";
    sinExpr.pattern = "sin({x})";
    sinExpr.description = "Sine of x (in radians)";
    sinExpr.parameters.append({"x", "0", "Angle in radians"});
    mathCategory.expressions.append(sinExpr);
    
    // cos(x)
    ExpressionTemplate cosExpr;
    cosExpr.name = "cos(x)";
    cosExpr.pattern = "cos({x})";
    cosExpr.description = "Cosine of x (in radians)";
    cosExpr.parameters.append({"x", "0", "Angle in radians"});
    mathCategory.expressions.append(cosExpr);
    
    // log(x)
    ExpressionTemplate logExpr;
    logExpr.name = "log(x)";
    logExpr.pattern = "log({x})";
    logExpr.description = "Natural logarithm of x";
    logExpr.parameters.append({"x", "1", "Value"});
    mathCategory.expressions.append(logExpr);
    
    // exp(x)
    ExpressionTemplate expExpr;
    expExpr.name = "exp(x)";
    expExpr.pattern = "exp({x})";
    expExpr.description = "e raised to the power of x";
    expExpr.parameters.append({"x", "0", "Exponent"});
    mathCategory.expressions.append(expExpr);
    
    categories.append(mathCategory);
    
    // Distributions Category
    ExpressionCategory distCategory;
    distCategory.name = "Distributions";
    
    // EXPO(mean)
    ExpressionTemplate expoExpr;
    expoExpr.name = "EXPO(mean)";
    expoExpr.pattern = "EXPO({mean})";
    expoExpr.description = "Exponential distribution";
    expoExpr.parameters.append({"mean", "1.0", "Mean value"});
    distCategory.expressions.append(expoExpr);
    
    // NORM(mean, stddev)
    ExpressionTemplate normExpr;
    normExpr.name = "NORM(mean, stddev)";
    normExpr.pattern = "NORM({mean}, {stddev})";
    normExpr.description = "Normal distribution";
    normExpr.parameters.append({"mean", "0.0", "Mean value"});
    normExpr.parameters.append({"stddev", "1.0", "Standard deviation"});
    distCategory.expressions.append(normExpr);
    
    // UNIF(min, max)
    ExpressionTemplate unifExpr;
    unifExpr.name = "UNIF(min, max)";
    unifExpr.pattern = "UNIF({min}, {max})";
    unifExpr.description = "Uniform distribution";
    unifExpr.parameters.append({"min", "0.0", "Minimum value"});
    unifExpr.parameters.append({"max", "1.0", "Maximum value"});
    distCategory.expressions.append(unifExpr);
    
    // TRIA(min, mode, max)
    ExpressionTemplate triaExpr;
    triaExpr.name = "TRIA(min, mode, max)";
    triaExpr.pattern = "TRIA({min}, {mode}, {max})";
    triaExpr.description = "Triangular distribution";
    triaExpr.parameters.append({"min", "0.0", "Minimum value"});
    triaExpr.parameters.append({"mode", "0.5", "Mode (most likely) value"});
    triaExpr.parameters.append({"max", "1.0", "Maximum value"});
    distCategory.expressions.append(triaExpr);
    
    // WEIB(alpha, beta)
    ExpressionTemplate weibExpr;
    weibExpr.name = "WEIB(alpha, beta)";
    weibExpr.pattern = "WEIB({alpha}, {beta})";
    weibExpr.description = "Weibull distribution";
    weibExpr.parameters.append({"alpha", "1.0", "Shape parameter"});
    weibExpr.parameters.append({"beta", "1.0", "Scale parameter"});
    distCategory.expressions.append(weibExpr);
    
    // GAMM(alpha, beta)
    ExpressionTemplate gammExpr;
    gammExpr.name = "GAMM(alpha, beta)";
    gammExpr.pattern = "GAMM({alpha}, {beta})";
    gammExpr.description = "Gamma distribution";
    gammExpr.parameters.append({"alpha", "1.0", "Shape parameter"});
    gammExpr.parameters.append({"beta", "1.0", "Scale parameter"});
    distCategory.expressions.append(gammExpr);
    
    // LOGN(mean, stddev)
    ExpressionTemplate lognExpr;
    lognExpr.name = "LOGN(mean, stddev)";
    lognExpr.pattern = "LOGN({mean}, {stddev})";
    lognExpr.description = "Lognormal distribution";
    lognExpr.parameters.append({"mean", "0.0", "Mean of ln(X)"});
    lognExpr.parameters.append({"stddev", "1.0", "Std dev of ln(X)"});
    distCategory.expressions.append(lognExpr);
    
    categories.append(distCategory);
}

void ExpressionBuilder::on_categorySelected(QTreeWidgetItem *item, int column)
{
    if (!item) return;
    
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
    if (!item) return;
    
    QString expressionName = item->data(Qt::UserRole).toString();
    
    // Find the expression
    currentExpression = nullptr;
    for (auto &category : categories) {
        for (auto &expression : category.expressions) {
            if (expression.name == expressionName) {
                currentExpression = &expression;
                break;
            }
        }
        if (currentExpression) break;
    }
    
    if (!currentExpression) return;
    
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
        parameterLayout->addRow(param.name + ":", input);
    }
}

void ExpressionBuilder::updateExpressionPreview()
{
    QString expression = buildExpression();
    expressionPreview->setText("<b>Expression:</b> " + expression);
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
    QString expression = buildExpression();
    if (expression.isEmpty()) {
        QMessageBox::warning(this, "Expression Builder", "Please select an expression first.");
        return;
    }
    
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(expression);
    
    QMessageBox::information(this, "Expression Builder", "Expression copied to clipboard:\n" + expression);
}

void ExpressionBuilder::on_close_clicked()
{
    this->close();
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
