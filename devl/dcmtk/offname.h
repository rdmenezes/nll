/*
 *
 *  Copyright (C) 1997-2001, OFFIS
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
 *  Module: ofstd
 *
 *  Author: Marco Eichelberg
 *
 *  Purpose:
 *    classes: OFFilenameCreator
 *
 *  Last Update:      $Author: meichel $
 *  Update Date:      $Date: 2001/06/01 15:51:34 $
 *  CVS/RCS Revision: $Revision: 1.5 $
 *  Status:           $State: Exp $
 *
 *  CVS/RCS Log at end of file
 *
 */

#ifndef __OFFNAME_H__
#define __OFFNAME_H__

#include "osconfig.h"    /* make sure OS specific configuration is included first */
#include "oftypes.h"
#include "ofstring.h"

/** A class for automatically creating unique pseudo-random filenames.
 */  
class OFFilenameCreator
{
public:
  /// default constructor
  OFFilenameCreator();
  
  /// copy constructor
  OFFilenameCreator(const OFFilenameCreator& copy);

  /// destructor
  virtual ~OFFilenameCreator();

  /// assignment operator
  OFFilenameCreator &operator=(const OFFilenameCreator& copy);

  /** creates a new unique filename.
   *  @param dir directory in which the file name should be located
   *  @param prefix filename prefix
   *  @param postfix filename postfix
   *  @param filename string in which the filename (path) is returned
   *  @return OFTrue if successful, OFFalse if no filename could be created.
   */
  OFBool makeFilename(unsigned int seed, const char *dir, const char *prefix, const char *postfix, OFString &filename);

  /** creates hash value. (i.e. random seed) from character string.
   *  @param str null-terminated string
   *  @return hash value
   */
  static unsigned int hashString(const char *str);

  /** simple but thread safe random number generator. The interface is derived
   *  from the Posix rand_r function. Uses a multiplicative congruential 
   *  random-number generator with period 2**32 that returns successive 
   *  pseudo-random numbers in the range of 0 to 0x7fffffff.
   *  @param seed pointer to seed of random number generator, must not be NULL.
   *  @return pseudo-random number in the range of 0 to 0x7fffffff.
   */
  static int myrand_r(unsigned int *seed);

private:

  /// date/time of creation of this object
  unsigned long creation_time;

   /** appends the lower 32 bit of the given number to the given string.
    *  Always appends exactly 8 digits (padded with leading zeroes).
    *  @param l number to be appended to string
    *  @param s string to be added to
    */
  static void addLongToString(unsigned long l, OFString& s);
};

#endif

/*
 *  $Log: offname.h,v $
 *  Revision 1.5  2001/06/01 15:51:34  meichel
 *  Updated copyright header
 *
 *  Revision 1.4  2000/10/10 12:01:04  meichel
 *  Implemented thread safe random number generator, needed on systems
 *    where threads but no Posix rand_r function are available.
 *
 *  Revision 1.3  2000/03/08 16:36:01  meichel
 *  Updated copyright header.
 *
 *  Revision 1.2  1999/03/22 09:00:50  joergr
 *  Added/Changed comments.
 *
 *  Revision 1.1  1999/03/03 13:58:04  meichel
 *  Implemented new class for automatically creating filenames.
 *
 *
 */

