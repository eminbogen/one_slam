/*************************************************************************
	> File Name: rgbd-slam-tutorial-gx/part V/src/visualOdometry.cpp
	> Author: xiang gao
	> Mail: gaoxiang12@mails.tsinghua.edu.cn
	> Created Time: 2015年08月01日 星期六 15时35分42秒
 ************************************************************************/

#include <iostream>
#include <fstream>
#include <sstream>
using namespace std;

#include "slamBase.h"

// 给定index，读取一帧数据
FRAME readFrame( int index, ParameterReader& pd);
// 度量运动的大小
double normofTransform( cv::Mat rvec, cv::Mat tvec );
vector<string> path_to_rgb;vector<string> path_to_depth;
int main( int argc, char** argv )
{
    ParameterReader pd;
    int startIndex  =   atoi( pd.getData( "start_index" ).c_str() );
    int endIndex    =   atoi( pd.getData( "end_index"   ).c_str() );

    // initialize
    cout<<"Initializing ..."<<endl;
    readfile_getstring( path_to_rgb,path_to_depth);
    int currIndex = startIndex; // 当前索引为currIndex
    FRAME lastFrame = readFrame( currIndex, pd ); // 上一帧数据
    // 我们总是在比较currFrame和lastFrame
    string detector = pd.getData( "detector" );
    string descriptor = pd.getData( "descriptor" );
    CAMERA_INTRINSIC_PARAMETERS camera = getDefaultCamera();
    computeKeyPointsAndDesp( lastFrame, detector, descriptor );
    PointCloud::Ptr cloud = image2PointCloud( lastFrame.rgb, lastFrame.depth, camera );
    
    pcl::visualization::CloudViewer viewer("viewer");

    // 是否显示点云
    bool visualize = pd.getData("visualize_pointcloud")==string("yes");

    int min_inliers = atoi( pd.getData("min_inliers").c_str() );
    double max_norm = atof( pd.getData("max_norm").c_str() );

    for ( currIndex=startIndex+1; currIndex<endIndex; currIndex++ )
    {
        cout<<"Reading files "<<currIndex<<endl;
        FRAME currFrame = readFrame( currIndex,pd ); // 读取currFrame
        computeKeyPointsAndDesp( currFrame, detector, descriptor );
        // 比较currFrame 和 lastFrame
        RESULT_OF_PNP result = estimateMotion( lastFrame, currFrame, camera );
        if ( result.inliers < min_inliers ) //inliers不够，放弃该帧
            continue;
        // 计算运动范围是否太大
        double norm = normofTransform(result.rvec, result.tvec);
        cout<<"norm = "<<norm<<endl;
        if ( norm >= max_norm )
            continue;
        Eigen::Isometry3d T = cvMat2Eigen( result.rvec, result.tvec );
        cout<<"T="<<T.matrix()<<endl;
        
        cloud = joinPointCloud( cloud, currFrame, T, camera );
        
        if ( visualize == true )
            viewer.showCloud( cloud );

        lastFrame = currFrame;
    }

    pcl::io::savePCDFile( "../data/result.pcd", *cloud );
    return 0;
}

FRAME readFrame( int index, ParameterReader& pd )
{
    FRAME f;//path_to_rgb,path_to_depth
    f.rgb = cv::imread(path_to_rgb.at(index) );
    f.depth = cv::imread(path_to_depth.at(index) , -1 );
    return f;
}

double normofTransform( cv::Mat rvec, cv::Mat tvec )
{
    return fabs(min(cv::norm(rvec), 2*M_PI-cv::norm(rvec)))+ fabs(cv::norm(tvec));
}
