// Unit tests for GraphNetwork and directed/DAG graph-network specializations.

#include <gtest/gtest.h>

#include "kernel/simulator/Persistence.h"
#include "kernel/simulator/Plugin.h"
#include "kernel/simulator/PluginInformation.h"
#include "kernel/simulator/PluginManager.h"
#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/model/Model.h"
#include "plugins/data/ModalModel/DirectedAcyclicGraphNetwork.h"
#include "plugins/data/ModalModel/DirectedGraphNetwork.h"
#include "plugins/data/ModalModel/GraphEdge.h"
#include "plugins/data/ModalModel/GraphNetwork.h"
#include "plugins/data/ModalModel/GraphNode.h"
#include "plugins/data/ModalModel/NetworkActivation.h"

#include <memory>
#include <string>
#include <vector>

namespace {

class FakeModelPersistenceRuntime : public Persistence_if {
public:
	bool save(std::string) override { return false; }
	bool load(std::string) override { return false; }
	bool hasChanged() override { return false; }
	void setHasChanged(bool) override {}
	bool getOption(Persistence_if::Options) override { return false; }
	void setOption(Persistence_if::Options, bool) override {}
	std::string getFormatedField(PersistenceRecord*) override { return ""; }
};

class GraphNetworkProbe : public GraphNetwork {
public:
	GraphNetworkProbe(Model* model, const std::string& name = "") : GraphNetwork(model, name) {}
	bool CheckProbe(std::string& errorMessage) { return _check(errorMessage); }
	void SaveInstanceProbe(PersistenceRecord* fields, bool saveDefaultValues = true) { _saveInstance(fields, saveDefaultValues); }
	bool LoadInstanceProbe(PersistenceRecord* fields) { return _loadInstance(fields); }
	void InitBetweenReplicationsProbe() { _initBetweenReplications(); }
};

class DirectedGraphNetworkProbe : public DirectedGraphNetwork {
public:
	DirectedGraphNetworkProbe(Model* model, const std::string& name = "") : DirectedGraphNetwork(model, name) {}
	bool CheckProbe(std::string& errorMessage) { return _check(errorMessage); }
	void SaveInstanceProbe(PersistenceRecord* fields, bool saveDefaultValues = true) { _saveInstance(fields, saveDefaultValues); }
	bool LoadInstanceProbe(PersistenceRecord* fields) { return _loadInstance(fields); }
	void InitBetweenReplicationsProbe() { _initBetweenReplications(); }
};

class DirectedAcyclicGraphNetworkProbe : public DirectedAcyclicGraphNetwork {
public:
	DirectedAcyclicGraphNetworkProbe(Model* model, const std::string& name = "") : DirectedAcyclicGraphNetwork(model, name) {}
	bool CheckProbe(std::string& errorMessage) { return _check(errorMessage); }
	void SaveInstanceProbe(PersistenceRecord* fields, bool saveDefaultValues = true) { _saveInstance(fields, saveDefaultValues); }
	bool LoadInstanceProbe(PersistenceRecord* fields) { return _loadInstance(fields); }
};

std::vector<std::string> names(const std::vector<GraphNode*>& nodes) {
	std::vector<std::string> result;
	for (GraphNode* node : nodes) {
		result.push_back(node != nullptr ? node->getName() : "");
	}
	return result;
}

} // namespace

TEST(GraphNetworkTest, EmptyGraphAndSingleNodeHaveStructuralSemanticsOnly) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);

	GraphNetwork graph(model, "Graph");
	EXPECT_FALSE(graph.isDirected());
	EXPECT_TRUE(graph.getNodes().empty());
	EXPECT_TRUE(graph.getEdges().empty());
	EXPECT_TRUE(graph.connectedComponents().empty());
	EXPECT_FALSE(graph.hasCycle());

	NetworkActivationResult result = graph.activate(NetworkActivationFrame(0));
	EXPECT_EQ(result.size(), 0u);
	EXPECT_DOUBLE_EQ(graph.getActivationCount(), 1.0);

	GraphNode a(model, "A");
	EXPECT_TRUE(graph.addNode(&a));
	EXPECT_EQ(graph.getNodes().size(), 1u);
	EXPECT_EQ(graph.degree(&a), 0u);
	ASSERT_EQ(graph.connectedComponents().size(), 1u);
	EXPECT_EQ(names(graph.connectedComponents().front()), std::vector<std::string>({"A"}));
}

TEST(GraphNetworkTest, UndirectedEdgeProvidesSymmetricAdjacencyAndDegrees) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	GraphNetwork graph(model, "Graph");
	GraphNode a(model, "A");
	GraphNode b(model, "B");
	GraphEdge ab(model, &a, &b, "AB");
	graph.addNode(&a);
	graph.addNode(&b);

	ASSERT_TRUE(graph.addEdge(&ab));

	EXPECT_FALSE(ab.isDirected());
	EXPECT_TRUE(graph.hasEdge(&a, &b));
	EXPECT_TRUE(graph.hasEdge(&b, &a));
	EXPECT_EQ(names(graph.getNeighbors(&a)), std::vector<std::string>({"B"}));
	EXPECT_EQ(names(graph.getNeighbors(&b)), std::vector<std::string>({"A"}));
	EXPECT_EQ(graph.degree(&a), 1u);
	EXPECT_EQ(graph.degree(&b), 1u);
	EXPECT_TRUE(graph.hasPath(&a, &b));
	EXPECT_TRUE(graph.hasPath(&b, &a));
}

TEST(GraphNetworkTest, DirectedEdgeDistinguishesSuccessorsAndPredecessors) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	DirectedGraphNetwork graph(model, "Digraph");
	GraphNode a(model, "A");
	GraphNode b(model, "B");
	GraphEdge ab(model, &a, &b, "AB");
	graph.addNode(&a);
	graph.addNode(&b);

	ASSERT_TRUE(graph.addEdge(&ab));

	EXPECT_TRUE(ab.isDirected());
	EXPECT_TRUE(graph.hasEdge(&a, &b));
	EXPECT_FALSE(graph.hasEdge(&b, &a));
	EXPECT_EQ(names(graph.getSuccessors(&a)), std::vector<std::string>({"B"}));
	EXPECT_TRUE(graph.getSuccessors(&b).empty());
	EXPECT_TRUE(graph.getPredecessors(&a).empty());
	EXPECT_EQ(names(graph.getPredecessors(&b)), std::vector<std::string>({"A"}));
	EXPECT_EQ(graph.outDegree(&a), 1u);
	EXPECT_EQ(graph.inDegree(&b), 1u);
	EXPECT_TRUE(graph.hasPath(&a, &b));
	EXPECT_FALSE(graph.hasPath(&b, &a));
}

TEST(GraphNetworkTest, SelfLoopIsStoredAndDetectedAsCycle) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	DirectedGraphNetwork graph(model, "Digraph");
	GraphNode a(model, "A");
	GraphEdge loop(model, &a, &a, "Loop");
	graph.addNode(&a);

	ASSERT_TRUE(graph.addEdge(&loop));

	EXPECT_EQ(graph.getEdges().size(), 1u);
	EXPECT_EQ(graph.inDegree(&a), 1u);
	EXPECT_EQ(graph.outDegree(&a), 1u);
	EXPECT_TRUE(graph.hasCycle());
	EXPECT_TRUE(graph.hasPath(&a, &a));
}

TEST(GraphNetworkTest, ParallelEdgesKeepIndependentIdentityAndWeights) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	DirectedGraphNetwork graph(model, "Digraph");
	GraphNode a(model, "A");
	GraphNode b(model, "B");
	GraphEdge e1(model, &a, &b, "e1");
	GraphEdge e2(model, &a, &b, "e2");
	e1.setWeight(1.5);
	e2.setWeight(2.5);
	graph.addNode(&a);
	graph.addNode(&b);

	ASSERT_TRUE(graph.addEdge(&e1));
	ASSERT_TRUE(graph.addEdge(&e2));

	std::vector<GraphEdge*> between = graph.getEdgesBetween(&a, &b);
	ASSERT_EQ(between.size(), 2u);
	EXPECT_EQ(between[0]->getName(), "e1");
	EXPECT_EQ(between[1]->getName(), "e2");
	EXPECT_DOUBLE_EQ(between[0]->getWeight(), 1.5);
	EXPECT_DOUBLE_EQ(between[1]->getWeight(), 2.5);
	EXPECT_EQ(graph.outDegree(&a), 2u);
	EXPECT_EQ(graph.inDegree(&b), 2u);
}

TEST(GraphNetworkTest, BreadthFirstSearchAndUnweightedShortestPathAreDeterministic) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	GraphNetwork graph(model, "Graph");
	GraphNode a(model, "A");
	GraphNode b(model, "B");
	GraphNode c(model, "C");
	GraphNode d(model, "D");
	GraphEdge ab(model, &a, &b, "AB");
	GraphEdge ac(model, &a, &c, "AC");
	GraphEdge bd(model, &b, &d, "BD");
	GraphEdge cd(model, &c, &d, "CD");
	for (GraphNode* node : {&a, &b, &c, &d}) {
		graph.addNode(node);
	}
	for (GraphEdge* edge : {&ab, &ac, &bd, &cd}) {
		graph.addEdge(edge);
	}

	GraphTraversalResult bfs = graph.breadthFirstSearch(&a);
	EXPECT_EQ(names(bfs.visitOrder), std::vector<std::string>({"A", "B", "C", "D"}));
	EXPECT_EQ(bfs.predecessorOf(&d), &b);
	EXPECT_EQ(bfs.distanceTo(&d), 2u);

	GraphPathResult path = graph.shortestPathUnweighted(&a, &d);
	ASSERT_TRUE(path.reachable);
	EXPECT_DOUBLE_EQ(path.distance, 2.0);
	EXPECT_EQ(names(path.nodes), std::vector<std::string>({"A", "B", "D"}));
	ASSERT_EQ(path.edges.size(), 2u);
	EXPECT_EQ(path.edges[0]->getName(), "AB");
	EXPECT_EQ(path.edges[1]->getName(), "BD");
}

TEST(GraphNetworkTest, DepthFirstSearchUsesInsertionOrderAndHandlesDisconnectedNodes) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	GraphNetwork graph(model, "Graph");
	GraphNode a(model, "A");
	GraphNode b(model, "B");
	GraphNode c(model, "C");
	GraphNode d(model, "D");
	GraphEdge ab(model, &a, &b, "AB");
	GraphEdge ac(model, &a, &c, "AC");
	graph.addNode(&a);
	graph.addNode(&b);
	graph.addNode(&c);
	graph.addNode(&d);
	graph.addEdge(&ab);
	graph.addEdge(&ac);

	GraphTraversalResult dfs = graph.depthFirstSearch(&a);

	EXPECT_EQ(names(dfs.visitOrder), std::vector<std::string>({"A", "B", "C"}));
	EXPECT_FALSE(dfs.visited(&d));
	EXPECT_EQ(names(graph.reachableNodes(&a)), std::vector<std::string>({"A", "B", "C"}));
}

TEST(GraphNetworkTest, WeightedShortestPathUsesDijkstraForNonnegativeWeights) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	DirectedGraphNetwork graph(model, "Digraph");
	GraphNode a(model, "A");
	GraphNode b(model, "B");
	GraphNode c(model, "C");
	GraphEdge ab(model, &a, &b, "AB");
	GraphEdge ac(model, &a, &c, "AC");
	GraphEdge bc(model, &b, &c, "BC");
	ab.setWeight(1.0);
	ac.setWeight(5.0);
	bc.setWeight(1.0);
	graph.addNode(&a);
	graph.addNode(&b);
	graph.addNode(&c);
	graph.addEdge(&ab);
	graph.addEdge(&ac);
	graph.addEdge(&bc);

	GraphPathResult path = graph.shortestPathDijkstra(&a, &c);

	ASSERT_TRUE(path.reachable);
	EXPECT_DOUBLE_EQ(path.distance, 2.0);
	EXPECT_EQ(names(path.nodes), std::vector<std::string>({"A", "B", "C"}));
	ASSERT_EQ(path.edges.size(), 2u);
	EXPECT_EQ(path.edges[0]->getName(), "AB");
	EXPECT_EQ(path.edges[1]->getName(), "BC");
}

TEST(GraphNetworkTest, ShortestPathReportsUnreachableAndRejectsNegativeWeights) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	DirectedGraphNetwork graph(model, "Digraph");
	GraphNode a(model, "A");
	GraphNode b(model, "B");
	GraphNode c(model, "C");
	GraphEdge ab(model, &a, &b, "AB");
	graph.addNode(&a);
	graph.addNode(&b);
	graph.addNode(&c);
	graph.addEdge(&ab);

	GraphPathResult unreachable = graph.shortestPathDijkstra(&b, &c);
	EXPECT_FALSE(unreachable.reachable);
	EXPECT_TRUE(unreachable.nodes.empty());

	ab.setWeight(-1.0);
	GraphPathResult rejected = graph.shortestPathDijkstra(&a, &b);
	EXPECT_FALSE(rejected.reachable);
	EXPECT_NE(rejected.errorMessage.find("nonnegative"), std::string::npos);
}

TEST(GraphNetworkTest, ConnectedComponentsRespectUndirectedConnectivity) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	GraphNetwork graph(model, "Graph");
	GraphNode a(model, "A");
	GraphNode b(model, "B");
	GraphNode c(model, "C");
	GraphNode d(model, "D");
	GraphEdge ab(model, &a, &b, "AB");
	GraphEdge cd(model, &c, &d, "CD");
	for (GraphNode* node : {&a, &b, &c, &d}) {
		graph.addNode(node);
	}
	graph.addEdge(&ab);
	graph.addEdge(&cd);

	std::vector<std::vector<GraphNode*>> components = graph.connectedComponents();

	ASSERT_EQ(components.size(), 2u);
	EXPECT_EQ(names(components[0]), std::vector<std::string>({"A", "B"}));
	EXPECT_EQ(names(components[1]), std::vector<std::string>({"C", "D"}));
}

TEST(GraphNetworkTest, DirectedCycleDetectionAndStronglyConnectedComponents) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	DirectedGraphNetwork graph(model, "Digraph");
	GraphNode a(model, "A");
	GraphNode b(model, "B");
	GraphNode c(model, "C");
	GraphNode d(model, "D");
	GraphEdge ab(model, &a, &b, "AB");
	GraphEdge ba(model, &b, &a, "BA");
	GraphEdge cd(model, &c, &d, "CD");
	for (GraphNode* node : {&a, &b, &c, &d}) {
		graph.addNode(node);
	}
	graph.addEdge(&ab);
	graph.addEdge(&ba);
	graph.addEdge(&cd);

	EXPECT_TRUE(graph.hasCycle());
	std::vector<std::vector<GraphNode*>> components = graph.stronglyConnectedComponents();

	ASSERT_EQ(components.size(), 3u);
	EXPECT_EQ(components[0].size(), 2u);
	EXPECT_TRUE((components[0][0] == &b && components[0][1] == &a) || (components[0][0] == &a && components[0][1] == &b));
}

TEST(GraphNetworkTest, DirectedAcyclicGraphRejectsCycleAndSupportsTopologicalOrder) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	DirectedAcyclicGraphNetworkProbe dag(model, "Dag");
	GraphNode a(model, "A");
	GraphNode b(model, "B");
	GraphNode c(model, "C");
	GraphEdge ab(model, &a, &b, "AB");
	GraphEdge bc(model, &b, &c, "BC");
	GraphEdge ca(model, &c, &a, "CA");
	dag.addNode(&a);
	dag.addNode(&b);
	dag.addNode(&c);

	ASSERT_TRUE(dag.addEdge(&ab));
	ASSERT_TRUE(dag.addEdge(&bc));
	EXPECT_FALSE(dag.addEdge(&ca));
	EXPECT_FALSE(dag.hasCycle());

	GraphTopologicalOrderResult order = dag.topologicalOrder();
	ASSERT_TRUE(order.acyclic) << order.errorMessage;
	EXPECT_EQ(names(order.order), std::vector<std::string>({"A", "B", "C"}));

	std::string errorMessage;
	EXPECT_TRUE(dag.CheckProbe(errorMessage)) << errorMessage;
}

TEST(GraphNetworkTest, RemovingNodeRemovesIncidentEdgesWithoutDeletingOtherTopology) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	GraphNetwork graph(model, "Graph");
	GraphNode a(model, "A");
	GraphNode b(model, "B");
	GraphNode c(model, "C");
	GraphEdge ab(model, &a, &b, "AB");
	GraphEdge bc(model, &b, &c, "BC");
	graph.addNode(&a);
	graph.addNode(&b);
	graph.addNode(&c);
	graph.addEdge(&ab);
	graph.addEdge(&bc);

	ASSERT_TRUE(graph.removeNode(&b));

	EXPECT_FALSE(graph.containsNode(&b));
	EXPECT_EQ(graph.getEdges().size(), 0u);
	EXPECT_TRUE(graph.containsNode(&a));
	EXPECT_TRUE(graph.containsNode(&c));
}

TEST(GraphNetworkTest, PersistenceRoundTripPreservesDirectedWeightedMultigraph) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);

	DirectedGraphNetworkProbe source(model, "PersistentDigraph");
	GraphNode a(model, "A");
	GraphNode b(model, "B");
	GraphNode c(model, "C");
	GraphEdge ab1(model, &a, &b, "AB1");
	GraphEdge ab2(model, &a, &b, "AB2");
	GraphEdge aa(model, &a, &a, "AA");
	ab1.setWeight(4.7);
	ab2.setWeight(2.3);
	for (GraphNode* node : {&a, &b, &c}) {
		source.addNode(node);
	}
	for (GraphEdge* edge : {&ab1, &ab2, &aa}) {
		source.addEdge(edge);
	}

	FakeModelPersistenceRuntime persistence;
	PersistenceRecord fields(persistence);
	source.SaveInstanceProbe(&fields, true);

	DirectedGraphNetworkProbe loaded(model, "LoadedDigraph");
	ASSERT_TRUE(loaded.LoadInstanceProbe(&fields));

	ASSERT_EQ(loaded.getNodes().size(), 3u);
	ASSERT_EQ(loaded.getEdges().size(), 3u);
	GraphNode* loadedA = loaded.findNodeByName("A");
	GraphNode* loadedB = loaded.findNodeByName("B");
	GraphNode* loadedC = loaded.findNodeByName("C");
	ASSERT_NE(loadedA, nullptr);
	ASSERT_NE(loadedB, nullptr);
	ASSERT_NE(loadedC, nullptr);
	EXPECT_TRUE(loaded.getIncidentEdges(loadedC).empty());

	std::vector<GraphEdge*> between = loaded.getEdgesBetween(loadedA, loadedB);
	ASSERT_EQ(between.size(), 2u);
	EXPECT_TRUE(between[0]->isDirected());
	EXPECT_TRUE(between[0]->hasWeight());
	EXPECT_DOUBLE_EQ(between[0]->getWeight(), 4.7);
	EXPECT_DOUBLE_EQ(between[1]->getWeight(), 2.3);
	EXPECT_TRUE(loaded.hasEdge(loadedA, loadedA));
	EXPECT_TRUE(loaded.hasCycle());
}

TEST(GraphNetworkTest, PluginRegistrationCreatesGraphNetworkDataDefinitions) {
	Simulator simulator;
	PluginManager* plugins = simulator.getPluginManager();
	ASSERT_NE(plugins, nullptr);

	ASSERT_NE(plugins->insert("graphnode.so"), nullptr);
	ASSERT_NE(plugins->insert("graphedge.so"), nullptr);
	ASSERT_NE(plugins->insert("graphnetwork.so"), nullptr);
	ASSERT_NE(plugins->insert("directedgraphnetwork.so"), nullptr);
	ASSERT_NE(plugins->insert("directedacyclicgraphnetwork.so"), nullptr);

	std::unique_ptr<PluginInformation> graphInfo(GraphNetwork::GetPluginInformation());
	std::unique_ptr<PluginInformation> directedInfo(DirectedGraphNetwork::GetPluginInformation());
	std::unique_ptr<PluginInformation> dagInfo(DirectedAcyclicGraphNetwork::GetPluginInformation());

	EXPECT_FALSE(graphInfo->isComponent());
	EXPECT_FALSE(directedInfo->isComponent());
	EXPECT_FALSE(dagInfo->isComponent());
	EXPECT_EQ(graphInfo->getCategory(), "ModalModel");
	EXPECT_EQ(directedInfo->getCategory(), "ModalModel");
	EXPECT_EQ(dagInfo->getCategory(), "ModalModel");
}
