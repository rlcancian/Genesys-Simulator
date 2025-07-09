#include "Loja01.h"


// GEnSyS Simulator
#include "../../../../kernel/simulator/Simulator.h"
// Model Components
#include "../../../../plugins/components/Create.h"
#include "../../../../plugins/components/Delay.h"
#include "../../../../plugins/components/Dispose.h"
#include "../../../../plugins/components/Seize.h"
#include "../../../../plugins/components/Release.h"
// Model data definitions
#include "../../../../kernel/simulator/Attribute.h"
#include "../../../../plugins/components/SeizableItem.h"
#include "../../../../plugins/components/QueueableItem.h"

Loja01::Loja01() {
}

int Loja01::main(int argc, char** argv) {
    //
    // Criando o simulador, vinculando os tratadores de eventos para mostrar mensagens na tela, e carregando os plugins.
    // Código "padrão"
    Simulator* genesys = new Simulator();
    this->setDefaultTraceHandlers(genesys->getTraceManager());
    genesys->getPluginManager()->autoInsertPlugins("autoloadplugins.txt");
    //
    // Instanciando um modelo
    Model* model = genesys->getModelManager()->newModel();
    PluginManager* plugins = genesys->getPluginManager();
    //
    // Ajustando informações do modelo. Não é importante, mas fornece alguma informações básicas
    //    sobre o que está sendo modelado
    ModelInfo* infos = model->getInfos();
    infos->setName("Loja 01");
    infos->setDescription("Primeiro exemplo da lista de exercícios do Professor Cancian");
    model->getTracer()->setTraceLevel(TraceManager::Level::L9_mostDetailed);
    //
    // Criando os componentes do modelo. Corresponde a arrastar e soltar os componentes
    //   da aba de componentes para a área de modelagem
    // Basicamente, pedimos ao gerenciador de plugins para que crie uma instância
    //   do componente que queremos, e já o adicione ao modelo, com certo "caption"
    //   (os dois parâmetros do método newInstance, que é uma "fábrica")
    //
    // No modelo da Loja 1, precisamos de um "Create" para a chegada de clientes,
    //    de um "Seize" para os clientes tentarem alocar um dos funcionários,
    //    de um "Delay" para representar o tempo de atendimento por um funcionário
    //    de um "Release" para liberar um funcionário previamente alocado, e
    //    de um "Dispose" para retirar os clientes (entidades) do sistema
    Create* create1 = plugins->newInstance<Create>(model, "Clientes chegam");
    Seize* seize1 = plugins->newInstance<Seize>(model, "São atendidos por um dos 5 funcionários");
    Delay* delay1 = plugins->newInstance<Delay>(model, "Tempo de Atendimento");
    Release* release1 = plugins->newInstance<Release>(model, "Liberam o funcionário");
    Dispose* dispose1 = plugins->newInstance<Dispose>(model, "Saem do sistema");
    //
    // Depois dos componentes criados, os conectamos, formando um "processo"
    //  do conceito de "modelagem orientada ao processo", ou seja, um "fluxograma" por
    //  onde a entidade se movimentará, realizando ações (disparando eventos)
    create1->getConnectionManager()->insert(seize1);
    seize1->getConnectionManager()->insert(delay1);
    delay1->getConnectionManager()->insert(release1);
    release1->getConnectionManager()->insert(dispose1);
    //
    // Concluindo a modelagem, preenchemos os parâmetros de cada componente.
    // Corresponde a dar um duplo clique no componente que arrastamos para a área
    //   de modelagem e preencher os parâmetros que aparecem na caixa de diálogo
    //
    // Clientes chegam na loja (tempo entre chegadas expo(4) min.),
    create1->setEntityTypeName("Cliente");
    create1->setTimeBetweenCreationsExpression("expo(4)", Util::TimeUnit::minute);
    //
    // são atendidos por um dos 5 funcionários
    //    cria um "Set" de funcionários
    Set *setFunc = plugins->newInstance<Set>(model, "SetFuncionarios");
    //    inclui os 5 recursos ao Set de funcionários
    setFunc->addElementSet(plugins->newInstance<Resource>(model, "Funcionario1"));
    setFunc->addElementSet(plugins->newInstance<Resource>(model, "Funcionario2"));
    setFunc->addElementSet(plugins->newInstance<Resource>(model, "Funcionario3"));
    setFunc->addElementSet(plugins->newInstance<Resource>(model, "Funcionario4"));
    setFunc->addElementSet(plugins->newInstance<Resource>(model, "Funcionario5"));
    //    e no "seize", manda alocar "1" item do Set "SetFuncionarios", alocando o
    //       recurso que estiver menos ocupado, e salvando o índice desse recurso em "meuFuncionario"
    //       (exatamente os mesmos parâmetros que seriam dados no Arena)
    seize1->getSeizeRequests()->insert(new SeizableItem(setFunc, "1", SeizableItem::SelectionRule::SMALLESTNUMBERBUSY, "meuFuncionario"));
    seize1->setQueueableItem(new QueueableItem(model, "QueueEsperaFuncionario"));
    //
    //(tempo de atendimento norm(20,10) min.)
    delay1->setDelayExpression("norm(20,10)");
    delay1->setDelayTimeUnit(Util::TimeUnit::minute);
    //
    // depois de atendido, o cliente libera o funcionário que o atendeu
    release1->getReleaseRequests()->insert(new SeizableItem(setFunc, "1", SeizableItem::SelectionRule::SPECIFICMEMBER, "meuFuncionario"));
    //
    // Depois do modelo estar pronto, ajusta as informações sobre a experimentação,
    //   ou seja, sobre a simulação do modelo.
    ModelSimulation* sim = model->getSimulation();
    sim->setReplicationLength(8);
    sim->setReplicationLengthTimeUnit(Util::TimeUnit::hour);
    sim->setNumberOfReplications(10);
    //
    // Não é necessário, mas pode salvar o modelo em arquivo. Aí já temos o
    //    modelo em linguagem de simulação do GenESyS, se a extensão GEN for escolhida
    model->save("./Loja01.gen");
    //
    // Por fim, simula o modelo
    sim->start();
    delete genesys;
    return 0;
}

