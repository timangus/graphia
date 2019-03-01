#include "separatebyattributetransform.h"
#include "transform/transformedgraph.h"
#include "attributes/conditionfncreator.h"
#include "graph/graphmodel.h"

#include "shared/utils/string.h"

#include <QObject>

void SeparateByAttributeTransform::apply(TransformedGraph& target) const
{
    target.setPhase(QObject::tr("Contracting"));

    if(config().attributeNames().empty())
    {
        addAlert(AlertType::Error, QObject::tr("Invalid parameter"));
        return;
    }

    auto attributeName = config().attributeNames().front();
    auto attribute = _graphModel->attributeValueByName(attributeName);

    GraphTransformConfig::TerminalCondition condition
    {
        QStringLiteral("$source.%1").arg(attributeName),
        ConditionFnOp::Equality::NotEqual,
        QStringLiteral("$target.%1").arg(attributeName),
    };

    auto conditionFn = CreateConditionFnFor::edge(*_graphModel, condition);
    if(conditionFn == nullptr)
    {
        addAlert(AlertType::Error, QObject::tr("Invalid condition"));
        return;
    }

    EdgeIdSet edgeIdsToRemove;

    for(auto edgeId : target.edgeIds())
    {
        if(conditionFn(edgeId))
            edgeIdsToRemove.insert(edgeId);
    }

    target.mutableGraph().removeEdges(edgeIdsToRemove);
}

std::unique_ptr<GraphTransform> SeparateByAttributeTransformFactory::create(const GraphTransformConfig&) const
{
    return std::make_unique<SeparateByAttributeTransform>(*graphModel());
}
