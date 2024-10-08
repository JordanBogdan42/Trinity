/**
 * @file Time.cpp
 * @date Apr 5, 2013
 * @author sizun
 * @note
 * SVN tag: $Id: Time.cpp 608 2013-04-05 12:33:02Z psizun $
 *
 * Contributors: Patrick Sizun
 *
 * This file is part of the GetBench software project.
 *
 * @copyright © Commissariat a l'Energie Atomique et aux Energies Alternatives (CEA)
 *
 * @par FREE SOFTWARE LICENCING
 * This software is governed by the CeCILL license under French law and abiding  * by the rules of distribution of free
 * software. You can use, modify and/or redistribute the software under the terms of the CeCILL license as circulated by
 * CEA, CNRS and INRIA at the following URL: "http://www.cecill.info". As a counterpart to the access to the source code
 * and rights to copy, modify and redistribute granted by the license, users are provided only with a limited warranty
 * and the software's author, the holder of the economic rights, and the successive licensors have only limited
 * liability. In this respect, the user's attention is drawn to the risks associated with loading, using, modifying
 * and/or developing or reproducing the software by the user in light of its specific status of free software, that may
 * mean that it is complicated to manipulate, and that also therefore means that it is reserved for developers and
 * experienced professionals having in-depth computer knowledge. Users are therefore encouraged to load and test the
 * software's suitability as regards their requirements in conditions enabling the security of their systems and/or data
 * to be ensured and, more generally, to use and operate it in the same conditions as regards security. The fact that
 * you are presently reading this means that you have had knowledge of the CeCILL license and that you accept its terms.
 *
 * @par COMMERCIAL SOFTWARE LICENCING
 * You can obtain this software from CEA under other licencing terms for commercial purposes. For this you will need to
 * negotiate a specific contract with a legal representative of CEA.
 *
 */

#include "get/utl/Time.h"
#include <cstdio>
#ifdef _WIN32
#   include <sys/timeb.h>
#   include <time.h>
#else
#   include <sys/time.h>
#endif

#ifdef __APPLE__
#   include <mach/mach.h>
#   include <mach/mach_time.h>
#endif

namespace get {
namespace utl {
//_________________________________________________________________________________________________
/**
 * Converts date and time to ISO 8601 extended string representation with fraction of seconds.
 * @param dateTime IceUtil time structure.
 * @return ISO string representation of the date and time.
 */
std::string buildTimeStamp(const IceUtil::Time & dateTime)
{
	const IceUtil::Int64 usec = dateTime.toMicroSeconds();
	time_t time = static_cast<long>(usec / 1000000);

	struct tm* timeinfo;
#ifdef _WIN32
	timeinfo = localtime(&time);
#else
	struct tm tr;
	localtime_r(&time, &tr);
	timeinfo = &tr;
#endif

	char stamp[128];
	sprintf(stamp, "%04d-%02d-%02dT%02d:%02d:%02d.%03d",
					1900 + timeinfo->tm_year, 1+timeinfo->tm_mon, timeinfo->tm_mday,
					timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec,
					static_cast<int>(usec % 1000000 / 1000)
				);
	return std::string(stamp);
}
//_________________________________________________________________________________________________

} // namespace utl
} // namespace get

