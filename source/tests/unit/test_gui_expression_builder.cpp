#include <gtest/gtest.h>
#include <QSet>
#include <QString>

#include "applications/gui/genesys/tools/expressionbuilder/ParserFunctionCatalog.h"
#include "parser/Genesys++-driver.h"

TEST(ExpressionBuilderCatalogTest, OnlyAdvertisesImplementedGrammarEntries) {
    const auto entries = parserFunctionCatalog();
    ASSERT_FALSE(entries.isEmpty());
    const QSet<QString> tokenOnlyOrStub = {
        QStringLiteral("abs"), QStringLiteral("pow"),
        QStringLiteral("disc"), QStringLiteral("lastinq"),
        QStringLiteral("resseizes"), QStringLiteral("numgr"),
        QStringLiteral("atrgr"), QStringLiteral("resutil")
    };
    QSet<QString> signatures;
    for (const auto& entry : entries) {
        EXPECT_FALSE(entry.category.isEmpty());
        EXPECT_FALSE(entry.name.isEmpty());
        EXPECT_FALSE(entry.syntax.isEmpty());
        EXPECT_FALSE(entry.description.isEmpty());
        EXPECT_FALSE(tokenOnlyOrStub.contains(entry.name)) << entry.name.toStdString();
        const QString signature = entry.name + QLatin1Char(':') + entry.syntax;
        EXPECT_FALSE(signatures.contains(signature)) << signature.toStdString();
        signatures.insert(signature);
    }
}

TEST(ExpressionBuilderCatalogTest, EveryExecutableExampleHasValidOrContextDependentSyntax) {
    genesyspp_driver driver(nullptr, nullptr, false);
    for (const auto& entry : parserFunctionCatalog()) {
        if (entry.name == QStringLiteral("operators")) {
            continue; // This is a reference row rather than an insertion template.
        }
        const auto validation = driver.validateSyntax(entry.example.toStdString());
        EXPECT_NE(validation.status, ExpressionSyntaxValidation::Status::InvalidSyntax)
            << entry.syntax.toStdString() << ": " << validation.errorMessage;
    }
}
