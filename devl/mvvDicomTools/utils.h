#ifndef MVV_DICOM_TOOLS_UTILS_H_
# define MVV_DICOM_TOOLS_UTILS_H_

# include "mvvDicomTools.h"
# include <Boost/filesystem.hpp>
# include <dcmtk/dcmdata/dcfilefo.h>
# include <dcmtk/dcmdata/dcdeftag.h>
# include <dcmtk/dcmimgle/dcmimage.h>
# include <dcmtk/dcmdata/dcxfer.h>
# include <memory>
# include <mvvScript/function-runnable.h>

using namespace boost::filesystem;
using namespace mvv::platform;
using namespace mvv::parser;

// TODO this is a quick fix for <RegistrationImpl> project: the tool to export DICOM volumes inverts y axis, if the DICOM is not reading for this particular
// the flag should be disabled!
//#define ORIENTATION_FOR_MATLAB_COMPABILITY

namespace mvv
{
   

   

   

   
}

#endif