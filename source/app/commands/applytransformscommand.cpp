#include "applytransformscommand.h"

#include <QObject>

#include "graph/graphmodel.h"
#include "ui/selectionmanager.h"
#include "ui/document.h"

ApplyTransformsCommand::ApplyTransformsCommand(GraphModel* graphModel,
                                               SelectionManager* selectionManager, Document* document,
                                               QStringList previousTransformations,
                                               QStringList transformations) :
    _graphModel(graphModel),
    _selectionManager(selectionManager),
    _document(document),
    _previousTransformations(std::move(previousTransformations)),
    _transformations(std::move(transformations)),
    _selectedNodeIds(_selectionManager->selectedNodes())
{}

QString ApplyTransformsCommand::description() const
{
    return QObject::tr("Apply Transforms");
}

QString ApplyTransformsCommand::verb() const
{
    return QObject::tr("Applying Transforms");
}

void ApplyTransformsCommand::doTransform(const QStringList& transformations, const QStringList& previousTransformations)
{
    _graphModel->buildTransforms(transformations, this);

    _document->executeOnMainThreadAndWait(
    [this, newTransformations = cancelled() ? previousTransformations : transformations]
    {
        _document->setTransforms(newTransformations);
    }, QStringLiteral("setTransforms"));
}

bool ApplyTransformsCommand::execute()
{
    doTransform(_transformations, _previousTransformations);
    return true;
}

void ApplyTransformsCommand::undo()
{
    doTransform(_previousTransformations, _transformations);

    // Restore the selection to what it was prior to the transformation
    _selectionManager->selectNodes(_selectedNodeIds);
}

void ApplyTransformsCommand::cancel()
{
    ICommand::cancel();

    _graphModel->cancelTransformBuild();
}
