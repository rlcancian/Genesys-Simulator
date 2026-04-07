#include "mainwindow.h"
#include "ui_mainwindow.h"

// Kernel
#include "../../../../kernel/simulator/SinkModelComponent.h"

#include <string>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cctype>
//#include <streambuf>

// QT
#include <QMessageBox>
#include <QTextStream>
#include <QFileDialog>
#include <QGraphicsScene>
#include <QDateTime>
#include <QEventLoop>
#include <QTemporaryFile>
#include <Qt>
#include <QGraphicsPixmapItem>
#include <QPropertyAnimation>
// #include <qt5/QtWidgets/qgraphicsitem.h>
#include <QtWidgets/qgraphicsitem.h>
#include <QGraphicsScene>
//#include <QDesktopWidget> //removed from qt6
#include <QScreen>
#include <QDebug>
#include <QRegularExpression>
#include <QRandomGenerator>

static std::string _escapeDotLabel(const std::string& text) {
    std::string escaped;
    escaped.reserve(text.size());
    for (char c : text) {
        switch (c) {
            case '\\':
                escaped += "\\\\";
                break;
            case '"':
                escaped += "\\\"";
                break;
            case '\n':
            case '\r':
                escaped += " ";
                break;
            default:
                escaped += c;
        }
    }
    return escaped;
}


void MainWindow::_actualizeModelSimLanguage() {
    Model* m = simulator->getModelManager()->current();
    if (m != nullptr) {
        m->getPersistence()->setOption(ModelPersistence_if::Options::SAVEDEFAULTS, true);
        std::string tempFilename = "./temp.tmp";
        m->getPersistence()->setOption(ModelPersistence_if::Options::SAVEDEFAULTS, false);
        bool res = m->save(tempFilename);
        std::string line;
        std::ifstream file(tempFilename);
        if (file.is_open()) {
            ui->TextCodeEditor->clear();
            while (std::getline(file, line)) {
                ui->TextCodeEditor->appendPlainText(QString::fromStdString(line));
            }
            file.close();
            _textModelHasChanged = false;
        }
    }
}

void MainWindow::_clearModelEditors() {
    ui->TextCodeEditor->clear();
    ui->textEdit_Simulation->clear();
    ui->textEdit_Reports->clear();
    ui->graphicsView->clear();
    ui->plainTextEditCppCode->clear();
    ui->treeWidgetComponents->clear();
    ui->treeWidgetDataDefnitions->clear();
}

bool MainWindow::_setSimulationModelBasedOnText() {
    Model* model = simulator->getModelManager()->current();
    if (this->_textModelHasChanged) {
        //@TODO !!!!!!!!!!!!!!
        // simulator->getModels()->remove(model);
        // model = nullptr;
    }
    if (model == nullptr) { // only model text written in UI
        QString modelLanguage = ui->TextCodeEditor->toPlainText();
        if (!simulator->getModelManager()->createFromLanguage(modelLanguage.toStdString())) {
            QMessageBox::critical(this, "Check Model", "Error in the model text. See console for more information.");
        }
        model = simulator->getModelManager()->current();
        if (model != nullptr) {

            _setOnEventHandlers();
        }
    }
    return simulator->getModelManager()->current() != nullptr;
}

std::string MainWindow::_adjustDotName(std::string name) {
    std::string text = Util::StrReplace(name, "[", "_");
    text = Util::StrReplace(text, "]", "");
    text = Util::StrReplace(text, ".", "_");
    for (char& c : text) {
        if (!(std::isalnum(static_cast<unsigned char>(c)) || c == '_')) {
            c = '_';
        }
    }
    if (!text.empty() && std::isdigit(static_cast<unsigned char>(text.front()))) {
        text = "_" + text;
    }
    return text;
}

void MainWindow::_insertTextInDot(std::string text, unsigned int compLevel, unsigned int compRank, std::map<unsigned int, std::map<unsigned int, std::list<std::string>*>*>* dotmap, bool isNode) {
    if (dotmap->find(compLevel) == dotmap->end()) {
        dotmap->insert({compLevel, new std::map<unsigned int, std::list<std::string>*>()});
    }
    std::pair<unsigned int, std::map<unsigned int, std::list<std::string>*>*> dotPair = (*dotmap->find(compLevel));
    if (dotPair.second->find(compRank) == dotPair.second->end()) {
        dotPair.second->insert({compRank, new std::list<std::string>()});
    }
    std::pair<unsigned int, std::list<std::string>*> dotPair2 = *(dotPair.second->find(compRank));
    if (isNode) {
        dotPair2.second->insert(dotPair2.second->begin(), text);
    } else {

        dotPair2.second->insert(dotPair2.second->end(), text);
    }
}

void MainWindow::_recursiveCreateModelGraphicPicture(ModelDataDefinition* componentOrData, std::list<ModelDataDefinition*>* visited, std::map<unsigned int, std::map<unsigned int, std::list<std::string>*>*>* dotmap) {
    const struct DOT_STYLES {
        //std::string nodeComponent = "shape=Mrecord, fontsize=11, fontname=\"Helvetica\", fontcolor=\"#1f2937\", color=\"#334155\", penwidth=1.2, style=\"rounded,filled\", fillcolor=\"#f8fafc\"";
        std::string nodeComponent = "shape=Mrecord, fontsize=11, fontname=\"Helvetica\", fontcolor=\"#1f2937\", color=\"#334155\", penwidth=1.2, style=\"rounded,filled\", fillcolor=\"bisque\"";
        //std::string nodeComponentOtherLevel = "shape=record, fontsize=12, fontcolor=black, style=filled, fillcolor=goldenrod3";
        std::string edgeComponent = "style=solid, arrowhead=\"vee\", color=\"#334155\", fontcolor=\"#334155\", fontsize=8, penwidth=1.1";
        std::string nodeDataDefInternal = "shape=Mrecord, fontsize=9, fontname=\"Helvetica\", color=\"#64748b\", fontcolor=\"#334155\", style=\"rounded,filled\", fillcolor=\"#e2e8f0\"";
        std::string nodeDataDefAttached = "shape=Mrecord, fontsize=9, fontname=\"Helvetica\", color=\"#475569\", fontcolor=\"#1e293b\", style=\"rounded,filled\", fillcolor=\"#dcfce7\"";
        std::string edgeDataDefInternal = "style=dashed, arrowhead=\"diamond\", color=\"#64748b\", fontcolor=\"#64748b\", fontsize=7";
        std::string edgeDataDefAttached = "style=dashed, arrowhead=\"odiamond\", color=\"#475569\", fontcolor=\"#475569\", fontsize=7";
        unsigned int rankSource = 0;
        unsigned int rankSink = 1;
        unsigned int rankComponent = 99;
        unsigned int rankComponentOtherLevel = 99;
        unsigned int rankDataDefInternal = 99;
        unsigned int rankDataDefAttached = 99;
        unsigned int rankDataDefRecursive = 99;
        unsigned int rankEdge = 99;
    } DOT;


    visited->insert(visited->end(), componentOrData);
    std::string text;
    unsigned int modellevel = simulator->getModelManager()->current()->getLevel();
    std::list<ModelDataDefinition*>::iterator visitedIt;
    ModelComponent* parentComponentSuperLevel = nullptr;
    unsigned int level = componentOrData->getLevel();
    if (dynamic_cast<ModelComponent*> (componentOrData) != nullptr) {
        if (level != modellevel && !ui->checkBox_ShowLevels->isChecked()) {
            // do not show the component itself, but its parent on the model level
            parentComponentSuperLevel = simulator->getModelManager()->current()->getComponentManager()->find(level);
            assert(parentComponentSuperLevel != nullptr);
            visitedIt = std::find(visited->begin(), visited->end(), parentComponentSuperLevel);
            if (visitedIt == visited->end()) {
                text = "  " + _adjustDotName(parentComponentSuperLevel->getName()) + " [" + DOT.nodeComponent + ", label=\"" + _escapeDotLabel(parentComponentSuperLevel->getClassname()) + "|" + _escapeDotLabel(parentComponentSuperLevel->getName()) + "\"]" + ";\n";
                _insertTextInDot(text, level, DOT.rankComponentOtherLevel, dotmap, true);
            }
        } else {
            text = "  " + _adjustDotName(componentOrData->getName()) + " [" + DOT.nodeComponent + ", label=\"" + _escapeDotLabel(componentOrData->getClassname()) + "|" + _escapeDotLabel(componentOrData->getName()) + "\"]" + ";\n";
            if (dynamic_cast<SourceModelComponent*> (componentOrData) != nullptr) {
                _insertTextInDot(text, level, DOT.rankSource, dotmap, true);
            } else if (dynamic_cast<SinkModelComponent*> (componentOrData) != nullptr) {
                _insertTextInDot(text, level, DOT.rankSink, dotmap, true);
            } else {
                _insertTextInDot(text, level, DOT.rankComponent, dotmap, true);
            }
        }
    }
    //
    ModelDataDefinition* data;
    std::string dataname, componentName;
    if (parentComponentSuperLevel != nullptr) {
        componentName = parentComponentSuperLevel->getName();
    } else {
        componentName = componentOrData->getName();
    }
    if (ui->checkBox_ShowInternals->isChecked()) {
        for (std::pair<std::string, ModelDataDefinition*> dataPair : *componentOrData->getInternalData()) {
            dataname = _adjustDotName(dataPair.second->getName());
            level = dataPair.second->getLevel();
            visitedIt = std::find(visited->begin(), visited->end(), dataPair.second);
            if (visitedIt == visited->end()) {
                if (dynamic_cast<ModelComponent*> (dataPair.second) == nullptr) {
                    text = "  " + dataname + " [" + DOT.nodeDataDefInternal + ", label=\"" + _escapeDotLabel(dataPair.second->getClassname()) + "|" + _escapeDotLabel(dataPair.second->getName()) + "\"]" + ";\n";
                    _insertTextInDot(text, level, DOT.rankDataDefInternal, dotmap, true);
                    if (ui->checkBox_ShowRecursive->isChecked()) {
                        _recursiveCreateModelGraphicPicture(dataPair.second, visited, dotmap);
                    }
                }
            }
            if (dataPair.second->getLevel() == modellevel || ui->checkBox_ShowLevels->isChecked()) {
                text = "    " + dataname + "->" + _adjustDotName(componentName) + " [" + DOT.edgeDataDefInternal + ", label=\"" + _escapeDotLabel(dataPair.first) + "\"];\n";
                _insertTextInDot(text, modellevel, DOT.rankEdge, dotmap);
            }
        }
    }
    if (ui->checkBox_ShowElements->isChecked()) {
        for (std::pair<std::string, ModelDataDefinition*> dataPair : *componentOrData->getAttachedData()) {
            dataname = _adjustDotName(dataPair.second->getName());
            level = dataPair.second->getLevel();
            visitedIt = std::find(visited->begin(), visited->end(), dataPair.second);
            if (visitedIt == visited->end()) {
                if (dynamic_cast<ModelComponent*> (dataPair.second) == nullptr) {
                    text = "  " + dataname + " [" + DOT.nodeDataDefAttached + ", label=\"" + _escapeDotLabel(dataPair.second->getClassname()) + "|" + _escapeDotLabel(dataPair.second->getName()) + "\"]" + ";\n";
                    _insertTextInDot(text, level, DOT.rankDataDefAttached, dotmap, true);
                }
                if (ui->checkBox_ShowRecursive->isChecked()) {
                    _recursiveCreateModelGraphicPicture(dataPair.second, visited, dotmap);
                }
            }
            text = "    " + dataname + "->" + _adjustDotName(componentName) + " [" + DOT.edgeDataDefAttached + ", label=\"" + _escapeDotLabel(dataPair.first) + "\"];\n";
            _insertTextInDot(text, modellevel, DOT.rankEdge, dotmap);
        }
    }
    ModelComponent* component = dynamic_cast<ModelComponent*> (componentOrData);
    if (component != nullptr) {
        level = component->getLevel();
        Connection* connection;
        for (unsigned short i = 0; i < component->getConnectionManager()->size(); i++) {
            connection = component->getConnectionManager()->getConnectionAtPort(i);
            visitedIt = std::find(visited->begin(), visited->end(), connection->component);
            if (visitedIt == visited->end()) {
                _recursiveCreateModelGraphicPicture(connection->component, visited, dotmap);
            }
            if (connection->component->getLevel() == modellevel || ui->checkBox_ShowLevels->isChecked()) {

                text = "    " + _adjustDotName(componentName) + "->" + _adjustDotName(connection->component->getName()) + "[" + DOT.edgeComponent + "];\n";
                _insertTextInDot(text, modellevel, DOT.rankEdge, dotmap);
            }
        }
    }
}

std::string MainWindow::_addCppCodeLine(std::string line, unsigned int indent) {
    std::string text = "";
    for (unsigned int i = 0; i < indent; i++) {
        text += "\t";
    }
    text += line + "\n";
    return text;
}

void MainWindow::_actualizeModelCppCode() {
    Model* m = simulator->getModelManager()->current();
    if (m != nullptr) {
        unsigned short tabs = 0;
        std::string text, text2, name;
        std::map<std::string, std::string>* code = new std::map<std::string, std::string>();
        text = _addCppCodeLine("/*");
        text += _addCppCodeLine(" * This C++ source code was automatically generated by GenESyS");
        text += _addCppCodeLine(" */");
        code->insert({"1begin", text});

        text = _addCppCodeLine("#include \"kernel/simulator/Simulator.h\"");
        text = _addCppCodeLine("#include \"kernel/simulator/PropertyGenesys.h\"");
        List<std::string>* included = new List<std::string>();
        for (ModelComponent* comp : *m->getComponentManager()->getAllComponents()) {
            name = comp->getClassname();
            if (included->find(name) == included->list()->end()) {
                included->insert(name);
                text += _addCppCodeLine("#include \"plugins/components/" + name + ".h\"");
            }
        }
        for (std::string ddClassname : *m->getDataManager()->getDataDefinitionClassnames()) {
            text += _addCppCodeLine("#include \"plugins/data/" + ddClassname + ".h\"");
            for (ModelDataDefinition* modeldata : *m->getDataManager()->getDataDefinitionList(ddClassname)->list()) {
                name = modeldata->getName();
                if (name.find(".") == std::string::npos) {
                    if (included->find(name) == included->list()->end()) {
                        included->insert(ddClassname);
                        text += _addCppCodeLine("#include \"plugins/data/" + ddClassname + "\"");
                    }
                }
            }
        }
        code->insert({"2include", text});

        text = _addCppCodeLine("\nint main(int argc, char** argv) {");
        tabs++;
        text += _addCppCodeLine("// Create simulator, a property editor, a model and get acess to plugins", tabs);
        text += _addCppCodeLine("Simulator* genesys = new Simulator();", tabs);
        text += _addCppCodeLine("PropertyEditorGenesys* propertyEditor = new PropertyEditorGenesys();", tabs);
        text += _addCppCodeLine("Model* model = genesys->getModels()->newModel();", tabs);
        text += _addCppCodeLine("PluginManager* plugins = genesys->getPlugins();", tabs);
        text += _addCppCodeLine("model->getTracer()->setTraceLevel(TraceManager::TraceLevel::L9_mostDetailed);", tabs);
        code->insert({"3main", text});

        text = _addCppCodeLine("// Create model components and setting properties", tabs);
        for (std::string ddClassname : *m->getDataManager()->getDataDefinitionClassnames()) {
            for (ModelDataDefinition* modeldata : *m->getDataManager()->getDataDefinitionList(ddClassname)->list()) {
                name = modeldata->getName();
                if (name.find(".") == std::string::npos) {
                    text += _addCppCodeLine(ddClassname + "* " + name + " = plugins->newInstance<" + ddClassname + ">(model, \"" + name + "\");", tabs);
                }
                for (auto prop : *modeldata->getProperties()->list()) {
                    // Fazer um loop até encontrar propriedade não alterável?
                    text += _addCppCodeLine("SimulationControl* property = propertyEditor->findProperty(" + std::to_string(modeldata->getId()) + ", " + prop->getName() + ");", tabs);
                    text += _addCppCodeLine("propertyEditor->changeProperty(property, " + prop->getValue() + ", false);", tabs);
                };
                text += _addCppCodeLine("", tabs);
            }
        }
        code->insert({"4datadef", text});

        text = _addCppCodeLine("// Create model components", tabs);
        for (ModelComponent* comp : *m->getComponentManager()->getAllComponents()) {
            name = comp->getName();
            if (name.find(".") == std::string::npos) {
                text += _addCppCodeLine(comp->getClassname() + "* " + name + " = plugins->newInstance<" + comp->getClassname() + ">(model, \"" + name + "\");", tabs);
            }
        }
        code->insert({"5modelcompdef", text});

        text = _addCppCodeLine("// Connect the components in the model", tabs);
        Connection* conn;
        for (ModelComponent* comp : *m->getComponentManager()->getAllComponents()) {
            name = comp->getName();
            if (name.find(".") == std::string::npos) {
                for (std::pair<unsigned int, Connection*> pair : *comp->getConnectionManager()->connections()) {//unsigned int i=0; i<comp->getConnections()->size(); i++) {
                    conn = pair.second; //comp->getConnections()->getConnectionAtPort(i);
                    text2 = conn->component->getName(); // + conn->second==0?"":","+std::to_string(conn->second);
                    text += _addCppCodeLine(name + "->getConnections()->insertAtPort(" + std::to_string(pair.first) + "," + text2 + ");", tabs);
                }
            }
        }
        code->insert({"6modelconnect", text});

        ModelSimulation* sim = m->getSimulation();
        text = _addCppCodeLine("// Define simulation options", tabs);
        text += _addCppCodeLine("ModelSimulation* sim = model->getSimulation();", tabs);
        text += _addCppCodeLine("sim->setReplicationLength(" + std::to_string(sim->getReplicationLength()) + ")", tabs);
        text += _addCppCodeLine("sim->setReplicationLengthTimeUnit(Util::TimeUnit::" + Util::StrTimeUnitLong(sim->getReplicationLengthTimeUnit()) + ");", tabs);
        text += _addCppCodeLine("sim->setWarmUpPeriod(" + std::to_string(sim->getWarmUpPeriod()) + ");", tabs);
        text += _addCppCodeLine("sim->setWarmUpPeriodTimeUnit(Util::TimeUnit::" + Util::StrTimeUnitLong(sim->getWarmUpPeriodTimeUnit()) + ");", tabs);
        text += _addCppCodeLine("sim->setReplicationReportBaseTimeUnit(Util::TimeUnit::" + Util::StrTimeUnitLong(sim->getReplicationBaseTimeUnit()) + ");", tabs);
        text2 = sim->isShowReportsAfterSimulation()?"true":"false";
        text += _addCppCodeLine("sim->setsetShowReportsAfterSimulation("+text2+");", tabs);
        code->insert({"7simulation", text});

        text = _addCppCodeLine("// simulate and show report", tabs);
        text += _addCppCodeLine("sim->start();", tabs);
        text += _addCppCodeLine("return 0;", tabs);
        tabs--;
        text += _addCppCodeLine("}", tabs);
        code->insert({"8end", text});

        // Show
        ui->plainTextEditCppCode->clear();
        for (std::pair<std::string, std::string> codeSection : *code) {
            //ui->plainTextEditCppCode->appendPlainText(QString::fromStdString("// " + codeSection.first+"\n"));
            ui->plainTextEditCppCode->appendPlainText(QString::fromStdString(codeSection.second));
        }
    } else {

    }
}

bool MainWindow::graphicalModelHasChanged() const {
    return _graphicalModelHasChanged;
}

void MainWindow::setGraphicalModelHasChanged(bool graphicalModelHasChanged) {
    _graphicalModelHasChanged = graphicalModelHasChanged;
    _actualizeTabPanes();
}

bool MainWindow::_createModelImage() {
    bool res = this->_setSimulationModelBasedOnText();
    if (!res || simulator->getModelManager()->current() == nullptr) {
        return false;
    }
    std::string dot = "digraph G {\n";
    dot += "  compound=true; rankdir=LR; \n";
    std::map<unsigned int, std::map<unsigned int, std::list<std::string>*>*>* dotmap = new std::map<unsigned int, std::map<unsigned int, std::list<std::string>*>*>();

    std::list<SourceModelComponent*>* sources = simulator->getModelManager()->current()->getComponentManager()->getSourceComponents();
    std::list<ModelDataDefinition*>* visited = new std::list<ModelDataDefinition*>();
    std::list<ModelDataDefinition*>::iterator visitedIt;
    for (SourceModelComponent* source : *sources) {
        visitedIt = std::find(visited->begin(), visited->end(), source);
        if (visitedIt == visited->end()) {
            _recursiveCreateModelGraphicPicture(source, visited, dotmap);
        }
    }
    std::list<ModelComponent*>* transfers = simulator->getModelManager()->current()->getComponentManager()->getTransferInComponents();
    for (ModelComponent* transfer : *transfers) {
        visitedIt = std::find(visited->begin(), visited->end(), transfer);
        if (visitedIt == visited->end()) {
            _recursiveCreateModelGraphicPicture(transfer, visited, dotmap);
        }
    }
    std::list<ModelComponent*>* allComps = simulator->getModelManager()->current()->getComponentManager()->getAllComponents();
    for (ModelComponent* comp : *allComps) {
        visitedIt = std::find(visited->begin(), visited->end(), comp);
        if (visitedIt == visited->end()) {
            _recursiveCreateModelGraphicPicture(comp, visited, dotmap);
        }
    }
    // combine all level subgraphs
    unsigned int modelLevel = simulator->getModelManager()->current()->getLevel();
    for (std::pair<unsigned int, std::map<unsigned int, std::list<std::string>*>*> dotpair : *dotmap) {
        if (dotpair.first == modelLevel) {
            dot += "\n  // model level\n";
            for (std::pair<unsigned int, std::list<std::string>*> dotpair2 : *dotpair.second) {
                dot += "  {\n";
                if (dotpair2.first == 0)
                    dot += "     rank=min  // " + std::to_string(dotpair2.first) + "\n";
                else if (dotpair2.first == 1)
                    dot += "     rank=max  // " + std::to_string(dotpair2.first) + "\n";
                else if (dotpair2.first < 10)
                    dot += "     rank=same  // " + std::to_string(dotpair2.first) + "\n";
                for (std::string str : *dotpair2.second) {
                    dot += "   " + str;
                }
                dot += "  }\n";
            }
        } else if (ui->checkBox_ShowLevels->isChecked()) {
            dot += "\n\n // submodel level  " + std::to_string(dotpair.first) + "\n";
            dot += " subgraph cluster_level_" + std::to_string(dotpair.first) + " {\n";
            dot += "   graph[style=filled; fillcolor=mistyrose2] label=\"" + simulator->getModelManager()->current()->getComponentManager()->find(dotpair.first)->getName() + "\";\n";
            for (std::pair<unsigned int, std::list<std::string>*> dotpair2 : *dotpair.second) {
                dot += "  {\n";
                if (dotpair2.first == 0)
                    dot += "     rank=min  // " + std::to_string(dotpair2.first) + "\n";
                else if (dotpair2.first == 1)
                    dot += "     rank=max  // " + std::to_string(dotpair2.first) + "\n";
                else if (dotpair2.first < 10)
                    dot += "     rank=same  // " + std::to_string(dotpair2.first) + "\n";
                for (std::string str : *dotpair2.second) {
                    dot += "   " + str;
                }
                dot += "  }\n";
            }
            dot += "\n }\n";
        }
    }
    dot += "}\n";
    visited->clear();
    std::string basefilename = "./.tempFixedGraphicalModelRepresentation";
    std::string dotfilename = basefilename + ".dot";
    std::string pngfilename = basefilename + ".png";
    try {
        std::ofstream savefile;
        savefile.open(dotfilename, std::ofstream::out);
        QString data = QString::fromStdString(dot);
        QStringList strList = data.split(QRegularExpression("[\n]"), Qt::SkipEmptyParts); //data.split(QRegExp("[\n]"), QString::SkipEmptyParts);
        for (unsigned int i = 0; i < strList.size(); i++) {
            savefile << strList.at(i).toStdString() << std::endl;
        }
        savefile.close();
        try {
            std::remove(pngfilename.c_str());
        } catch (...) {

        }
        try {
            std::string command = "dot -Tpng " + dotfilename + " -o " + pngfilename;
            system(command.c_str());
            QPixmap pm(QString::fromStdString(pngfilename)); // <- path to image file
            //int w = ui->label_ModelGraphic->width();
            //int h = ui->label_ModelGraphic->height();
            ui->label_ModelGraphic->setPixmap(pm); //.scaled(w, h, Qt::IgnoreAspectRatio));
            ui->label_ModelGraphic->setScaledContents(false);
            try {
                //std::remove(dotfilename.c_str());
                //std::remove(pngfilename.c_str());
            } catch (...) {

            }
            return true;
        } catch (...) {
        }
    } catch (...) {
    }

    return false;
}

//-----------------------------------------------------------------

bool MainWindow::_saveTextModel(QFile *saveFile, QString data)
{
    QTextStream out(saveFile);

    try
    {
        static const QRegularExpression regex("[\n]");
        QStringList strList = data.split(regex);
        for (const QString &line : strList)
        {
            out << line << Qt::endl;
        }
        return true;
    }
    catch (const std::exception &e)
    {
        return false;
    }
}

bool MainWindow::_saveGraphicalModel(QString filename)
{
    QFile saveFile(filename);

    try
    {
        if (!saveFile.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QMessageBox::information(this, tr("Unable to access file to save"),
                                     saveFile.errorString());
            return false;
        }

        _saveTextModel(&saveFile, ui->TextCodeEditor->toPlainText());

        QTextStream out(&saveFile);
        out << "#Genegys Graphic Model" << Qt::endl;
        QString line = "0\tView\t";
        line += "zoom=" + QString::number(ui->horizontalSlider_ZoomGraphical->value());
        line += ", grid=" + QString::number(ui->actionShowGrid->isChecked()) + ", rule=0, snap="+ QString::number(ui->actionShowGrid->isChecked()) + ", viewpoint=(0,0)";
        out << line << Qt::endl;

        ModelGraphicsScene* scene = (ModelGraphicsScene*) (ui->graphicsView->getScene());

        if (scene)
        {
            for (QGraphicsItem *item : *ui->graphicsView->getScene()->getGraphicalModelComponents())
            {
                GraphicalModelComponent *gmc = (GraphicalModelComponent *)item;
                if (gmc)
                {
                    line = QString::fromStdString(std::to_string(gmc->getComponent()->getId()) + "\t" + gmc->getComponent()->getClassname() + "\t" + gmc->getComponent()->getName() + "\t" + "color=" + gmc->getColor().name().toStdString() + "\t" + "position=(" + std::to_string(gmc->scenePos().x()) + "," + std::to_string(gmc->scenePos().y() + gmc->getHeight()/2) + ")");
                    out << line << Qt::endl;
                }
            }

            QList<AnimationCounter *> *counters = myScene()->getAnimationsCounter();

            if (counters) {
                if (!counters->empty()) {
                    out << Qt::endl;
                    out << "#Counters" << Qt::endl;
                    int id = 0;
                    for (AnimationCounter *counter : *counters) {
                        int idCounter = -1;
                        if (counter->getCounter() != nullptr) {
                            idCounter = counter->getCounter()->getId();
                        }
                        line = QString("Counter_%1 \t id=%2 \t position=(%3,%4) \t width=%5 \t height=%6")
                                   .arg(id)
                                   .arg(idCounter)
                                   .arg(counter->scenePos().x(), 0, 'f', 2)
                                   .arg(counter->scenePos().y(), 0, 'f', 2)
                                   .arg(counter->boundingRect().width(), 0, 'f', 2)
                                   .arg(counter->boundingRect().height(), 0, 'f', 2);

                        out << line << Qt::endl;
                        id++;
                    }
                }
            }

            QList<AnimationVariable *> *variables = myScene()->getAnimationsVariable();

            if (variables) {
                if (!variables->empty()) {
                    out << Qt::endl;
                    out << "#Variables" << Qt::endl;
                    int id = 0;
                    for (AnimationVariable *variable : *variables) {
                        int idVariable = -1;
                        if (variable->getVariable() != nullptr) {
                            idVariable = variable->getVariable()->getId();
                        }
                        line = QString("Variable_%1 \t id=%2 \t position=(%3,%4) \t width=%5 \t height=%6")
                                   .arg(id)
                                   .arg(idVariable)
                                   .arg(variable->scenePos().x(), 0, 'f', 2)
                                   .arg(variable->scenePos().y(), 0, 'f', 2)
                                   .arg(variable->boundingRect().width(), 0, 'f', 2)
                                   .arg(variable->boundingRect().height(), 0, 'f', 2);

                        out << line << Qt::endl;
                        id++;
                    }
                }
            }

            QList<AnimationTimer *> *timers = myScene()->getAnimationsTimer();

            if (timers) {
                if (!timers->empty()) {
                    out << Qt::endl;
                    out << "#Timers" << Qt::endl;
                    int id = 0;
                    for (AnimationTimer *timer : *timers) {
                        line = QString("Timer_%1 \t hour=%2 \t minute=%3 \t second=%4 \t format=%5 \t position=(%6,%7) \t width=%8 \t height=%9")
                                   .arg(id)
                                   .arg(timer->getInitialHours())
                                   .arg(timer->getInitialMinutes())
                                   .arg(timer->getInitialSeconds())
                                   .arg(static_cast<unsigned int>(timer->getTimeFormat()))
                                   .arg(timer->scenePos().x(), 0, 'f', 2)
                                   .arg(timer->scenePos().y(), 0, 'f', 2)
                                   .arg(timer->boundingRect().width(), 0, 'f', 2)
                                   .arg(timer->boundingRect().height(), 0, 'f', 2);

                        out << line << Qt::endl;
                        id++;
                    }
                }
            }

            /*/Lines
            out << "LINE" << Qt::endl;
            for (QGraphicsItem *item : *ui->graphicsView->getScene()->getGraphicalModelComponents())
            {
                QGraphicsLineItem *drawLine = dynamic_cast<QGraphicsLineItem *> (item);
                if (drawLine)
                {
                    line = QString::fromStdString(std::to_string(drawLine->))
                }
            }*/
        }

        saveFile.close();
        return true;
    }
    catch (const std::exception &e)
    {
        return false;
    }

    //QString data = QString::fromStdString(dot);
    //QStringList strList = data.split(QRegExp("[\n]"), QString::SkipEmptyParts);
    //for (unsigned int i = 0; i < strList.size(); i++) {
    //	savefile << strList.at(i).toStdString() << std::endl;
    //}
}

Model *MainWindow::_loadGraphicalModel(std::string filename) {
    QFile file(QString::fromStdString(filename));

    Model *model = nullptr;

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::information(this, tr("Unable to access file to save"),
                                 file.errorString());
        return nullptr;
    }

    QFileInfo fileInfo(file.fileName());
    QString extension = fileInfo.suffix();

    if (extension != "gui") {
        model = simulator->getModelManager()->loadModel(file.fileName().toStdString());
        if (model != nullptr) _generateGraphicalModelFromModel();
        return model;
    }

    QString content = file.readAll();
    file.close();

    QStringList lines = content.split("\n");

    QStringList simulLang;
    QStringList gui;
    QStringList counters;
    QStringList variables;
    QStringList timers;

    bool guiFlag = false;
    bool counterFlag = false;
    bool variableFlag = false;
    bool timerFlag = false;

    for (const QString &line : lines) {
        if (line.startsWith("#Genegys Graphic Model")) {
            guiFlag = true;

            counterFlag = false;
            variableFlag = false;
            timerFlag = false;
            continue;
        }

        if (line.startsWith("#Counters")) {
            counterFlag = true;

            guiFlag = false;
            variableFlag = false;
            timerFlag = false;
            continue;
        }

        if (line.startsWith("#Variables")) {
            variableFlag = true;

            guiFlag = false;
            counterFlag = false;
            timerFlag = false;
            continue;
        }

        if (line.startsWith("#Timers")) {
            timerFlag = true;

            guiFlag = false;
            counterFlag = false;
            variableFlag = false;
            continue;
        }

        if (!guiFlag && !timerFlag && !counterFlag && !variableFlag) {
            simulLang.append(line);
        } else {
            if (counterFlag) {
                counters.append(line);
            } else if (variableFlag) {
                variables.append(line);
            } else if (timerFlag) {
                timers.append(line);
            } else {
                gui.append(line);
            }
        }
    }

    file.close();

    QDateTime currentDateTime = QDateTime::currentDateTime();
    QString newFilename = QString("/tempFile-%1.gen").arg(currentDateTime.toString("yyyy-MM-dd-hh-mm-ss"));

    QString filePath = QDir::tempPath() + newFilename;
    QFile tempFile(filePath);
    tempFile.open(QIODevice::ReadWrite | QIODevice::Text);

    QTextStream outStream(&tempFile);
    for (const QString& line : simulLang) {
        outStream << line << Qt::endl;
    }

    outStream.flush();

    std::string nameTempFile = tempFile.fileName().toStdString();
    model = simulator->getModelManager()->loadModel(nameTempFile);

    tempFile.close();

    QFile::remove(tempFile.fileName());

    if (model != nullptr) {
        std::list<ModelComponent*> c = * model->getComponentManager()->getAllComponents();

        _clearModelEditors();

        bool firstLine = true;

        for (const QString& line : gui) {
            if (line.trimmed().isEmpty()) {
                continue;
            }

            if (firstLine) {
                QRegularExpression regex("(\\d+)\\s*View\\s*zoom=(\\d+),\\s*grid=(\\d+),\\s*rule=(\\d+),\\s*snap=(\\d+),\\s*viewpoint=\\(([^,]+),([^\\)]+)\\)");
                QRegularExpressionMatch match = regex.match(line);

                if (match.hasMatch()) {
                    int index = match.captured(1).toInt();
                    int zoom = match.captured(2).toInt();
                    int grid = match.captured(3).toInt();
                    int rule = match.captured(4).toInt();
                    int snap = match.captured(5).toInt();
                    qreal viewpointX = match.captured(6).toDouble();
                    qreal viewpointY = match.captured(7).toDouble();

                    if (grid) {
                        ui->actionShowGrid->setChecked(true);
                        myScene()->showGrid();
                    }

                    if (snap) {
                        ui->actionShowSnap->setChecked(true);
                        myScene()->setSnapToGrid(true);
                    }

                    ui->horizontalSlider_ZoomGraphical->setValue(zoom + TraitsGUI<GMainWindow>::zoomButtonChange);
                    double factor = ((double) zoom / 100.0)*(2 - 0.5) + 0.5;
                    double scaleFactor = 1.0;
                    scaleFactor *= factor;
                    //ui->label_ModelGraphic->resize(scaleFactor * ui->label_ModelGraphic->pixmap()->size());
                }
                firstLine = false;
                continue;
            }

            QStringList split = line.split("\t");
            if (split.size() < 5) {
                continue;
            }

            Util::identification id = split[0].toULong();

            // Component
            QString comp = split[1];

            // Color
            QString col = split[3];

            // Posição
            QString pos = split[4];

            // Expressao regular para pegar a cor
            QRegularExpression regexColor("color=#([0-9A-Fa-f]{6})");

            // Cria a expressao regular match
            QRegularExpressionMatch match = regexColor.match(col);

            QString hexColor;

            // Extrai a cor
            if (match.hasMatch()) {hexColor = match.captured(1);}

            // Cria a cor
            QColor color("#"+ hexColor);

            // Expressao regular para pegar a cor
            QRegularExpression regexPos("position=\\((-?\\d+\\.?\\d*),(-?\\d+\\.?\\d*),(-?\\d+\\.?\\d*),(-?\\d+\\.?\\d*)\\)");

            // Cria a expressao regular match
            match = regexPos.match(pos);

            QPoint position;

            // Extrai a posição
            if (match.hasMatch()) {
                // Extrai x e y
                qreal x = match.captured(1).toDouble();
                qreal y = match.captured(3).toDouble();

                // Seta x e y em pos
                position.setX(x);
                position.setY(y);
            }

            // Pega a cena para adicionar o componente nela
            ModelGraphicsScene *scene = (ModelGraphicsScene *)(ui->graphicsView->scene());

            // Pega o Plugin
            Plugin* plugin = simulator->getPluginManager()->find(comp.toStdString());
            if (plugin == nullptr) {
                ui->textEdit_Console->append(QString("Warning: Could not find plugin \"%1\" while loading GUI item id=%2. Item ignored.")
                                             .arg(comp).arg(id));
                continue;
            }

            // Cria o componente no modelo
            ModelComponent* component = simulator->getModelManager()->current()->getComponentManager()->find(id);

            if (!component) continue;
            // Desenha na tela
            scene->addGraphicalModelComponent(plugin, component, position, color);
        }

        QList<QGraphicsItem*> *graphicalComponents = ui->graphicsView->getScene()->getGraphicalModelComponents();

        for (unsigned int i = 0; i < (unsigned int) graphicalComponents->size(); i++) {
            GraphicalModelComponent *source = dynamic_cast<GraphicalModelComponent *> (graphicalComponents->at(i));
            if (source == nullptr || source->getComponent() == nullptr) {
                continue;
            }
            std::map<unsigned int, Connection*> *connections = source->getComponent()->getConnectionManager()->connections();
            if (connections == nullptr) {
                continue;
            }

            for (auto it = connections->begin(); it != connections->end(); ++it) {
                unsigned int portSource = it->first;
                Connection* connection = it->second;
                if (connection == nullptr || connection->component == nullptr) {
                    continue;
                }

                GraphicalModelComponent* destination = ui->graphicsView->getScene()->findGraphicalModelComponent(connection->component->getId());
                if (destination == nullptr || destination->getGraphicalInputPorts().empty()) {
                    continue;
                }
                unsigned int portDestination = destination->getGraphicalInputPorts().at(0)->portNum();
                if (portSource >= source->getGraphicalOutputPorts().size()) {
                    continue;
                }
                if (portDestination >= destination->getGraphicalInputPorts().size()) {
                    continue;
                }

                source->setOcupiedOutputPorts(source->getOcupiedOutputPorts() + 1);
                destination->setOcupiedInputPorts(destination->getOcupiedInputPorts() + 1);

                std::string nameSource = source->getComponent()->getName();
                GraphicalComponentPort* sourceport = source->getGraphicalOutputPorts().at(portSource);

                std::string nameDestination = destination->getComponent()->getName();
                GraphicalComponentPort* destport = destination->getGraphicalInputPorts().at(portDestination);

                ui->graphicsView->getScene()->addGraphicalConnection(sourceport, destport, portSource, portDestination);
            }
        }


        if (!counters.empty()) {
            QRegularExpression regex("Counter_(\\d+) \\t id=(-?\\d+) \\t position=\\(([^,]+),([^\\)]+)\\) \\t width=([^\\t]+) \\t height=([^\\t]+)");

            for (const QString& line : counters) {
                if (line.trimmed().isEmpty()) {
                    continue;
                }

                AnimationCounter *counter = new AnimationCounter();

                QRegularExpressionMatch match = regex.match(line);

                if (match.hasMatch()) {
                    int id = match.captured(2).toInt();
                    qreal posX = match.captured(3).toDouble();
                    qreal posY = match.captured(4).toDouble();
                    int width = match.captured(5).toDouble();
                    int height = match.captured(6).toDouble();

                    counter->setIdCounter(id);

                    QRectF newRect = QRectF(0, 0, width, height);
                    counter->setRect(newRect.normalized());
                    counter->setPos(QPointF(posX, posY));

                    myScene()->getAnimationsCounter()->append(counter);

                    myScene()->addItem(counter);
                } else {
                    delete counter;
                }
            }
        }

        if (!variables.empty()) {
            QRegularExpression regex("Variable_(\\d+) \\t id=(-?\\d+) \\t position=\\(([^,]+),([^\\)]+)\\) \\t width=([^\\t]+) \\t height=([^\\t]+)");

            for (const QString& line : variables) {
                if (line.trimmed().isEmpty()) {
                    continue;
                }

                AnimationVariable *variable = new AnimationVariable();

                QRegularExpressionMatch match = regex.match(line);

                if (match.hasMatch()) {
                    int id = match.captured(2).toInt();
                    qreal posX = match.captured(3).toDouble();
                    qreal posY = match.captured(4).toDouble();
                    int width = match.captured(5).toDouble();
                    int height = match.captured(6).toDouble();

                    variable->setIdVariable(id);

                    QRectF newRect = QRectF(0, 0, width, height);
                    variable->setRect(newRect.normalized());
                    variable->setPos(QPointF(posX, posY));

                    myScene()->getAnimationsVariable()->append(variable);

                    myScene()->addItem(variable);
                } else {
                    delete variable;
                }
            }
        }

        if (!timers.empty()) {
            QRegularExpression regex("Timer_(\\d+) \\s* hour=(\\d+) \\s* minute=(\\d+) \\s* second=(\\d+) \\s* format=(\\d+) \\s* position=\\(([^,]+),([^\\)]+)\\) \\s* width=([^\\t]+) \\s* height=([^\\t]+)");

            for (const QString& line : timers) {
                if (line.trimmed().isEmpty()) {
                    continue;
                }

                AnimationTimer *timer = new AnimationTimer(myScene());

                QRegularExpressionMatch match = regex.match(line);

                if (match.hasMatch()) {
                    int hour = match.captured(2).toInt();
                    int minute = match.captured(3).toInt();
                    int second = match.captured(4).toInt();
                    int format = match.captured(5).toInt();
                    qreal posX = match.captured(6).toDouble();
                    qreal posY = match.captured(7).toDouble();
                    int width = match.captured(8).toDouble();
                    int height = match.captured(9).toDouble();

                    timer->setInitialHours(hour);
                    timer->setInitialMinutes(minute);
                    timer->setInitialSeconds(second);
                    timer->setTimeFormat(Util::TimeFormat(format));

                    timer->setTime(0.0);

                    QRectF newRect = QRectF(0, 0, width, height);
                    timer->setRect(newRect.normalized());
                    timer->setPos(QPointF(posX, posY));

                    myScene()->getAnimationsTimer()->append(timer);

                    myScene()->addItem(timer);

                } else {
                    delete timer;
                }
            }
        }

        ui->textEdit_Console->append("\n");
        _modelfilename = QString::fromStdString(filename);
    }
    return model;
}


void MainWindow::_recursivalyGenerateGraphicalModelFromModel(ModelComponent* component, List<ModelComponent*>* visited, std::map<ModelComponent*,GraphicalModelComponent*>* map, int *x, int *y, int *ymax, int sequenceInLine) {
    PluginManager* pm = simulator->getPluginManager();
    GraphicalModelComponent *gmc;
    ModelGraphicsScene* scene = ui->graphicsView->getScene();
    Plugin* plugin = pm->find(component->getClassname());
    if (plugin == nullptr) {
        ui->textEdit_Console->append(QString::fromStdString(
            "Warning: Skipping component \"" + component->getName() + "\" (id="
            + std::to_string(component->getId()) + ") because plugin \""
            + component->getClassname() + "\" is unavailable."));
        return;
    }
    // get color from category
    QColor color = Qt::white;
    auto colorIt = _pluginCategoryColor->find(plugin->getPluginInfo()->getCategory());
    if (colorIt != _pluginCategoryColor->end()) {
        color = colorIt->second;
    }
    gmc = scene->addGraphicalModelComponent(plugin, component, QPoint(*x, *y), color);
    if (gmc == nullptr) {
        ui->textEdit_Console->append(QString::fromStdString(
            "Warning: Failed to draw component \"" + component->getName() + "\" (id="
            + std::to_string(component->getId()) + ")."));
        return;
    }
    map->insert({component,gmc});
    visited->insert(component);
    int yIni = *y;
    int xIni = *x;
    const int deltaY = TraitsGUI<GModelComponent>::width * TraitsGUI<GModelComponent>::heightProportion * 1.5;
    GraphicalComponentPort *sourceGraphicalPort, *destinyGraphicalPort;
    for(auto connectionMap: *component->getConnectionManager()->connections()) {
        if (connectionMap.second == nullptr || connectionMap.second->component == nullptr) {
            continue;
        }
        ModelComponent* nextComp = connectionMap.second->component;
        if (visited->find(nextComp)==visited->list()->end()) { // nextComponent was not visited yet
            if (++sequenceInLine==6) {
                *x -= 5 * TraitsGUI<GModelComponent>::width * 1.5;
                *y+= deltaY;
                sequenceInLine = 0;
            } else {
                *x += TraitsGUI<GModelComponent>::width * 1.5;
            }
            if (*y > *ymax)
                *ymax = *y;
            _recursivalyGenerateGraphicalModelFromModel(nextComp, visited, map, x, y, ymax, sequenceInLine);
            auto destinyIt = map->find(nextComp);
            if (destinyIt != map->end()) {
                GraphicalModelComponent *destinyGmc = destinyIt->second;
                if (connectionMap.first < gmc->getGraphicalOutputPorts().size()
                        && connectionMap.second->channel.portNumber < destinyGmc->getGraphicalInputPorts().size()) {
                    sourceGraphicalPort = gmc->getGraphicalOutputPorts().at(connectionMap.first);
                    destinyGraphicalPort = destinyGmc->getGraphicalInputPorts().at(connectionMap.second->channel.portNumber);
                    scene->addGraphicalConnection(sourceGraphicalPort, destinyGraphicalPort, connectionMap.first, connectionMap.second->channel.portNumber);
                }
            }
            *x = xIni;
            *y+= deltaY;
            sequenceInLine--;
        }
    }
    *y = yIni;
}

void MainWindow::_actualizeModelComponents(bool force) {
    Model* m = simulator->getModelManager()->current();
    ui->treeWidgetComponents->clear();
    if (m == nullptr) {
        return;
    }
    for (ModelComponent* comp : *m->getComponentManager()->getAllComponents()) {
        QList<QTreeWidgetItem *> items = ui->treeWidgetComponents->findItems(QString::fromStdString(std::to_string(comp->getId())), Qt::MatchExactly | Qt::MatchRecursive, 0);
        if (items.size() == 0) {
            QTreeWidgetItem* treeComp = new QTreeWidgetItem(ui->treeWidgetComponents);
            treeComp->setText(0, QString::fromStdString(std::to_string(comp->getId())));
            treeComp->setText(1, QString::fromStdString(comp->getClassname()));
            treeComp->setText(2, QString::fromStdString(comp->getName()));
            std::string properties = "";
            for (auto prop : *comp->getProperties()->list()) {
                properties += prop->getName() + ":" + prop->getValue() + ", ";
            }
            properties = properties.substr(0, properties.length() - 2);
            treeComp->setText(3, QString::fromStdString(properties));
        }
    }
    ui->treeWidgetComponents->resizeColumnToContents(0);
    ui->treeWidgetComponents->resizeColumnToContents(1);
    ui->treeWidgetComponents->resizeColumnToContents(2);
}

void MainWindow::_actualizeModelTextHasChanged(bool hasChanged) {
    if (_textModelHasChanged != hasChanged) {
    }
    _textModelHasChanged = hasChanged;
}

void MainWindow::_actualizeModelDataDefinitions(bool force) {
    Model* m = simulator->getModelManager()->current();
    ui->treeWidgetDataDefnitions->clear();
    if (m == nullptr) {
        return;
    }
    for (std::string dataTypename : *m->getDataManager()->getDataDefinitionClassnames()) {
        for (ModelDataDefinition* comp : *m->getDataManager()->getDataDefinitionList(dataTypename)->list()) {
            QList<QTreeWidgetItem *> items = ui->treeWidgetDataDefnitions->findItems(QString::fromStdString(std::to_string(comp->getId())), Qt::MatchExactly | Qt::MatchRecursive, 0);
            if (items.size() == 0) {
                QTreeWidgetItem* treeComp = new QTreeWidgetItem(ui->treeWidgetDataDefnitions);
                treeComp->setText(0, QString::fromStdString(std::to_string(comp->getId())));
                treeComp->setText(1, QString::fromStdString(comp->getClassname()));
                treeComp->setText(2, QString::fromStdString(comp->getName()));
                std::string properties = "";
                for (auto prop : *comp->getProperties()->list()) {
                    properties += prop->getName() + ":" + prop->getValue() + ", ";
                }
                properties = properties.substr(0, properties.length() - 2);
                treeComp->setText(3, QString::fromStdString(properties));
            }
        }
    }
    ui->treeWidgetDataDefnitions->resizeColumnToContents(0);
    ui->treeWidgetDataDefnitions->resizeColumnToContents(1);
    ui->treeWidgetDataDefnitions->resizeColumnToContents(2);
}

void MainWindow::_actualizeGraphicalModel(SimulationEvent * re) {
    Event* event = re->getCurrentEvent();
    if (event != nullptr) {
        ui->graphicsView->selectModelComponent(event->getComponent());
    }
}

void MainWindow::_generateGraphicalModelFromModel() {
    Model* m=simulator->getModelManager()->current();
    if (m!=nullptr) {
        ui->graphicsView->setCanNotifyGraphicalModelEventHandlers(false);
        //ui->graphicsView->getScene()->showGrid();
        int x, y, ymax;
        x=TraitsGUI<GView>::sceneCenter - TraitsGUI<GView>::sceneDistanceCenter*0.8; //ui->graphicsView->sceneRect().left();
        y=TraitsGUI<GView>::sceneCenter - TraitsGUI<GView>::sceneDistanceCenter*0.8; //ui->graphicsView->sceneRect().top();
        ymax=y;
        ComponentManager* cm = m->getComponentManager();
        if (cm == nullptr) {
            ui->graphicsView->setCanNotifyGraphicalModelEventHandlers(true);
            return;
        }
        List<ModelComponent*>* visited = new List<ModelComponent*>();
        std::map<ModelComponent*,GraphicalModelComponent*>* map = new std::map<ModelComponent*,GraphicalModelComponent*>();
        for(SourceModelComponent* source: *cm->getSourceComponents()) {
            _recursivalyGenerateGraphicalModelFromModel(source, visited, map, &x, &y, &ymax, 0);
            y= ymax + TraitsGUI<GModelComponent>::width * TraitsGUI<GModelComponent>::heightProportion * 3; // get heigth mapped to scene??
        }
        // check if any component remains unvisited
        bool foundNotVisited;
        do {
            foundNotVisited = false;
            for (ModelComponent* comp: *cm->getAllComponents()) {
                if (visited->find(comp) == visited->list()->end()) { // found a compponent not visited yet
                    foundNotVisited = true;
                    visited->insert(comp);
                    // recursive create
                }
            }
        } while (foundNotVisited);
        delete map;
        delete visited;
        ui->graphicsView->setCanNotifyGraphicalModelEventHandlers(true);
    }
}


//-----------------------------------------

void MainWindow::_initModelGraphicsView() {
    ((ModelGraphicsView*) (ui->graphicsView))->setSceneMouseEventHandler(this, &MainWindow::_onSceneMouseEvent);
    ((ModelGraphicsView *)(ui->graphicsView))->setSceneWheelInEventHandler(this, &MainWindow::_onSceneWheelInEvent);
    ((ModelGraphicsView *)(ui->graphicsView))->setSceneWheelOutEventHandler(this, &MainWindow::_onSceneWheelOutEvent);
    ((ModelGraphicsView*) (ui->graphicsView))->setGraphicalModelEventHandler(this, &MainWindow::_onSceneGraphicalModelEvent);
    connect(ui->graphicsView->scene(), &QGraphicsScene::changed, this, &MainWindow::sceneChanged);
    connect(ui->graphicsView->scene(), &QGraphicsScene::focusItemChanged, this, &MainWindow::sceneFocusItemChanged);
    connect(ui->graphicsView->scene(), &QGraphicsScene::selectionChanged, this, &MainWindow::sceneSelectionChanged);

    // Cria uma stack undo/redo
    ui->graphicsView->getScene()->setUndoStack(new QUndoStack(this));
}
