/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2010-05-27 16:06:53 +0200 (Do, 27 Mai 2010) $
Version:   $Revision:  $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __mitkKinectController_h
#define __mitkKinectController_h

#include "mitkToFHardwareExports.h"
#include "mitkCommon.h"
#include "mitkToFConfig.h"

#include "itkObject.h"
#include "itkObjectFactory.h"

namespace mitk
{
  /**
  * @brief Interface to the Kinect camera
  * 
  *
  * @ingroup ToFHardware
  */
  class MITK_TOFHARDWARE_EXPORT KinectController : public itk::Object
  {
  public: 

    mitkClassMacro( KinectController , itk::Object );

    itkNewMacro( Self );

    unsigned int GetCaptureWidth() const;
    unsigned int GetCaptureHeight() const;
    bool GetUseIR() const;

    void SetUseIR(bool useIR);

    /*!
    \brief opens a connection to the Kinect camera.
    */
    virtual bool OpenCameraConnection();
    /*!
    \brief closes the connection to the camera
    */
    virtual bool CloseCameraConnection();
    /*!
    \brief updates the camera. The update function of the hardware interface is called only when new data is available
    */
    virtual bool UpdateCamera();
    /*!
    \brief acquire new distance data from the Kinect camera
    \param distances pointer to memory location where distances should be stored
    */
    void GetDistances(float* distances);
    void GetAmplitudes(float* amplitudes);
    void GetIntensities(float* intensities);
    /*!
    \brief acquire new rgb data from the Kinect camera
    \parama rgb pointer to memory location where rgb information should be stored
    */
    void GetRgb(unsigned char* rgb);
    /*!
    \brief convenience method for faster access to distance and rgb data
    \param distances pointer to memory location where distances should be stored
    \param rgb pointer to memory location where rgb information should be stored
    */
    void GetAllData(float* distances, float* amplitudes, unsigned char* rgb);

  protected:

    KinectController();

    ~KinectController();

  private:
    class KinectControllerPrivate;
    KinectControllerPrivate *d;

  };
} //END mitk namespace
#endif