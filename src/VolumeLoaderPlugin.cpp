

#include "HdvolLoaderPlugin.h"

#include "PointData.h"

#include "Set.h"
#include<iostream> 

#include<fstream> 
#include <QtCore>
#include <QtDebug>
#include <QFileDialog>
#include <qmessagebox.h>

#include <random>
#include <windows.h>
#include <vector>
#include <algorithm>



Q_PLUGIN_METADATA(IID "nl.tudelft.HdvolLoaderPlugin")

using namespace hdps;

// =============================================================================
// View
// =============================================================================

HdvolLoaderPlugin::~HdvolLoaderPlugin(void)
{
    
}

/**
 * Mandatory plugin override function. Any initial state can be set here.
 * This function gets called when an instance of the plugin is created.
 * In this case when someone select the loader option from the menu.
 */
void HdvolLoaderPlugin::init()
{

}

/**
 * Funtion the loads in the data and transforms it from its file type to pointsdata. 
 * It also fills in missing data in the input volume with 0 values (needed for visualization)
 * and scales all non-missing data from 1-100 in order to properly be able to use color maps 
 * without running into the issue of values inside the object having the same value as the missing
 * data values. The scaling also helps create a general colormap the works for all data.
 */
void HdvolLoaderPlugin::loadData()
{
    
    
    
    
    QString filePath= QFileDialog::getOpenFileName(nullptr, "Open a .hdvol file", "C:/ "); // Open the file selector
    

    std::string hdvol = ".hdvol"; 

    int found = filePath.toStdString().find(".hdvol");
    

    if (found == -1) {
        QMessageBox messageBox;
        messageBox.critical(0, "Error", "File: " + filePath + " is not of type .hdvol"); // Throws error if file format is wrong
        messageBox.setFixedSize(500, 200);
    }
    else {
        std::string base_filename = filePath.toStdString().substr(filePath.toStdString().find_last_of("/\\") + 1);
        std::string::size_type const p(base_filename.find_last_of('.'));
        std::string fileName = base_filename.substr(0, p);
        

        QString name = _core->addData("Points", QString::fromStdString(fileName)); // create a datafile in the hdps core
        Points& points = _core->requestData<Points>(name); // get the created datafile

        int a;
        float b;
        std::ifstream file(filePath.toStdString(), std::ios::in | std::ios::binary);
        file.read((char*)&a, sizeof(a)); // read the number of points as stated in the hdvol metadata (this excludes missing data)
        int numPoints = a;
        file.read((char*)&a, sizeof(a)); // read the number of dimensions stated in the hdvol metadata
        int numDimensions = a;

        std::vector<int16_t> dimensions(6); // creates vector for holding hdvol size data
        

        for (int i = 0; i < 6;i++) {
            file.read((char*)&a, sizeof(a));
            dimensions[i] = a; // reads the minimum and maximum xyz values from hdvol metadata in the following order. xMin,xMax,yMin,yMax,zMin,zMax
        }

        std::vector<int16_t> coordinates(3);// empty vector to hold coordinate data for current point in loop to be placed in dataVector

        int xSize = dimensions[1] - dimensions[0];// calculate xSize
        int ySize = dimensions[3] - dimensions[2];// calculate ySize
        int zSize = dimensions[5] - dimensions[4];// calculate zSize

        // creates a 4D dataVector of size (xSize,ySize,zSize,numDimensions) with inial values 0
        std::vector<std::vector<std::vector<std::vector<float>>>> dataArray(xSize, std::vector<std::vector<std::vector<float>>>(ySize, std::vector<std::vector<float>>(zSize,std::vector<float>(numDimensions,0))));
        // creates a 4D binary objectMask vector used for resetting non-Object points (so missing data) back to 0 after scaling has been performed
        std::vector<std::vector<std::vector<std::vector<float>>>> dataMask(xSize, std::vector<std::vector<std::vector<float>>>(ySize, std::vector<std::vector<float>>(zSize, std::vector<float>(numDimensions, 0))));
        
        // creates a 1D vector used to read the completed dataset into the hdps points datatype
        std::vector<float> dataSet((xSize * ySize*zSize* numDimensions));
        
        // creates 1D vectors of size numDimensions that are used to contain the minimum and maximum values in each dimension for scaling purposes
        std::vector<float> dimensionMinimum(numDimensions, 0);
        std::vector<float> dimensionMaximum(numDimensions, 0);

        int zCor;
        int yCor;
        int xCor;
        int index=0;
        int dataIterator;
        
        
        for (int k = 0; k < numPoints; k++) { // loop through all the datapoint specified in the hdvol file
            for (int t = 0; t < 3; t++) { // loop through xyz values of current datapoint
                file.read((char*)&a, 4);
                coordinates[t] = a; // Read coordinates off current datapoint
                
            }
            xCor = coordinates[0] - 1;
            yCor = coordinates[1] - 1;
            zCor = coordinates[2] - 1;
            for (int i = 0; i < numDimensions; i++) // loop through all dimensions of current datapoint
            {   
                file.read((char*)&b, 4);

                dataArray[xCor][yCor][zCor][i] = b; // read current dimension of current datapoint into the dataVector
                dataMask[xCor][yCor][zCor][i] = 1; // set the current datapoint to 1 for non-missing data
                
                if (k == 0) {// if its the firs itteration set initial minimum
                    dimensionMinimum[i] = b; 
                }
                if (b < dimensionMinimum[i]) { // otherwise only change the minimum if the current value is lower than the lowest value currently stored
                    dimensionMinimum[i] = b;
                }
                if (k == 0) {// if its the firs itteration set initial maximum
                    dimensionMaximum[i] = b;
                }
                if (b > dimensionMaximum[i]) {// otherwise only change the maximum if the current value is lower than the highest value currently stored
                    dimensionMaximum[i] = b;
                }
            }
        }

        //------------------------------------------------------------------------- Can probably be merged, do if time left -----------------------------------------------------
        //loop through the dataArray to scale all the datapoints from 1-100 for non-missing data and 0 for missing data
        for (int z = 0; z < zSize; z++) {
            for (int y = 0; y < ySize; y++) {
                for (int x = 0; x < xSize; x++) {
                    for (int dim = 0; dim < numDimensions; dim++) {
                        
                        /**calculate the scaling with the following formula if desired range is [a,b]
                        * 
                        * x_normalized = ((b-a)* ((x-min(x))/(max(x)-min(x))))+a
                        * 
                        * Then multiply with the objectMask in order to make the missing data 0 again
                        */

                        dataArray[x][y][z][dim] = (((100-1)*((dataArray[x][y][z][dim] - dimensionMinimum[dim])/ (dimensionMaximum[dim] - dimensionMinimum[dim])))+1)*dataMask[x][y][z][dim];
                        
                    }
                }
            }
        }
        
    
        int iterator = 0;
        
        index = 0;

        
        // loop through data again to read the dataArray into 1D vector 
        for (int z = 0; z < zSize; z++) { 
            for (int y = 0; y < ySize; y++) {
                for (int x = 0; x < xSize; x++) {
                    for (int dim=0; dim < numDimensions; dim++) {
                        dataSet[iterator]=dataArray[x][y][z][dim];//Read out 4D vector, convert to 1D vector                       
                        
                        iterator++;
                    }
                }
            }
        }
        //------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        file.close(); // stop reading the file
        
       
   
        
        points.setData(dataSet.data(), xSize*ySize*zSize, numDimensions);// pass 1D vector to points data in core
        
        
        //declare the x,y and z sizes of the data which is needed to create the imagedata object in the 3D viewer
        points.setProperty("xDim", xSize);
        points.setProperty("yDim", ySize);
        points.setProperty("zDim", zSize);
       

        // Notify the core system of the new data
        _core->notifyDataAdded(name);

        

    }
    
}

HdvolLoaderPlugin* HdvolLoaderPluginFactory::produce()
{
    return new HdvolLoaderPlugin(this);
}

hdps::DataTypes HdvolLoaderPluginFactory::supportedDataTypes() const
{
    DataTypes supportedTypes;
    supportedTypes.append(PointType);
    return supportedTypes;
}
