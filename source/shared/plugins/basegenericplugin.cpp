#include "basegenericplugin.h"

#include "shared/loading/gmlfileparser.h"
#include "shared/loading/pairwisetxtfileparser.h"
#include "shared/loading/graphmlparser.h"

BaseGenericPluginInstance::BaseGenericPluginInstance() :
    _userNodeDataTableModel(&_userNodeData)
{
    connect(this, SIGNAL(loadSuccess()), this, SLOT(onLoadSuccess()));
    connect(this, SIGNAL(selectionChanged(const ISelectionManager*)),
            this, SLOT(onSelectionChanged(const ISelectionManager*)));
}

void BaseGenericPluginInstance::initialise(IGraphModel* graphModel, ISelectionManager* selectionManager,
                                           ICommandManager* commandManager, const IParserThread* parserThread)
{
    BasePluginInstance::initialise(graphModel, selectionManager, commandManager, parserThread);

    _userNodeData.initialise(graphModel->mutableGraph());
    _userNodeDataTableModel.initialise(selectionManager);
}

std::unique_ptr<IParser> BaseGenericPluginInstance::parserForUrlTypeName(const QString& urlTypeName)
{
    if(urlTypeName == "GML")
        return std::make_unique<GmlFileParser>(&_userNodeData);
    else if(urlTypeName == "PairwiseTXT")
        return std::make_unique<PairwiseTxtFileParser>(this, &_userNodeData);
    else if(urlTypeName == "GraphML")
        return std::make_unique<GraphMLParser>(&_userNodeData);

    return nullptr;
}

void BaseGenericPluginInstance::setEdgeWeight(EdgeId edgeId, float weight)
{
    if(_edgeWeights == nullptr)
    {
        _edgeWeights = std::make_unique<EdgeArray<float>>(graphModel()->mutableGraph());

        graphModel()->createAttribute(tr("Edge Weight"))
            .setFloatValueFn([this](EdgeId edgeId_) { return _edgeWeights->get(edgeId_); })
            .setFlag(AttributeFlag::AutoRangeMutable)
            .setDescription(tr("The Edge Weight is a generic value associated with the edge."));
    }

    _edgeWeights->set(edgeId, weight);
}

QString BaseGenericPluginInstance::selectedNodeNames() const
{
    QString s;

    for(auto nodeId : selectionManager()->selectedNodes())
    {
        if(!s.isEmpty())
            s += ", ";

        s += graphModel()->nodeName(nodeId);
    }

    return s;
}

void BaseGenericPluginInstance::onLoadSuccess()
{
    _userNodeData.setNodeNamesToFirstUserDataVector(*graphModel());
    _userNodeData.exposeAsAttributes(*graphModel());
}

void BaseGenericPluginInstance::onSelectionChanged(const ISelectionManager*)
{
    emit selectedNodeNamesChanged();
    _userNodeDataTableModel.onSelectionChanged();
}

BaseGenericPlugin::BaseGenericPlugin()
{
    registerUrlType("GML", QObject::tr("GML File"), QObject::tr("GML Files"), {"gml"});
    registerUrlType("PairwiseTXT", QObject::tr("Pairwise Text File"), QObject::tr("Pairwise Text Files"), {"txt", "layout"});
    registerUrlType("GraphML", QObject::tr("GraphML File"), QObject::tr("GraphML Files"), {"graphml"});

}

QStringList BaseGenericPlugin::identifyUrl(const QUrl& url) const
{
    //FIXME actually look at the file contents
    return identifyByExtension(url);
}
