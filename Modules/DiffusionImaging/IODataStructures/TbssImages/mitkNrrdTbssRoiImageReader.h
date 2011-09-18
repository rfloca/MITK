/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-07-14 19:11:20 +0200 (Tue, 14 Jul 2009) $
Version:   $Revision: 18127 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef __mitkNrrdTbssRoiImageReader_h
#define __mitkNrrdTbssRoiImageReader_h

#include "mitkCommon.h"
#include "mitkFileReader.h"
#include "mitkTbssRoiImageSource.h"
#include "itkImage.h"
#include "mitkTbssRoiImage.h"

namespace mitk
{

  /** \brief 
  */

  template < class TPixelType >
  class MitkDiffusionImaging_EXPORT NrrdTbssRoiImageReader : public mitk::TbssRoiImageSource<TPixelType>, public FileReader
  {
  public:

    typedef mitk::TbssRoiImage<TPixelType> OutputType;
    typedef itk::Image<TPixelType,3>     ImageType;
    typedef TbssRoiImageSource<TPixelType>  TbssVolSourceType;



    mitkClassMacro( NrrdTbssRoiImageReader, TbssVolSourceType )
    itkNewMacro(Self)

    const char* GetFileName() const;
    void SetFileName(const char* aFileName);
    const char* GetFilePrefix() const;
    void SetFilePrefix(const char* aFilePrefix);
    const char* GetFilePattern() const;
    void SetFilePattern(const char* aFilePattern);

    static bool CanReadFile(const std::string filename, const std::string filePrefix, const std::string filePattern);

  protected:

    /** Does the real work. */
    virtual void GenerateData();

    void ReadRoiInfo(itk::MetaDataDictionary dict);

    std::string m_FileName;
    std::string m_FilePrefix;
    std::string m_FilePattern;

    typename OutputType::Pointer m_OutputCache;
    itk::TimeStamp m_CacheTime;



  private:
    void operator=(const Self&); //purposely not implemented
  };

} //namespace MITK

#include "mitkNrrdTbssRoiImageReader.cpp"

#endif // __mitkNrrdTbssImageReader_h
