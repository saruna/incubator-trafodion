// **********************************************************************
// @@@ START COPYRIGHT @@@
//
// (C) Copyright 2007-2014 Hewlett-Packard Development Company, L.P.
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//
// @@@ END COPYRIGHT @@@
// **********************************************************************

#include <HdfsLogger.h>
#include <log4cpp/RollingFileAppender.hh>
#include <log4cpp/PatternLayout.hh>
#include <log4cpp/PropertyConfigurator.hh>
#include <log4cpp/Configurator.hh>

const char CAT_JNI_TOP        []      = "JniTop";
const char CAT_ORC_FILE_READER[]      = "OrcFileReader";
const char CAT_SEQ_FILE_READER[]      = "SeqFileReader";
const char CAT_SEQ_FILE_WRITER[]      = "SeqFileWriter";
const char CAT_HBASE          []      = "HBase";

// **************************************************************************
// **************************************************************************
HdfsLogger::HdfsLogger()
  : CommonLogger()
{
}

// **************************************************************************
// **************************************************************************
HdfsLogger& HdfsLogger::instance()
{
  static HdfsLogger onlyInstance_;
  return onlyInstance_;
}

// **************************************************************************
// Call the superclass to configure log4cpp from the config file.
// If the configuration file is not found, perform default initialization:
// Create an appender, layout, and categories.
// Attaches layout to the appender and appender to categories.
// **************************************************************************
NABoolean HdfsLogger::initLog4cpp(const char* configFileName)
{
  if (CommonLogger::initLog4cpp(configFileName))
    return TRUE;

  NAString logFileName;

  // get the log directory
  logFileName = logFolder_;
  logFileName += "Hdfs.log";

  fileAppender_ = new log4cpp::RollingFileAppender("FileAppender", logFileName.data());

  log4cpp::PatternLayout *fileLayout = new log4cpp::PatternLayout();
  fileLayout->setConversionPattern("%d, %p, %c, %m%n");
  fileAppender_->setLayout(fileLayout);

  // Top level categories
  initCategory(CAT_SEQ_FILE_READER, log4cpp::Priority::ERROR);
  initCategory(CAT_SEQ_FILE_WRITER, log4cpp::Priority::ERROR);
  initCategory(CAT_HBASE,           log4cpp::Priority::ERROR);

  //log4cpp::Category::getInstance(CAT_SEQ_FILE_READER).error("Failed to read the config file, using ERROR as logging level."); 
  return FALSE;
}

// **************************************************************************
// **************************************************************************
void HdfsLogger::initCategory(const char* cat, log4cpp::Priority::PriorityLevel defaultPriority)
{
  log4cpp::Category& catObj = log4cpp::Category::getInstance(cat);
  catObj.setAppender(fileAppender_);
  catObj.setPriority(defaultPriority);
}

