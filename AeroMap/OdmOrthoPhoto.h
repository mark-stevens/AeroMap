#ifndef ODMORTHOPHOTO_H
#define ODMORTHOPHOTO_H

// C++
#include <limits.h>
#include <istream>
#include <ostream>

// PCL
//#include <pcl/io/obj_io.h>
//#include <pcl/common/transforms.h>

// OpenCV
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

// PCL
//#include <pcl/common/eigen.h>
//#include <pcl/common/common.h>

// OpenCV
#include <opencv2/core/core.hpp>

// GDAL
#include "gdal_priv.h"
#include "cpl_conv.h" // for CPLMalloc()

#include "AeroMap.h"

struct Bounds
{
    float xMin;
    float xMax;
    float yMin;
    float yMax;

    Bounds() : xMin(0), xMax(0), yMin(0), yMax(0) {}
    Bounds(float xMin, float xMax, float yMin, float yMax) :
        xMin(xMin), xMax(xMax), yMin(yMin), yMax(yMax){}
    Bounds(const Bounds &b)
    {
        xMin = b.xMin;
        xMax = b.xMax;
        yMin = b.yMin;
        yMax = b.yMax;
    }
};

class OdmOrthoPhoto
{
public:
    OdmOrthoPhoto();
    ~OdmOrthoPhoto();

    int CreateOrthoPhoto();

    void SetInputFileList(std::vector<XString> inputFiles);

private:

    int width, height;
    //void parseArguments(int argc, char* argv[]);
    //void printHelp();

    //Bounds computeBoundsForModel(const pcl::TextureMesh &mesh);
    
    // Creates a transformation which aligns the area for the orthophoto.
    Eigen::Transform<float, 3, Eigen::Affine> getROITransform(float xMin, float yMin) const;

    template <typename T>
    void initBands(int count);

    template <typename T>
    void initAlphaBand();

    template <typename T>
    void finalizeAlphaBand();

    void saveTIFF(const std::string &filename, GDALDataType dataType);
    
    //template <typename T>
    //void drawTexturedTriangle(const cv::Mat &texture, const pcl::Vertices &polygon, const pcl::PointCloud<VEC3>::Ptr &meshCloud, const std::vector<Eigen::Vector2f> &uvs, size_t faceIndex);

    template <typename T>
    void renderPixel(int row, int col, float u, float v, const cv::Mat &texture);

    void getBarycentricCoordinates(VEC3 v1, VEC3 v2, VEC3 v3, float x, float y, float &l1, float &l2, float &l3) const;

    bool isSliverPolygon(VEC3 v1, VEC3 v2, VEC3 v3) const;

    //bool isModelOk(const pcl::TextureMesh &mesh);

    //bool loadObjFile(std::string inputFile, pcl::TextureMesh &mesh, std::vector<pcl::MTLReader> &companions);

    //bool readHeader (const std::string &file_name, pcl::PCLPointCloud2 &cloud,
    //                 Eigen::Vector4f &origin, Eigen::Quaternionf &orientation,
    //                 int &file_version, int &data_type, unsigned int &data_idx,
    //                 const int offset,
    //                 std::vector<pcl::MTLReader> &companions);

    std::vector<std::string> m_InputFiles;
    std::string m_OutputFile;               // Path to the destination file
    std::string m_OutputCornerFile;         // Path to the output corner file
    std::string m_BandsOrder;

    float m_Resolution;               // number of pixels per meter in the ortho photo

    std::vector<void *> m_bands;
    std::vector<GDALColorInterp> colorInterps;
    std::vector<std::string> bandDescriptions;
    void* mp_AlphaBand;                            // Keep alpha band separate
    int currentBandIndex;

    cv::Mat depth_;                             // depth of the ortho photo as an OpenCV matrix, CV_32F
};

#endif // #ifndef ODMORTHOPHOTO_H
