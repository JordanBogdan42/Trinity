/* =============================================================================
 * $Id: Exception.h 741 2012-06-12 08:10:23Z psizun $
 * @file Exception.h
 * @author fchateau
 * -----------------------------------------------------------------------------
 * © Commissariat a l'Energie Atomique et aux Energies Alternatives (CEA)
 * -----------------------------------------------------------------------------
 * FREE SOFTWARE LICENCING
 * This software is governed by the CeCILL license under French law and abiding
 * by the rules of distribution of free software. You can use, modify and/or
 * redistribute the software under the terms of the CeCILL license as circulated
 * by CEA, CNRS and INRIA at the following URL: "http://www.cecill.info".
 * As a counterpart to the access to the source code and rights to copy, modify
 * and redistribute granted by the license, users are provided only with a
 * limited warranty and the software's author, the holder of the economic
 * rights, and the successive licensors have only limited liability. In this
 * respect, the user's attention is drawn to the risks associated with loading,
 * using, modifying and/or developing or reproducing the software by the user in
 * light of its specific status of free software, that may mean that it is
 * complicated to manipulate, and that also therefore means that it is reserved
 * for developers and experienced professionals having in-depth computer
 * knowledge. Users are therefore encouraged to load and test the software's
 * suitability as regards their requirements in conditions enabling the security
 * of their systems and/or data to be ensured and, more generally, to use and
 * operate it in the same conditions as regards security.
 * The fact that you are presently reading this means that you have had
 * knowledge of the CeCILL license and that you accept its terms.
 * -----------------------------------------------------------------------------
 * COMMERCIAL SOFTWARE LICENCING
 * You can obtain this software from CEA under other licencing terms for
 * commercial purposes. For this you will need to negotiate a specific contract
 * with a legal representative of CEA.
 * =============================================================================
 */

#ifndef mfm_Exception_h_INCLUDED
#define mfm_Exception_h_INCLUDED

#include <boost/cstdint.hpp>
#include <exception>
#include <string>

namespace mfm {
//______________________________________________________________________
/**
 * This class is the base class of every exception in the module.
 * It must be derived in order to define specific error informations
 * for each exception type. The derived class must implement the formatMessage
 * method because it will be used to initialize the message attribute.
 *
 * @author fchateau
 */
class Exception : public std::exception
{
protected:
	mutable std::string message;	///< the error message. "mutable" is for lazy initialization in getter.
public:
	Exception();
	Exception(const std::string& message);
	Exception(const Exception& value);
	Exception& operator =(const Exception& value);
	virtual ~Exception() throw();

	const char* what() const throw();

	const std::string& getMessage() const;

protected:
	/**
	 * Format an error message for the actual exception based on its attributes.
	 * @return Returns the generated error message.
	 */
	virtual std::string formatMessage() const;
};
//______________________________________________________________________
/**
 * Exception thrown when a item query fails.
 */
class ItemNotFound : public ::mfm::Exception
{
public:
	ItemNotFound(size_t indexNotFound=0, size_t itemCount=0);
	~ItemNotFound() throw();
private:
	std::string formatMessage() const;
	size_t indexNotFound_; ///< Item index not found.
	size_t itemCount_; ///< Actual number of items.
};
//______________________________________________________________________
/**
 * Logic error. Thrown when encountering a invalid size or position.
 */
class OutOfRangeError : public ::mfm::Exception
{
public:
	OutOfRangeError();
	OutOfRangeError(const std::string & details);
	~OutOfRangeError() throw();
private:
	std::string formatMessage() const;
};
//______________________________________________________________________
/**
 * Exception thrown when a field query fails.
 */
class FieldNotFound : public ::mfm::Exception
{
public:
	FieldNotFound(std::string const & fieldName);
	~FieldNotFound() throw();
private:
	std::string formatMessage() const;
	std::string fieldName_; ///< Name of the field not found.
};
//______________________________________________________________________
/**
 * Exception thrown when a bit field query fails.
 */
class BitFieldNotFound : public ::mfm::Exception
{
public:
	BitFieldNotFound(std::string const & fieldName, std::string const & bitFieldName);
	~BitFieldNotFound() throw();
private:
	std::string formatMessage() const;
	std::string fieldName_; ///< Name of the field within the bit field has not been found.
	std::string bitFieldName_; ///< Name of the bit field not found.
};
//______________________________________________________________________
/**
 * Exception thrown when no format description is found for a frame.
 */
class FormatNotFound : public ::mfm::Exception
{
public:
	FormatNotFound(uint16_t type);
	~FormatNotFound() throw();
private:
	std::string formatMessage() const;
protected:
	uint16_t type_; ///< Type code for which no frame format has been found.
};
//______________________________________________________________________
/**
 * Exception thrown when no format description is found for a specific revision of a given frame type.
 */
class FormatRevisionNotFound : public ::mfm::FormatNotFound
{
public:
	FormatRevisionNotFound(uint16_t type, uint8_t revision);
	~FormatRevisionNotFound() throw();
private:
	std::string formatMessage() const;
protected:
	uint8_t revision_; ///< Revision for which no frame format has been found.
};
//______________________________________________________________________
} /* namespace mfm */
#endif /* mfm_Exception_h_INCLUDED */
