#include "ToolLauncher.h"

#include <gtest/gtest.h>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>

namespace {
QString createExecutable(const QString& path, bool executable = true) {
    EXPECT_TRUE(QDir().mkpath(QFileInfo(path).absolutePath()));
    QFile file(path);
    EXPECT_TRUE(file.open(QIODevice::WriteOnly));
    EXPECT_EQ(file.write("#!/bin/sh\nexit 0\n"), 17);
    file.close();
    const auto basePermissions = QFileDevice::ReadOwner | QFileDevice::WriteOwner |
                                 QFileDevice::ReadUser | QFileDevice::WriteUser;
    EXPECT_TRUE(QFile::setPermissions(path, executable
        ? basePermissions | QFileDevice::ExeOwner | QFileDevice::ExeUser
        : basePermissions));
    return path;
}
}  // namespace

TEST(ToolLauncherPaths, DebianLikePrefixWithSpacesResolvesInstalledBindir) {
    QTemporaryDir temporary;
    ASSERT_TRUE(temporary.isValid());
    const QString prefix = temporary.path() + QStringLiteral("/prefix with spaces");
    const QString guiDir = prefix + QStringLiteral("/libexec/genesys/system/bin");
    const QString tool = prefix + QStringLiteral("/bin/genesys-optimizer-gui");
    ASSERT_TRUE(QDir().mkpath(guiDir));
    createExecutable(tool);

    const QStringList candidates = ToolLauncher::candidateExecutables(
        QStringLiteral("genesys-optimizer-gui"), guiDir);
    ASSERT_EQ(candidates.size(), 2);
    EXPECT_EQ(candidates.at(0), guiDir + QStringLiteral("/genesys-optimizer-gui"));
    EXPECT_EQ(candidates.at(1), tool);
    EXPECT_EQ(ToolLauncher::findExecutable(QStringLiteral("genesys-optimizer-gui"), guiDir), tool);
}

TEST(ToolLauncherPaths, PortableUsesColocatedExecutableFirst) {
    QTemporaryDir temporary;
    ASSERT_TRUE(temporary.isValid());
    const QString guiDir = temporary.path() + QStringLiteral("/libexec/genesys/system/bin");
    const QString tool = guiDir + QStringLiteral("/genesys-dataanalyser-gui");
    createExecutable(tool);
    EXPECT_EQ(ToolLauncher::findExecutable(QStringLiteral("genesys-dataanalyser-gui"), guiDir), tool);
}

TEST(ToolLauncherPaths, DevelopmentRelativePathsAreExplicitAndDeduplicated) {
    QTemporaryDir temporary;
    ASSERT_TRUE(temporary.isValid());
    const QString guiDir = temporary.path() + QStringLiteral("/build/gui-app/source/applications/gui/genesys");
    const QString tool = temporary.path() + QStringLiteral("/build/gui-app/source/applications/gui/optimizer/genesys-optimizer-gui");
    createExecutable(tool);
    const QStringList dirs{QStringLiteral("../optimizer"), QStringLiteral("../optimizer")};
    const QStringList candidates = ToolLauncher::candidateExecutables(
        QStringLiteral("genesys-optimizer-gui"), guiDir, dirs);
    EXPECT_EQ(candidates.size(), 2);
    EXPECT_EQ(ToolLauncher::findExecutable(QStringLiteral("genesys-optimizer-gui"), guiDir, dirs), tool);
}

TEST(ToolLauncherPaths, MissingOrNonExecutableToolDoesNotResolve) {
    QTemporaryDir temporary;
    ASSERT_TRUE(temporary.isValid());
    const QString guiDir = temporary.path() + QStringLiteral("/libexec/genesys/system/bin");
    ASSERT_TRUE(QDir().mkpath(guiDir));
    const QString name = QStringLiteral("genesys-ai-assistant-gui");
    EXPECT_TRUE(ToolLauncher::findExecutable(name, guiDir).isEmpty());
    createExecutable(guiDir + QStringLiteral("/genesys-ai-assistant-gui"), false);
    EXPECT_TRUE(ToolLauncher::findExecutable(name, guiDir).isEmpty());
}

TEST(ToolLauncherPaths, InstalledRuntimeDoesNotSearchDevelopmentFallback) {
    QTemporaryDir temporary;
    ASSERT_TRUE(temporary.isValid());
    const QString prefix = temporary.path() + QStringLiteral("/prefix with spaces");
    const QString guiDir = prefix + QStringLiteral("/libexec/genesys/system/bin");
    const QString outside = prefix + QStringLiteral("/libexec/genesys/system/optimizer/genesys-optimizer-gui");
    ASSERT_TRUE(QDir().mkpath(guiDir));
    createExecutable(outside);

    const QStringList candidates = ToolLauncher::candidateExecutables(
        QStringLiteral("genesys-optimizer-gui"), guiDir, {QStringLiteral("../optimizer")});
    ASSERT_EQ(candidates.size(), 2);
    EXPECT_EQ(candidates[0], guiDir + QStringLiteral("/genesys-optimizer-gui"));
    EXPECT_EQ(candidates[1], prefix + QStringLiteral("/bin/genesys-optimizer-gui"));
    EXPECT_TRUE(ToolLauncher::findExecutable(
        QStringLiteral("genesys-optimizer-gui"), guiDir, {QStringLiteral("../optimizer")}).isEmpty());
}

TEST(ToolLauncherPaths, RejectsArbitraryProgramsAndPathTraversal) {
    QTemporaryDir temporary;
    ASSERT_TRUE(temporary.isValid());
    for (const QString& name : {QStringLiteral("sh"),
                                QStringLiteral("../genesys-optimizer-gui"),
                                QStringLiteral("/bin/sh"),
                                QStringLiteral("genesys-optimizer-gui;sh"),
                                QStringLiteral("genesys-optimizer-gui/../../sh")}) {
        EXPECT_TRUE(ToolLauncher::candidateExecutables(name, temporary.path()).isEmpty());
        EXPECT_TRUE(ToolLauncher::findExecutable(name, temporary.path()).isEmpty());
    }
    EXPECT_TRUE(ToolLauncher::candidateExecutables(QStringLiteral("genesys-optimizer-gui"),
                                                    temporary.path(),
                                                    {QStringLiteral("/usr/bin")}).size() == 1);
}
