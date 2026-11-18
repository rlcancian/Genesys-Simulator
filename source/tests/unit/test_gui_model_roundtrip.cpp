#include <gtest/gtest.h>
#include <QApplication>
#include <QAction>
#include <QFile>
#include <QPlainTextEdit>
#include <QScrollBar>
#include <QSlider>
#include <QTemporaryDir>
#include <QTextEdit>
#include <QUndoStack>
#include <set>
#include "kernel/simulator/OnEventManager.h"
#include "graphicals/ModelGraphicsView.h"
#include "graphicals/GraphicalConnection.h"
#include "services/GraphicalModelBuilder.h"
#include "services/GraphicalModelSerializer.h"
#include "controllers/TraceConsoleController.h"
#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/model/Model.h"
#include "plugins/components/Logic/Create.h"
#include "plugins/components/Logic/Assign.h"
#include "plugins/components/Logic/Dispose.h"
#include "plugins/components/Decisions/Decide.h"
#include "plugins/components/DiscreteProcessing/Delay.h"
#include "plugins/components/DiscreteProcessing/Seize.h"
#include "plugins/components/DiscreteProcessing/Release.h"
#include "plugins/data/Logic/Set.h"
#include "plugins/data/DiscreteProcessing/Resource.h"
#include "plugins/data/DiscreteProcessing/Queue.h"

namespace {
void events() { for(int i=0;i<8;++i) QApplication::processEvents(); }
struct RoundTripGui {
    Simulator simulator;
    ModelGraphicsView view;
    PropertyEditorGenesys properties;
    std::map<SimulationControl*,DataComponentProperty*> propertyList;
    std::map<SimulationControl*,DataComponentEditor*> editors;
    std::map<SimulationControl*,ComboBoxEnum*> combos;
    QUndoStack undo;
    QPlainTextEdit text;
    QTextEdit console;
    QSlider zoom;
    QAction grid,rule,snap,guides,statistics,editable,shared,recursive;
    std::map<std::string,QColor> colors;
    QString filename;
    GraphicalModelBuilder builder;
    GraphicalModelSerializer serializer;
    RoundTripGui() : builder(&simulator,&view,view.getScene(),&colors,&console),
        serializer(&simulator,&view,&text,&view,&zoom,&grid,&rule,&snap,&guides,
                   &statistics,&editable,&shared,&recursive,&console,&filename,
                   [this]{clear();},[this]{builder.generateGraphicalModelFromModel();},
                   [this]{view.getScene()->setShowStatisticsDataDefinitions(statistics.isChecked());},
                   [this]{view.getScene()->setShowEditableDataDefinitions(editable.isChecked());},
                   [this]{view.getScene()->setShowSharedDataDefinitions(shared.isChecked());}) {
        simulator.getPluginManager()->autoInsertPlugins();
        view.setSimulator(&simulator);
        auto* scene=view.getScene();scene->setSimulator(&simulator);scene->setUndoStack(&undo);
        scene->setPropertyEditor(&properties);scene->setPropertyList(&propertyList);
        scene->setPropertyEditorUI(&editors);scene->setComboBox(&combos);
        for(auto* action:{&grid,&rule,&snap,&guides,&statistics,&editable,&shared,&recursive}) action->setCheckable(true);
        zoom.setRange(10,1000);zoom.setValue(100);
        QObject::connect(&zoom,&QSlider::valueChanged,&view,[this](int value){QTransform t;t.scale(value/100.0,value/100.0);view.setTransform(t);});
        view.resize(900,700);view.show();events();
    }
    void clear() {
        auto* s=view.getScene();undo.clear();s->grid()->clear();
        s->clearGraphicalModelConnections();s->clearGraphicalDiagramConnections();
        s->clearGraphicalModelComponents();s->clearGraphicalModelDataDefinitions();
        s->clearAnimations();s->clear();
        s->getGraphicalModelComponents()->clear();s->getGraphicalConnections()->clear();
        s->getGraphicalModelDataDefinitions()->clear();s->getGraphicalDiagramsConnections()->clear();
        s->getAllComponents()->clear();s->getAllConnections()->clear();s->getAllDataDefinitions()->clear();s->getAllGraphicalDiagramsConnections()->clear();
    }
};
using Edge=std::tuple<unsigned long long,unsigned int,unsigned long long,unsigned int>;
std::multiset<Edge> logicalEdges(Model* model) {
    std::multiset<Edge> edges;
    for(auto* c:*model->getComponentManager()->getAllComponents())
        for(auto&[port,connection]:*c->getConnectionManager()->connections())
            if(connection&&connection->component)edges.emplace(c->getId(),port,connection->component->getId(),connection->channel.portNumber);
    return edges;
}
void expectGraph(RoundTripGui& gui,Model* model) {
    std::multiset<Edge> graphical;
    for(auto* item:*gui.view.getScene()->getGraphicalConnections()) {
        auto* e=dynamic_cast<GraphicalConnection*>(item);ASSERT_NE(e,nullptr);
        graphical.emplace(e->getSourceGraphicalComponent()->getComponent()->getId(),e->getPortSourceConnection(),
                          e->getDestinationGraphicalComponent()->getComponent()->getId(),e->getPortDestinationConnection());
        EXPECT_FALSE(e->boundingRect().isEmpty());
    }
    EXPECT_EQ(graphical,logicalEdges(model));
}
void expectTwoCycles(RoundTripGui& gui,Model* model) {
    QTemporaryDir dir;ASSERT_TRUE(dir.isValid());
    for(int round=0;round<2;++round) {
        auto* scene=gui.view.getScene();
        scene->setGridVisible(round==0);scene->setSnapToGrid(round!=0);
        gui.view.setRuleVisible(round!=0);gui.view.setGuidesVisible(round==0);
        scene->setShowStatisticsDataDefinitions(round!=0);scene->setShowEditableDataDefinitions(true);
        scene->setShowSharedDataDefinitions(round==0);scene->setShowRecursiveDataDefinitions(true);
        GraphicalModelBuilder::synchronizeGraphicalDataDefinitionsLayer(&gui.simulator,scene);
        // The scene is authoritative even if an action has become stale.
        gui.grid.setChecked(round!=0);
        gui.zoom.setValue(round==0?143:219);events();
        gui.view.horizontalScrollBar()->setValue(23000+round*100);
        gui.view.verticalScrollBar()->setValue(24000+round*100);events();
        const auto h=gui.view.horizontalScrollBar()->value(),v=gui.view.verticalScrollBar()->value();
        const auto transform=gui.view.transform();
        const auto edges=logicalEdges(model);
        std::map<Util::identification,std::tuple<std::string,std::string,std::string,QPointF,QColor>> components;
        std::map<Util::identification,QPointF> definitions;
        std::map<std::pair<Util::identification,QString>,QPointF> ports;
        for(auto* c:scene->graphicalModelComponentItems()) {
            auto* k=c->getComponent();components[k->getId()]={k->getName(),k->getLabel(),k->getClassname(),c->pos(),c->getColor()};
            for(auto* port:c->getGraphicalInputPorts())ports[{k->getId(),"in"+QString::number(port->portNum())}]=port->pos();
            for(auto* port:c->getGraphicalOutputPorts())ports[{k->getId(),"out"+QString::number(port->portNum())}]=port->pos();
        }
        for(auto* d:*scene->getAllDataDefinitions())definitions[d->getDataDefinition()->getId()]=d->pos();
        const auto file=dir.filePath(QString("cycle%1.gui").arg(round));
        ASSERT_TRUE(gui.serializer.saveGraphicalModel(file));
        model=gui.serializer.loadGraphicalModel(file.toStdString());ASSERT_NE(model,nullptr);events();
        EXPECT_EQ(logicalEdges(model),edges);expectGraph(gui,model);
        EXPECT_EQ(scene->isGridVisible(),round==0);EXPECT_EQ(gui.grid.isChecked(),round==0);
        for(auto* line:*scene->grid()->lines)EXPECT_EQ(line->isVisible(),round==0);
        EXPECT_EQ(scene->getSnapToGrid(),round!=0);EXPECT_EQ(gui.view.isRuleVisible(),round!=0);EXPECT_EQ(gui.view.isGuidesVisible(),round==0);
        EXPECT_EQ(scene->showStatisticsDataDefinitions(),round!=0);EXPECT_TRUE(scene->showEditableDataDefinitions());
        EXPECT_EQ(scene->showSharedDataDefinitions(),round==0);EXPECT_TRUE(scene->showRecursiveDataDefinitions());
        EXPECT_EQ(gui.view.transform(),transform);EXPECT_NEAR(gui.view.horizontalScrollBar()->value(),h,1);EXPECT_NEAR(gui.view.verticalScrollBar()->value(),v,1);
        for(auto* c:scene->graphicalModelComponentItems()) {
            auto* k=c->getComponent();ASSERT_TRUE(components.contains(k->getId()));auto& [name,label,type,pos,color]=components.at(k->getId());
            EXPECT_EQ(k->getName(),name);EXPECT_EQ(k->getLabel(),label);EXPECT_EQ(k->getClassname(),type);
            EXPECT_NEAR(c->pos().x(),pos.x(),0.001);EXPECT_NEAR(c->pos().y(),pos.y(),0.001);EXPECT_EQ(c->getColor(),color);
            for(auto* port:c->getGraphicalInputPorts()){auto expected=ports.at({k->getId(),"in"+QString::number(port->portNum())});EXPECT_NEAR(port->pos().x(),expected.x(),0.001);EXPECT_NEAR(port->pos().y(),expected.y(),0.001);}
            for(auto* port:c->getGraphicalOutputPorts()){auto expected=ports.at({k->getId(),"out"+QString::number(port->portNum())});EXPECT_NEAR(port->pos().x(),expected.x(),0.001);EXPECT_NEAR(port->pos().y(),expected.y(),0.001);}
        }
        for(auto* d:*scene->getAllDataDefinitions())if(definitions.contains(d->getDataDefinition()->getId())) {
            const auto pos=definitions.at(d->getDataDefinition()->getId());EXPECT_NEAR(d->pos().x(),pos.x(),0.001);EXPECT_NEAR(d->pos().y(),pos.y(),0.001);
        }
    }
}
}
TEST(GraphicalRoundTrip, ConvergenceAndVisualStateSurviveTwoCycles) {
    RoundTripGui gui;
    auto* model=gui.simulator.getModelManager()->newModel();
    auto* resource=new Resource(model,"CPU");auto* queue=new Queue(model,"Queue");auto* set=new Set(model,"Processador4.TotalTimeFailed");set->addElementSet(resource);
    auto* create=new Create(model,"Create");auto* decide=new Decide(model,"Decide");decide->addConditions("1");
    auto* a=new Assign(model,"AssignA");auto* b=new Assign(model,"AssignB");
    a->getAssignments()->insert(new Assignment(model,"attribute","1"));b->getAssignments()->insert(new Assignment(model,"attribute","2"));
    auto* seize=new Seize(model,"Seize");seize->setQueue(queue);seize->addRequest(new SeizableItem(set));
    auto* sink=new Dispose(model,"Sink");create->connectTo(decide);decide->getConnectionManager()->insertAtPort(0,new Connection{a,{0}});decide->getConnectionManager()->insertAtPort(1,new Connection{b,{0}});a->connectTo(seize);b->connectTo(seize,1);seize->connectTo(sink);
    ASSERT_TRUE(model->check());gui.builder.generateGraphicalModelFromModel();expectGraph(gui,model);
    for(auto* c:gui.view.getScene()->graphicalModelComponentItems())c->setColor(QColor("#8a2345"));
    expectTwoCycles(gui,model);
}
TEST(GraphicalRoundTrip, LocalReferenceModelPreservesGraphAndVisualState) {
    const auto path=qEnvironmentVariable("GENESYS_REFERENCE_GUI");
    if(path.isEmpty())GTEST_SKIP()<<"Set GENESYS_REFERENCE_GUI to exercise a supplied model without changing it";
    RoundTripGui gui;
    auto* model=gui.serializer.loadGraphicalModel(path.toStdString());ASSERT_NE(model,nullptr);events();
    expectGraph(gui,model);
    auto* seize=dynamic_cast<Seize*>(model->getComponentManager()->find("Seize_1"));ASSERT_NE(seize,nullptr);
    EXPECT_TRUE(seize->getPriorityExpression().empty());
    auto* set=seize->getSeizeRequests()->getAtRank(0)->getSet();ASSERT_NE(set,nullptr);
    EXPECT_EQ(set->getName(),"Processador4.TotalTimeFailed");ASSERT_EQ(set->getElementSet()->size(),4u);
    expectTwoCycles(gui,model);
}

TEST(GraphicalRoundTrip, LocalReferenceExecutionUsesExplicitValidationCopy) {
    const auto path=qEnvironmentVariable("GENESYS_REFERENCE_GUI");
    if(path.isEmpty())GTEST_SKIP()<<"Set GENESYS_REFERENCE_GUI for the supplied-model execution check";
    struct Observations {
        int errors=0,syntaxErrors=0,wrongRelease=0,backwards=0,releaseEvents=0;
        double lastTime=0;
        void error(TraceErrorEvent e) {
            ++errors;const auto message=e.getText()+e.getExceptionMessage();
            if(message.find("syntax error")!=std::string::npos)++syntaxErrors;
            if(message.find("cannot release")!=std::string::npos)++wrongRelease;
        }
        void replication(SimulationEvent*){lastTime=0;}
        void event(SimulationEvent* e) {
            if(e->getSimulatedTime()<lastTime)++backwards;
            lastTime=e->getSimulatedTime();
            if(e->getCurrentEvent()&&e->getCurrentEvent()->getComponent()&&e->getCurrentEvent()->getComponent()->getClassname()=="Release")++releaseEvents;
        }
    } original,corrected;
    {
        RoundTripGui gui;auto* model=gui.serializer.loadGraphicalModel(path.toStdString());ASSERT_NE(model,nullptr);
        model->getTracer()->addTraceErrorHandler(&original,&Observations::error);
        model->getOnEventManager()->addOnReplicationStartHandler(&original,&Observations::replication);
        model->getOnEventManager()->addOnProcessEventHandler(&original,&Observations::event);
        ASSERT_TRUE(model->check());model->getSimulation()->start();
        EXPECT_GT(original.wrongRelease,0);EXPECT_EQ(original.syntaxErrors,0);
    }
    {
        RoundTripGui gui;auto* model=gui.serializer.loadGraphicalModel(path.toStdString());ASSERT_NE(model,nullptr);
        auto* release=dynamic_cast<Release*>(model->getComponentManager()->find("Release_1"));ASSERT_NE(release,nullptr);
        auto* request=release->getReleaseRequests()->getAtRank(0);
        request->setSelectionRule(SeizableItem::SelectionRule::SPECIFICMEMBER);
        request->setSaveAttribute("meuProcessador");
        // Explicit validation-copy assumption, not a silent change to the user's
        // distributions: bound service times to keep the event calendar monotonic.
        for(auto name:{"Assign_2","Assign_3"}) {
            auto* assign=dynamic_cast<Assign*>(model->getComponentManager()->find(name));ASSERT_NE(assign,nullptr);
            for(auto* assignment:*assign->getAssignments()->list())
                assignment->setExpression("MAX(0,"+assignment->getExpression()+")");
        }
        ASSERT_TRUE(model->check());
        const auto copy=qEnvironmentVariable("GENESYS_VALIDATED_GUI_COPY");
        if(!copy.isEmpty())ASSERT_TRUE(gui.serializer.saveGraphicalModel(copy));
        model->getTracer()->addTraceErrorHandler(&corrected,&Observations::error);
        model->getOnEventManager()->addOnReplicationStartHandler(&corrected,&Observations::replication);
        model->getOnEventManager()->addOnProcessEventHandler(&corrected,&Observations::event);
        model->getSimulation()->start();
        EXPECT_EQ(corrected.errors,0);EXPECT_EQ(corrected.backwards,0);EXPECT_GT(corrected.releaseEvents,0);
        EXPECT_FALSE(model->getSimulation()->isRunning());
    }
    std::cout<<"REFERENCE original errors="<<original.errors<<" syntax="<<original.syntaxErrors<<" wrongRelease="<<original.wrongRelease<<" backwards="<<original.backwards
             <<"; validation-copy errors="<<corrected.errors<<" releases="<<corrected.releaseEvents<<" backwards="<<corrected.backwards<<'\n';
}

TEST(GraphicalRoundTrip, DeferredViewportRestorationDoesNotChangeAnotherModelView) {
    RoundTripGui first,second;
    auto* model=first.simulator.getModelManager()->newModel();
    auto* source=new Create(model,"ViewportSource");source->connectTo(new Dispose(model,"ViewportSink"));
    ASSERT_TRUE(model->check());first.builder.generateGraphicalModelFromModel();
    first.zoom.setValue(143);events();
    first.view.horizontalScrollBar()->setValue(12000);first.view.verticalScrollBar()->setValue(13000);
    const auto x=first.view.horizontalScrollBar()->value(),y=first.view.verticalScrollBar()->value();
    QTemporaryDir directory;ASSERT_TRUE(directory.isValid());const auto file=directory.filePath("view.gui");
    ASSERT_TRUE(first.serializer.saveGraphicalModel(file));ASSERT_NE(first.serializer.loadGraphicalModel(file.toStdString()),nullptr);
    second.zoom.setValue(219);second.view.horizontalScrollBar()->setValue(5000);second.view.verticalScrollBar()->setValue(6000);
    const auto otherX=second.view.horizontalScrollBar()->value(),otherY=second.view.verticalScrollBar()->value();
    const auto otherTransform=second.view.transform();events();
    EXPECT_EQ(second.view.horizontalScrollBar()->value(),otherX);EXPECT_EQ(second.view.verticalScrollBar()->value(),otherY);
    EXPECT_EQ(second.view.transform(),otherTransform);
    EXPECT_NEAR(first.view.horizontalScrollBar()->value(),x,1);EXPECT_NEAR(first.view.verticalScrollBar()->value(),y,1);
}

TEST(GraphicalRoundTrip, ConsolePreservesComponentAndExpressionDiagnostic) {
    QTextEdit console;
    TraceConsoleController controller(&console,nullptr,nullptr);
    std::runtime_error error("Seize_1.PriorityExpression [1 +]: syntax error");
    controller.simulatorTraceErrorHandler(TraceErrorEvent("Error executing component Seize_1",error));
    EXPECT_TRUE(console.toPlainText().contains("Error executing component Seize_1"));
    EXPECT_TRUE(console.toPlainText().contains("PriorityExpression [1 +]: syntax error"));
}
