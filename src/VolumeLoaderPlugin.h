#pragma once

#include <LoaderPlugin.h>

using namespace mv::plugin;


// =============================================================================
// Loader
// =============================================================================

class VolumeLoaderPlugin : public LoaderPlugin
{
    Q_OBJECT
public:
    VolumeLoaderPlugin(const PluginFactory* factory) : LoaderPlugin(factory) { }
    ~VolumeLoaderPlugin(void) override;
    
    void init() override;

    void loadData() Q_DECL_OVERRIDE;

private:
    unsigned int _numDimensions;

    QString _dataSetName;
};


// =============================================================================
// Factory
// =============================================================================

class VolumeLoaderPluginFactory : public LoaderPluginFactory
{
    Q_INTERFACES(mv::plugin::LoaderPluginFactory mv::plugin::PluginFactory)
    Q_OBJECT
    Q_PLUGIN_METADATA(IID   "nl.tudelft.VolumeLoaderPlugin"
                      FILE  "VolumeLoaderPlugin.json")
    
public:
    VolumeLoaderPluginFactory(void) {}
    ~VolumeLoaderPluginFactory(void) override {}

    VolumeLoaderPlugin* produce() override;

    mv::DataTypes supportedDataTypes() const override;
};
