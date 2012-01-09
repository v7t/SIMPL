/* ============================================================================
 * Copyright (c) 2010, Michael A. Jackson (BlueQuartz Software)
 * Copyright (c) 2010, Dr. Michael A. Groeber (US Air Force Research Laboratories
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice, this
 * list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 *
 * Neither the name of Michael A. Groeber, Michael A. Jackson, the US Air Force,
 * BlueQuartz Software nor the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior written
 * permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  This code was written under United States Air Force Contract number
 *                           FA8650-07-D-5800
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#include "H5CtfVolumeReader.h"

#include <cmath>

#include "H5Support/H5Lite.h"
#include "H5Support/H5Utilities.h"

#include "EbsdLib/Utilities/StringUtils.h"

#include "EbsdLib/EbsdConstants.h"
#include "EbsdLib/HKL/H5CtfReader.h"

#if defined (H5Support_NAMESPACE)
using namespace H5Support_NAMESPACE;
#endif

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
H5CtfVolumeReader::H5CtfVolumeReader() :
    H5EbsdVolumeReader()
{

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
H5CtfVolumeReader::~H5CtfVolumeReader()
{

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
std::vector<CtfPhase::Pointer> H5CtfVolumeReader::getPhases()
{
  m_Phases.clear();

  // Get the first valid index of a z slice
  std::string index = StringUtils::numToString(getZStart());

  // Open the hdf5 file and read the data
  hid_t fileId = H5Utilities::openFile(getFileName(), true);
  if (fileId < 0)
  {
    std::cout << "Error" << std::endl;
    return m_Phases;
  }
  herr_t err = 0;

  hid_t gid = H5Gopen(fileId, index.c_str(), H5P_DEFAULT);
  H5CtfReader::Pointer reader = H5CtfReader::New();
  reader->setHDF5Path(index);
  err = reader->readHeader(gid);
  if (err < 0)
  {
    std::cout  << "Error reading the header information from the .h5ebsd file" << std::endl;
    err = H5Gclose(gid);
    err = H5Fclose(fileId);
    return m_Phases;
  }
  m_Phases = reader->getPhases();
  if (err < 0)
  {
    std::cout << "Error reading the .HDF5 EBSD Header data" << std::endl;
    err = H5Gclose(gid);
    err = H5Fclose(fileId);
    return m_Phases;
  }
  return m_Phases;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int H5CtfVolumeReader::loadData(float* eulerangles,
                                int* phases,
                                bool* goodVoxels,
                                int64_t xpoints,
                                int64_t ypoints,
                                int64_t zpoints,
                                Ebsd::RefFrameZDir ZDir,
                                std::vector<QualityMetricFilter::Pointer> filters)
{
  int index = 0;
  int err = -1;

  int readerIndex;
  int xpointsslice;
  int ypointsslice;
  int xpointstemp;
  int ypointstemp;
//  int xstop;
//  int ystop;
  int zval;

  float* euler1Ptr = NULL;
  float* euler2Ptr = NULL;
  float* euler3Ptr = NULL;
  int* phasePtr = NULL;
  int xstartspot;
  int ystartspot;

  // Create an Array of Void Pointers that will point to the data that is going to
  // serve as the filter data, such as Confidence Index or Image Quality
  std::vector<void*> dataPointers(filters.size(), NULL);
  std::vector<Ebsd::NumType> dataTypes(filters.size(), Ebsd::UnknownNumType);

  err = readVolumeInfo();

  for (int slice = 0; slice < zpoints; ++slice)
  {
    H5CtfReader::Pointer reader = H5CtfReader::New();
    reader->setFileName(getFileName());
    reader->setHDF5Path(StringUtils::numToString(slice + getZStart()));
    reader->setUserZDir(getStackingOrder());
    reader->setRotateSlice(getRotateSlice());
    reader->setReorderArray(getReorderArray());

    err = reader->readFile();
    if (err < 0)
    {
      std::cout << "H5CtfVolumeReader Error: There was an issue loading the data from the hdf5 file." << std::endl;
      return -1;
    }
    readerIndex = 0;
    xpointsslice = reader->getXCells();
    ypointsslice = reader->getYCells();
    euler1Ptr = reader->getEuler1Pointer();
    euler2Ptr = reader->getEuler2Pointer();
    euler3Ptr = reader->getEuler3Pointer();
    phasePtr = reader->getPhasePointer();
    // Gather some information about the filters and types in order to run the QualityMetric Filter
    for (size_t i = 0; i < filters.size(); ++i)
    {
      dataPointers[i] = reader->getPointerByName(filters[i]->getFieldName());
      dataTypes[i] = reader->getPointerType(filters[i]->getFieldName());
    }

    // Figure out which are good voxels
    DataArray<bool>::Pointer good_voxels = determineGoodVoxels(filters, dataPointers, xpointsslice * ypointsslice, dataTypes);

    xpointstemp = xpoints;
    ypointstemp = ypoints;
    xstartspot = (xpointstemp - xpointsslice) / 2;
    ystartspot = (ypointstemp - ypointsslice) / 2;

    if (ZDir == 0) zval = slice;
    if (ZDir == 1) zval = (zpoints - 1) - slice;

    // Copy the data from the current storage into the ReconstructionFunc Storage Location
    for (int j = 0; j < ypointsslice; j++)
    {
      for (int i = 0; i < xpointsslice; i++)
      {
        index = (zval * xpointstemp * ypointstemp) + ((j + ystartspot) * xpointstemp) + (i + xstartspot);
        eulerangles[3*index] = euler1Ptr[readerIndex]; // Phi1
        eulerangles[3*index + 1] = euler2Ptr[readerIndex]; // Phi
        eulerangles[3*index + 2] = euler3Ptr[readerIndex]; // Phi2
        phases[index] = phasePtr[readerIndex]; // Phase Add 1 to the phase number because .ctf files are zero based for phases
        if (NULL != good_voxels.get())
        {
          goodVoxels[index] = good_voxels->GetValue(readerIndex);
        }
        ++readerIndex;
      }
    }
  }
  return err;

}

