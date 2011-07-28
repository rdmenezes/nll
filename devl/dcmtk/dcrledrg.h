/*
 *
 *  Copyright (C) 1994-2002, OFFIS
 *
 *  This software and supporting documentation were developed by
 *
 *    Kuratorium OFFIS e.V.
 *    Healthcare Information and Communication Systems
 *    Escherweg 2
 *    D-26121 Oldenburg, Germany
 *
 *  THIS SOFTWARE IS MADE AVAILABLE,  AS IS,  AND OFFIS MAKES NO  WARRANTY
 *  REGARDING  THE  SOFTWARE,  ITS  PERFORMANCE,  ITS  MERCHANTABILITY  OR
 *  FITNESS FOR ANY PARTICULAR USE, FREEDOM FROM ANY COMPUTER DISEASES  OR
 *  ITS CONFORMITY TO ANY SPECIFICATION. THE ENTIRE RISK AS TO QUALITY AND
 *  PERFORMANCE OF THE SOFTWARE IS WITH THE USER.
 *
 *  Module:  dcmdata
 *
 *  Author:  Marco Eichelberg
 *
 *  Purpose: singleton class that registers RLE decoder.
 *
 *  Last Update:      $Author: meichel $
 *  Update Date:      $Date: 2002/06/06 14:52:36 $
 *  Source File:      $Source: /share/dicom/cvs-depot/dcmtk/dcmdata/include/dcrledrg.h,v $
 *  CVS/RCS Revision: $Revision: 1.1 $
 *  Status:           $State: Exp $
 *
 *  CVS/RCS Log at end of file
 *
 */

#ifndef DCRLEDRG_H
#define DCRLEDRG_H

#include "osconfig.h"
#include "oftypes.h"  /* for OFBool */

class DcmRLECodecParameter;
class DcmRLECodecDecoder;

/** singleton class that registers an RLE decoder.
 */
class DcmRLEDecoderRegistration 
{
public: 
  /** registers RLE decoder.
   *  If already registered, call is ignored unless cleanup() has
   *  been performed before.
   *  @param pCreateSOPInstanceUID flag indicating whether or not
   *    a new SOP Instance UID should be assigned upon decompression.
   *  @param pVerbose verbose mode flag
   */   
  static void registerCodecs(
    OFBool pCreateSOPInstanceUID = OFFalse,
    OFBool pVerbose = OFFalse);

  /** deregisters decoder.
   *  Attention: Must not be called while other threads might still use
   *  the registered codecs, e.g. because they are currently decoding
   *  DICOM data sets through dcmdata.
   */  
  static void cleanup();

private:

  /// flag indicating whether the decoder is already registered.
  static OFBool registered;

  /// pointer to codec parameter
  static DcmRLECodecParameter *cp;
  
  /// pointer to RLE decoder
  static DcmRLECodecDecoder *codec;
  
};

#endif

/*
 * CVS/RCS Log
 *
 */
