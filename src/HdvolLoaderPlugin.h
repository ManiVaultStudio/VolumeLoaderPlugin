#pragma once

#include <LoaderPlugin.h>

using namespace hdps::plugin;

// =============================================================================
// Loader
// =============================================================================

class HdvolLoaderPlugin : public QObject, public LoaderPlugin
{
    Q_OBJECT
public:
    HdvolLoaderPlugin(const PluginFactory* factory) : LoaderPlugin(factory) { }
    ~HdvolLoaderPlugin(void) override;
    
    void init() override;

    void loadData() Q_DECL_OVERRIDE;

private:
    unsigned int _numDimensions;

    QString _dataSetName;
};


// =============================================================================
// Factory
// =============================================================================

class HdvolLoaderPluginFactory : public LoaderPluginFactory
{
    Q_INTERFACES(hdps::plugin::LoaderPluginFactory hdps::plugin::PluginFactory)
    Q_OBJECT
    Q_PLUGIN_METADATA(IID   "nl.tudelft.HdvolLoaderPlugin"
                      FILE  "HdvolLoaderPlugin.json")
    
public:
    HdvolLoaderPluginFactory(void) {}
    ~HdvolLoaderPluginFactory(void) override {}

    HdvolLoaderPlugin* produce() override;

    hdps::DataTypes supportedDataTypes() const override;
};
