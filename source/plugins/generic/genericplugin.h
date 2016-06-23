#ifndef GENERICPLUGIN_H
#define GENERICPLUGIN_H

#include "shared/plugins/basegenericplugin.h"

class GenericPluginInstance : public BaseGenericPluginInstance
{
    Q_OBJECT
};

class GenericPlugin : public BaseGenericPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID IPluginIID FILE "genericplugin.json")

public:
    QString name() const { return "Generic"; }
    QString description() const
    {
        return tr("A plugin that loads generic graphs from a variety "
                  "of file formats.");
    }
    QString imageSource() const { return "qrc:///tools.svg"; }

    std::unique_ptr<IPluginInstance> createInstance();

    QString qmlPath() const { return "qrc:///qml/genericplugin.qml"; }
};

#endif // GENERICPLUGIN_H
