#ifndef GENESYS_PARSER_FUNCTION_CATALOG_H
#define GENESYS_PARSER_FUNCTION_CATALOG_H

#include <QObject>
#include <QString>
#include <QVector>

// Shared, GUI-local discovery metadata. The Bison/Flex grammar remains the
// authoritative source of syntactic acceptance. Token-only and stub functions
// are intentionally excluded until their runtime contract is implemented.
struct ParserFunctionInfo {
    QString category;
    QString name;
    QString syntax;
    QString description;
    QString example;
};

inline QVector<ParserFunctionInfo> parserFunctionCatalog() {
    return {
        {QObject::tr("Arithmetic"), QStringLiteral("round"), QStringLiteral("round(expression)"), QObject::tr("Rounds a numeric expression to the nearest integer."), QStringLiteral("round(3.7)")},
        {QObject::tr("Arithmetic"), QStringLiteral("trunc"), QStringLiteral("trunc(expression)"), QObject::tr("Removes the fractional part of a numeric expression."), QStringLiteral("trunc(3.7)")},
        {QObject::tr("Arithmetic"), QStringLiteral("frac"), QStringLiteral("frac(expression)"), QObject::tr("Returns only the fractional part of a numeric expression."), QStringLiteral("frac(3.7)")},
        {QObject::tr("Arithmetic"), QStringLiteral("mod"), QStringLiteral("mod(expression, expression)"), QObject::tr("Integer remainder of the first expression divided by the second."), QStringLiteral("mod(17, 5)")},
        {QObject::tr("Arithmetic"), QStringLiteral("min"), QStringLiteral("min(expression, expression)"), QObject::tr("Returns the smaller of two numeric expressions."), QStringLiteral("min(10, 4)")},
        {QObject::tr("Arithmetic"), QStringLiteral("max"), QStringLiteral("max(expression, expression)"), QObject::tr("Returns the larger of two numeric expressions."), QStringLiteral("max(10, 4)")},
        {QObject::tr("Math"), QStringLiteral("exp"), QStringLiteral("exp(expression)"), QObject::tr("Natural exponential function."), QStringLiteral("exp(1)")},
        {QObject::tr("Math"), QStringLiteral("sqrt"), QStringLiteral("sqrt(expression)"), QObject::tr("Square root of a numeric expression."), QStringLiteral("sqrt(16)")},
        {QObject::tr("Math"), QStringLiteral("log"), QStringLiteral("log(expression)"), QObject::tr("Base-10 logarithm."), QStringLiteral("log(100)")},
        {QObject::tr("Math"), QStringLiteral("ln"), QStringLiteral("ln(expression)"), QObject::tr("Natural logarithm."), QStringLiteral("ln(exp(1))")},
        {QObject::tr("Trigonometry"), QStringLiteral("sin"), QStringLiteral("sin(expression)"), QObject::tr("Sine of an angle in radians."), QStringLiteral("sin(0)")},
        {QObject::tr("Trigonometry"), QStringLiteral("cos"), QStringLiteral("cos(expression)"), QObject::tr("Cosine of an angle in radians."), QStringLiteral("cos(0)")},
        {QObject::tr("Logic"), QStringLiteral("if"), QStringLiteral("if(condition, trueExpression, falseExpression)"), QObject::tr("Evaluates one expression when condition is non-zero and another when condition is zero."), QStringLiteral("if(2 > 1, 10, 20)")},
        {QObject::tr("Logic"), QStringLiteral("if"), QStringLiteral("if(condition, trueExpression)"), QObject::tr("Returns the true expression when condition is non-zero, otherwise returns zero."), QStringLiteral("if(1, 42)")},
        {QObject::tr("Logic"), QStringLiteral("operators"), QStringLiteral("<, >, <=, >=, ==, <>, !=, and, or, nand, xor, not"), QObject::tr("Relational and logical operators return numeric booleans, 1 for true and 0 for false."), QStringLiteral("(2 <= 3) and not false")},
        {QObject::tr("Random"), QStringLiteral("rnd"), QStringLiteral("rnd"), QObject::tr("Uniform random value in [0, 1]."), QStringLiteral("rnd")},
        {QObject::tr("Random"), QStringLiteral("expo"), QStringLiteral("expo(mean)"), QObject::tr("Exponential random variate using the configured sampler."), QStringLiteral("expo(5)")},
        {QObject::tr("Random"), QStringLiteral("norm"), QStringLiteral("norm(mean, stddev)"), QObject::tr("Normal random variate."), QStringLiteral("norm(10, 2)")},
        {QObject::tr("Random"), QStringLiteral("unif"), QStringLiteral("unif(min, max)"), QObject::tr("Uniform random variate between min and max."), QStringLiteral("unif(1, 10)")},
        {QObject::tr("Random"), QStringLiteral("weib"), QStringLiteral("weib(shape, scale)"), QObject::tr("Weibull random variate."), QStringLiteral("weib(2, 10)")},
        {QObject::tr("Random"), QStringLiteral("logn"), QStringLiteral("logn(mean, stddev)"), QObject::tr("Lognormal random variate."), QStringLiteral("logn(1, 0.25)")},
        {QObject::tr("Random"), QStringLiteral("gamm"), QStringLiteral("gamm(shape, scale)"), QObject::tr("Gamma random variate."), QStringLiteral("gamm(2, 3)")},
        {QObject::tr("Random"), QStringLiteral("erla"), QStringLiteral("erla(mean, m)"), QObject::tr("Erlang random variate."), QStringLiteral("erla(8, 2)")},
        {QObject::tr("Random"), QStringLiteral("tria"), QStringLiteral("tria(min, mode, max)"), QObject::tr("Triangular random variate."), QStringLiteral("tria(1, 3, 8)")},
        {QObject::tr("Random"), QStringLiteral("beta"), QStringLiteral("beta(alpha, beta, infLimit, supLimit)"), QObject::tr("Scaled beta random variate."), QStringLiteral("beta(2, 5, 0, 1)")},
        {QObject::tr("Simulation"), QStringLiteral("tnow"), QStringLiteral("tnow"), QObject::tr("Current simulated time."), QStringLiteral("tnow")},
        {QObject::tr("Simulation"), QStringLiteral("tfin"), QStringLiteral("tfin"), QObject::tr("Configured replication length."), QStringLiteral("tfin")},
        {QObject::tr("Simulation"), QStringLiteral("maxrep"), QStringLiteral("maxrep"), QObject::tr("Configured number of replications."), QStringLiteral("maxrep")},
        {QObject::tr("Simulation"), QStringLiteral("numrep"), QStringLiteral("numrep"), QObject::tr("Current replication number."), QStringLiteral("numrep")},
        {QObject::tr("Simulation"), QStringLiteral("ident"), QStringLiteral("ident"), QObject::tr("Current entity id, when an event/entity context exists."), QStringLiteral("ident")},
        {QObject::tr("Simulation"), QStringLiteral("entitieswip"), QStringLiteral("entitieswip"), QObject::tr("Number of entity data definitions in the model."), QStringLiteral("entitieswip")},
        {QObject::tr("Collectors"), QStringLiteral("tavg"), QStringLiteral("tavg(statisticsCollector)"), QObject::tr("Time-average value of a StatisticsCollector."), QStringLiteral("tavg(MyCollector)")},
        {QObject::tr("Collectors"), QStringLiteral("count"), QStringLiteral("count(counter)"), QObject::tr("Current value of a Counter."), QStringLiteral("count(MyCounter)")},
        {QObject::tr("Queue"), QStringLiteral("nq"), QStringLiteral("nq(queue)"), QObject::tr("Number of entities waiting in a queue."), QStringLiteral("nq(MyQueue)")},
        {QObject::tr("Queue"), QStringLiteral("firstinq"), QStringLiteral("firstinq(queue)"), QObject::tr("Id of the first entity waiting in a queue, or zero when empty."), QStringLiteral("firstinq(MyQueue)")},
        {QObject::tr("Queue"), QStringLiteral("saque"), QStringLiteral("saque(queue, attribute)"), QObject::tr("Sum of an attribute over entities waiting in a queue."), QStringLiteral("saque(MyQueue, MyAttribute)")},
        {QObject::tr("Queue"), QStringLiteral("aque"), QStringLiteral("aque(queue, rank, attribute)"), QObject::tr("Attribute value of an entity at a queue rank."), QStringLiteral("aque(MyQueue, 1, MyAttribute)")},
        {QObject::tr("Resource"), QStringLiteral("mr"), QStringLiteral("mr(resource)"), QObject::tr("Resource capacity."), QStringLiteral("mr(MyResource)")},
        {QObject::tr("Resource"), QStringLiteral("nr"), QStringLiteral("nr(resource)"), QObject::tr("Number of busy units in a resource."), QStringLiteral("nr(MyResource)")},
        {QObject::tr("Resource"), QStringLiteral("state"), QStringLiteral("state(resource)"), QObject::tr("Numeric state of a resource."), QStringLiteral("state(MyResource)")},
        {QObject::tr("Resource"), QStringLiteral("irf"), QStringLiteral("irf(resource)"), QObject::tr("Returns 1 when a resource is failed, otherwise 0."), QStringLiteral("irf(MyResource)")},
        {QObject::tr("Resource"), QStringLiteral("setsum"), QStringLiteral("setsum(set)"), QObject::tr("Counts busy resources in a set."), QStringLiteral("setsum(MySet)")},
        {QObject::tr("Set"), QStringLiteral("numset"), QStringLiteral("numset(set)"), QObject::tr("Number of model data definitions in a set."), QStringLiteral("numset(MySet)")}
    };
}

#endif // GENESYS_PARSER_FUNCTION_CATALOG_H
