#include "application.h"

#include "loading/parserthread.h"
#include "shared/interfaces/iplugin.h"
#include "shared/loading/iparser.h"

#include "shared/utils/utils.h"

#include <QPluginLoader>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>

#include <memory>
#include <cmath>

const char* Application::_uri = "com.kajeka";

Application::Application(QObject *parent) :
    QObject(parent),
    _pluginDetails(&_plugins)
{
    loadPlugins();
}

IPlugin* Application::pluginForUrlTypeName(const QString& urlTypeName) const
{
    std::vector<IPlugin*> viablePlugins;

    for(auto plugin : _plugins)
    {
        auto urlTypeNames = plugin->loadableUrlTypeNames();
        bool willLoad = std::any_of(urlTypeNames.begin(), urlTypeNames.end(),
        [&urlTypeName](const QString& loadableUrlTypeName)
        {
            return loadableUrlTypeName.compare(urlTypeName) == 0;
        });

        if(willLoad)
            viablePlugins.push_back(plugin);
    }

    if(viablePlugins.size() == 0)
        return nullptr;

    //FIXME: Allow the user to choose which plugin to use if there is more than 1
    Q_ASSERT(viablePlugins.size() == 1);

    auto* plugin = viablePlugins.at(0);

    return plugin;

}

bool Application::canOpen(const QString& urlTypeName) const
{
    return std::any_of(_plugins.begin(), _plugins.end(),
    [&urlTypeName](auto plugin)
    {
        return plugin->loadableUrlTypeNames().contains(urlTypeName);
    });
}

bool Application::canOpenAnyOf(const QStringList& urlTypeNames) const
{
    return std::any_of(urlTypeNames.begin(), urlTypeNames.end(),
    [this](const QString& urlTypeName)
    {
        return canOpen(urlTypeName);
    });
}

QStringList Application::urlTypesOf(const QUrl& url) const
{
    QStringList urlTypeNames;

    for(auto plugin : _plugins)
        urlTypeNames.append(plugin->identifyUrl(url));

    urlTypeNames.removeDuplicates();

    return urlTypeNames;
}

void Application::loadPlugins()
{
    std::vector<QString> pluginsDirs =
    {
        qApp->applicationDirPath() + "/plugins",
        QStandardPaths::writableLocation(
            QStandardPaths::StandardLocation::AppDataLocation) + "/plugins"
    };

#ifdef Q_OS_MAC
    QDir dotAppDir(qApp->applicationDirPath());

    // Within the bundle itself
    dotAppDir.cdUp();
    pluginsDirs.emplace_back(dotAppDir.absolutePath() + "/PlugIns");

    // Adjacent to the .app file
    dotAppDir.cdUp();
    dotAppDir.cdUp();
    pluginsDirs.emplace_back(dotAppDir.absolutePath() + "/plugins");
#endif

    for(auto& pluginsDir : pluginsDirs)
    {
        if(pluginsDir.isEmpty() || !QDir(pluginsDir).exists())
            continue;

        QDir pluginsQDir(pluginsDir);

        for(auto& fileName : pluginsQDir.entryList(QDir::Files))
        {
            QPluginLoader pluginLoader(pluginsQDir.absoluteFilePath(fileName));
            QObject* plugin = pluginLoader.instance();
            if(!pluginLoader.isLoaded())
                qDebug() << pluginLoader.errorString();

            if(plugin)
            {
                auto* iplugin = qobject_cast<IPlugin*>(plugin);
                if(iplugin)
                {
                    bool pluginNameAlreadyUsed = std::any_of(_plugins.begin(), _plugins.end(),
                    [pluginName = iplugin->name()](auto loadedPlugin)
                    {
                        return loadedPlugin->name().compare(pluginName, Qt::CaseInsensitive) == 0;
                    });

                    if(pluginNameAlreadyUsed)
                    {
                        qDebug() << "WARNING: not loading plugin" << iplugin->name() <<
                                    "as a plugin of the same name is already loaded";
                        pluginLoader.unload();
                        continue;
                    }

                    initialisePlugin(iplugin);
                }
            }
        }
    }

    updateNameFilters();
}

void Application::initialisePlugin(IPlugin* plugin)
{
    _plugins.push_back(plugin);
    emit pluginDetailsChanged();
}

void Application::updateNameFilters()
{
    struct FileType
    {
        QString _collectiveDescription;
        QStringList _extensions;
    };

    std::vector<FileType> fileTypes;

    for(auto plugin : _plugins)
    {
        for(auto& urlTypeName : plugin->loadableUrlTypeNames())
        {
            FileType fileType = {plugin->collectiveDescriptionForUrlTypeName(urlTypeName),
                                 plugin->extensionsForUrlTypeName(urlTypeName)};
            fileTypes.emplace_back(fileType);
        }
    }

    // Sort by collective description
    std::sort(fileTypes.begin(), fileTypes.end(),
    [](const auto& a, const auto& b)
    {
        return a._collectiveDescription.compare(b._collectiveDescription, Qt::CaseInsensitive) < 0;
    });

    QString description = QObject::tr("All Files (");
    bool second = false;

    for(auto fileType : fileTypes)
    {
        for(auto extension : fileType._extensions)
        {
            if(second)
                description += " ";
            else
                second = true;

            description += "*." + extension;
        }
    }

    description += ")";

    _nameFilters.clear();
    _nameFilters.append(description);

    for(auto fileType : fileTypes)
    {
        description = fileType._collectiveDescription + " (";

        for(auto extension : fileType._extensions)
            description += "*." + extension;

        description += ")";

        _nameFilters.append(description);
    }

    emit nameFiltersChanged();
}

QAbstractListModel* Application::pluginDetails()
{
    return &_pluginDetails;
}

int PluginDetailsModel::rowCount(const QModelIndex&) const
{
    return static_cast<int>(_plugins->size());
}

QVariant PluginDetailsModel::data(const QModelIndex& index, int role) const
{
    int row = index.row();

    if(row < 0 || row >= rowCount(index))
        return QVariant(QVariant::Invalid);

    auto* plugin = _plugins->at(row);

    switch(role)
    {
    case Name:
        return plugin->name();

    case Description:
    {
        QString urlTypes;
        for(auto& loadbleUrlTypeName : plugin->loadableUrlTypeNames())
        {
            if(!urlTypes.isEmpty())
                urlTypes += tr(", ");

            urlTypes += plugin->collectiveDescriptionForUrlTypeName(loadbleUrlTypeName);
        }

        if(urlTypes.isEmpty())
            urlTypes = tr("None");

        return QString(tr("%1\n\nSupported data types: %2"))
                .arg(plugin->description()).arg(urlTypes);
    }

    case ImageSource:
        return plugin->imageSource();

    default:
        break;
    }

    return QVariant(QVariant::Invalid);
}

QHash<int, QByteArray> PluginDetailsModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[Name] = "name";
    roles[Description] = "description";
    roles[ImageSource] = "imageSource";
    return roles;
}
