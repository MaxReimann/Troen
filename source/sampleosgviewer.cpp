#include "sampleosgviewer.h"
#include <iostream>

using namespace troen;

SampleOSGViewer::SampleOSGViewer(osg::GraphicsContext* context /*= NULL*/)
{
	setThreadingModel(osgViewer::ViewerBase::ThreadingModel::ThreadPerContext);
}

SampleOSGViewer::~SampleOSGViewer()
{
}
