/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2007-12-11 14:46:19 +0100 (Di, 11 Dez 2007) $
Version:   $Revision: 13129 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkNavigationDataRecorder.h"
#include <fstream>

#include <mitkTimeStamp.h>
#include <tinyxml.h>

#include <itksys/SystemTools.hxx>







mitk::NavigationDataRecorder::NavigationDataRecorder()
{
  m_NumberOfInputs = 0;
  m_RecordingMode = NormalFile;
  m_Recording = false;
  m_NumberOfRecordedFiles = 0;
  m_Stream = NULL;
  m_FileName = "";
  m_SystemTimeClock = RealTimeClock::New();
  m_OutputFormat = mitk::NavigationDataRecorder::xml;

  //To get a start time
  mitk::TimeStamp::GetInstance()->Start(this);
  

}

mitk::NavigationDataRecorder::~NavigationDataRecorder()
{
}


void mitk::NavigationDataRecorder::GenerateData()
{

}

void mitk::NavigationDataRecorder::AddNavigationData( const NavigationData* nd )
{
  // Process object is not const-correct so the const_cast is required here
  this->SetNthInput(m_NumberOfInputs, 
    const_cast< mitk::NavigationData * >( nd ) );

  m_NumberOfInputs++;

  this->Modified();
}

void mitk::NavigationDataRecorder::SetRecordingMode( RecordingMode mode )
{
  m_RecordingMode = mode;
  this->Modified();
}

void mitk::NavigationDataRecorder::Update()
{
  if (m_Recording)
  {
    DataObjectPointerArray inputs = this->GetInputs(); //get all inputs
    mitk::NavigationData::TimeStampType timestamp=0.0; // timestamp for mitk time
    timestamp = mitk::TimeStamp::GetInstance()->GetElapsed();


    mitk::NavigationData::TimeStampType sysTimestamp = 0.0; // timestamp for system time
    sysTimestamp = m_SystemTimeClock->GetCurrentStamp();

    // cast system time double value to stringstream to avoid low precision rounding 
    std::ostringstream strs;
    strs.precision(15); // rounding precision for system time double value
    strs << sysTimestamp;
    std::string sysTimeStr = strs.str();

    //write csv header if first line
    if ((m_OutputFormat == mitk::NavigationDataRecorder::csv) && m_firstLine)
      {
      m_firstLine = false;
      *m_Stream << "TimeStamp";
      for (unsigned int index = 0; index < inputs.size(); index++){ *m_Stream << ";Valid_Tool" << index <<
                                                                                ";X_Tool" << index <<
                                                                                ";Y_Tool" << index <<
                                                                                ";Z_Tool" << index <<
                                                                                ";QX_Tool" << index <<
                                                                                ";QY_Tool" << index <<
                                                                                ";QZ_Tool" << index <<
                                                                                ";QR_Tool" << index;}
      *m_Stream << "\n";
      }
      
    *m_Stream << timestamp;
    for (unsigned int index = 0; index < inputs.size(); index++)
    {
      mitk::NavigationData* nd = dynamic_cast<mitk::NavigationData*>(inputs[index].GetPointer());
      nd->Update(); // call update to propagate update to previous filters

      mitk::NavigationData::PositionType position;
      mitk::NavigationData::OrientationType orientation(0.0, 0.0, 0.0, 0.0);
      mitk::NavigationData::CovarianceMatrixType matrix;

      bool hasPosition = true;    
      bool hasOrientation = true; 
      bool dataValid = false;

      position.Fill(0.0);
      matrix.SetIdentity();

      position = nd->GetPosition();
      orientation = nd->GetOrientation();
      matrix = nd->GetCovErrorMatrix();
      
      hasPosition = nd->GetHasPosition();
      hasOrientation = nd->GetHasOrientation();
      dataValid = nd->IsDataValid();

      //use this one if you want the timestamps of the source
      //timestamp = nd->GetTimeStamp();

      //a timestamp is never < 0! this case happens only if you are using the timestamp of the nd object instead of getting a new one
      if (timestamp >= 0)
      { 
        if (this->m_OutputFormat = mitk::NavigationDataRecorder::xml)
          {
          TiXmlElement* elem = new TiXmlElement("ND");

          elem->SetDoubleAttribute("Time", timestamp);
          elem->SetAttribute("SystemTime", sysTimeStr); // tag for system time
          elem->SetDoubleAttribute("Tool", index);
          elem->SetDoubleAttribute("X", position[0]);
          elem->SetDoubleAttribute("Y", position[1]);
          elem->SetDoubleAttribute("Z", position[2]);

          elem->SetDoubleAttribute("QX", orientation[0]);
          elem->SetDoubleAttribute("QY", orientation[1]);
          elem->SetDoubleAttribute("QZ", orientation[2]);
          elem->SetDoubleAttribute("QR", orientation[3]);

          elem->SetDoubleAttribute("C00", matrix[0][0]);
          elem->SetDoubleAttribute("C01", matrix[0][1]);
          elem->SetDoubleAttribute("C02", matrix[0][2]);
          elem->SetDoubleAttribute("C03", matrix[0][3]);
          elem->SetDoubleAttribute("C04", matrix[0][4]);
          elem->SetDoubleAttribute("C05", matrix[0][5]);
          elem->SetDoubleAttribute("C10", matrix[1][0]);
          elem->SetDoubleAttribute("C11", matrix[1][1]);
          elem->SetDoubleAttribute("C12", matrix[1][2]);
          elem->SetDoubleAttribute("C13", matrix[1][3]);
          elem->SetDoubleAttribute("C14", matrix[1][4]);
          elem->SetDoubleAttribute("C15", matrix[1][5]);

          if (dataValid)
            elem->SetAttribute("Valid",1);
          else
            elem->SetAttribute("Valid",0);

          if (hasOrientation)
            elem->SetAttribute("hO",1);
          else
            elem->SetAttribute("hO",0);

          if (hasPosition)
            elem->SetAttribute("hP",1);
          else
            elem->SetAttribute("hP",0);
          
          *m_Stream << "        " << *elem << std::endl;

          delete elem;
          }
        else if (this->m_OutputFormat = mitk::NavigationDataRecorder::csv)
          {
          *m_Stream << ";" << dataValid << ";" << position[0] << ";" << position[1] << ";" << position[2] << ";" << orientation[0] << ";" << orientation[1] << ";" << orientation[2] << ";" << orientation[3];
          }
      }
    }
    if (this->m_OutputFormat = mitk::NavigationDataRecorder::csv) *m_Stream << "\n";
  }
}

void mitk::NavigationDataRecorder::StartRecording()
{
  if (m_Recording)
  {
    std::cout << "Already recording please stop before start new recording session" << std::endl;
    return;
  }
  if (m_Stream == NULL)
  {
    std::stringstream ss;
    std::ostream* stream;
    
    //An existing extension will be cut and replaced with .xml
    std::string tmpPath = itksys::SystemTools::GetFilenamePath(m_FileName);
    m_FileName = itksys::SystemTools::GetFilenameWithoutExtension(m_FileName);
    if (m_OutputFormat == mitk::NavigationDataRecorder::csv) ss << tmpPath << "/" <<  m_FileName << "-" << m_NumberOfRecordedFiles << ".csv";
    else ss << tmpPath << "/" <<  m_FileName << "-" << m_NumberOfRecordedFiles << ".xml";
    switch(m_RecordingMode)
    {
      case Console:
        stream = &std::cout;
        break;
      case NormalFile:

        //Check if there is a file name and path
        if (m_FileName == "")
        {
          stream = &std::cout;
          std::cout << "No file name or file path set the output is redirected to the console";
        }
        else
        {
          stream = new std::ofstream(ss.str().c_str());
        }

        break;
      case ZipFile:
        stream = &std::cout;
        std::cout << "Sorry no ZipFile support yet";
        break;
      default:
        stream = &std::cout;
        break;
    }
    m_firstLine = true;
    StartRecording(stream);
  }




}
void mitk::NavigationDataRecorder::StartRecording(std::ostream* stream)
{
  if (m_Recording)
  {
    std::cout << "Already recording please stop before start new recording session" << std::endl;
    return;
  }

  m_Stream = stream;
  m_Stream->precision(10);

  //TODO store date and GMT time
  if (m_Stream)
  {
    if (m_OutputFormat == mitk::NavigationDataRecorder::xml)
      {
      *m_Stream << "<?xml version=\"1.0\" ?>" << std::endl;
      /**m_Stream << "<Version Ver=\"1\" />" << std::endl;*/
      // should be a generic version, meaning a member variable, which has the actual version
      *m_Stream << "    " << "<Data ToolCount=\"" << (m_NumberOfInputs) << "\" version=\"1.0\">" << std::endl;
      }
    m_Recording = true;
  }
}
void mitk::NavigationDataRecorder::StopRecording()
{
  if (!m_Recording)
  {
    std::cout << "You have to start a recording first" << std::endl;
    return;
  }
 
  if (m_Stream && m_OutputFormat == mitk::NavigationDataRecorder::xml)
  {
    *m_Stream << "</Data>" << std::endl;
  }

  m_NumberOfRecordedFiles++;
  m_Stream = NULL;
  m_Recording = false;
}