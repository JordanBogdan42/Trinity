//
// Copyright (C) 2004-2006 Maciej Sobczak, Stephen Hutton, David Courtney
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#define SOCI_ODBC_SOURCE
#include "soci-odbc.h"
#include <soci.h>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <sstream>

#ifdef _MSC_VER
// disables the warning about converting int to void*.  This is a 64 bit compatibility
// warning, but odbc requires the value to be converted on this line
// SQLSetStmtAttr(statement_.hstmt_, SQL_ATTR_PARAMSET_SIZE, (SQLPOINTER)arraySize, 0);
#pragma warning(disable:4312)
#endif


using namespace SOCI;
using namespace SOCI::details;

void ODBCVectorUseTypeBackEnd::prepareIndicators(std::size_t size)
{
    if (size == 0)
    {
         throw SOCIError("Vectors of size 0 are not allowed.");
    }

    indHolderVec_.resize(size);
    indHolders_ = &indHolderVec_[0];
}

void ODBCVectorUseTypeBackEnd::prepareForBind(void *&data, SQLUINTEGER &size, SQLSMALLINT &sqlType, SQLSMALLINT &cType)
{
    switch (type_)
    {    // simple cases
    case eXShort:
        {
            sqlType = SQL_SMALLINT;
            cType = SQL_C_SSHORT;
            size = sizeof(short);
            std::vector<short> *vp = static_cast<std::vector<short> *>(data);
            std::vector<short> &v(*vp);
            prepareIndicators(v.size());
            data = &v[0];
        }
        break;
    case eXInteger:
        {
            sqlType = SQL_INTEGER;
            cType = SQL_C_SLONG;
            size = sizeof(int);
            std::vector<int> *vp = static_cast<std::vector<int> *>(data);
            std::vector<int> &v(*vp);
            prepareIndicators(v.size());
            data = &v[0];
        }
        break;
    case eXUnsignedLong:
        {
            sqlType = SQL_BIGINT;
            cType = SQL_C_ULONG;
            size = sizeof(unsigned long);
            std::vector<unsigned long> *vp
                 = static_cast<std::vector<unsigned long> *>(data);
            std::vector<unsigned long> &v(*vp);
            prepareIndicators(v.size());
            data = &v[0];
        }
        break;
    case eXDouble:
        {
            sqlType = SQL_DOUBLE;
            cType = SQL_C_DOUBLE;
            size = sizeof(double);
            std::vector<double> *vp = static_cast<std::vector<double> *>(data);
            std::vector<double> &v(*vp);
            prepareIndicators(v.size());
            data = &v[0];
        }
        break;

    // cases that require adjustments and buffer management
    case eXChar:
        {
            std::vector<char> *vp
                = static_cast<std::vector<char> *>(data);
            std::size_t const vsize = vp->size();

            prepareIndicators(vsize);

            size = sizeof(char) * 2;
            buf_ = new char[size * vsize];

            char *pos = buf_;

            for (std::size_t i = 0; i != vsize; ++i)
            {
                *pos++ = (*vp)[i];
                *pos++ = 0;
            }

            sqlType = SQL_CHAR;
            cType = SQL_C_CHAR;
            data = buf_;
        }
        break;
    case eXStdString:
        {
            sqlType = SQL_CHAR;
            cType = SQL_C_CHAR;

            std::vector<std::string> *vp
                = static_cast<std::vector<std::string> *>(data);
            std::vector<std::string> &v(*vp);

            std::size_t maxSize = 0;
            std::size_t const vecSize = v.size();
            prepareIndicators(vecSize);
            for (std::size_t i = 0; i != vecSize; ++i)
            {
                std::size_t sz = v[i].length() + 1;  // add one for null
				indHolderVec_[i] = static_cast<long>(sz);
                maxSize = sz > maxSize ? sz : maxSize;
            }

            buf_ = new char[maxSize * vecSize];
            memset(buf_, 0, maxSize * vecSize);

            char *pos = buf_;
            for (std::size_t i = 0; i != vecSize; ++i)
            {
                strncpy(pos, v[i].c_str(), v[i].length());
                pos += maxSize;
            }

            data = buf_;
            size = static_cast<SQLINTEGER>(maxSize);
        }
        break;
    case eXStdTm:
        {
            std::vector<std::tm> *vp
                = static_cast<std::vector<std::tm> *>(data);

            prepareIndicators(vp->size());

            buf_ = new char[sizeof(TIMESTAMP_STRUCT) * vp->size()];

            sqlType = SQL_TYPE_TIMESTAMP;
            cType = SQL_C_TYPE_TIMESTAMP;
            data = buf_;
            size = 19; // This number is not the size in bytes, but the number
                      // of characters in the date if it was written out
                      // yyyy-mm-dd hh:mm:ss
        }
        break;

    case eXStatement: break; // not supported
    case eXRowID:     break; // not supported
    case eXBLOB:      break; // not supported
    case eXCString:   break; // not supported
    }

    colSize_ = size;
}

void ODBCVectorUseTypeBackEnd::bindHelper(int &position, void *data, eExchangeType type)
{
    data_ = data; // for future reference
    type_ = type; // for future reference

    SQLSMALLINT sqlType;
    SQLSMALLINT cType;
    SQLUINTEGER size;

    prepareForBind(data, size, sqlType, cType);

    SQLINTEGER arraySize = (SQLINTEGER)indHolderVec_.size();
    SQLSetStmtAttr(statement_.hstmt_, SQL_ATTR_PARAMSET_SIZE, (SQLPOINTER)arraySize, 0);

	SQLRETURN rc = SQLBindParameter(statement_.hstmt_, position++, SQL_PARAM_INPUT,
                                    cType, sqlType, size, 0, (SQLPOINTER)data,
                                    size, indHolders_);

    if (is_odbc_error(rc))
    {
        throw ODBCSOCIError(SQL_HANDLE_STMT, statement_.hstmt_,
            "Error while binding value to column");
    }
}

void ODBCVectorUseTypeBackEnd::bindByPos(int &position,
        void *data, eExchangeType type)
{
    if (statement_.boundByName_)
    {
        throw SOCIError(
         "Binding for use elements must be either by position or by name.");
    }

    bindHelper(position, data, type);

    statement_.boundByPos_ = true;
}

void ODBCVectorUseTypeBackEnd::bindByName(
    std::string const &name, void *data, eExchangeType type)
{
    if (statement_.boundByPos_)
    {
        throw SOCIError(
         "Binding for use elements must be either by position or by name.");
    }

    int position = -1;
    int count = 1;

    for (std::vector<std::string>::iterator it = statement_.names_.begin();
         it != statement_.names_.end(); ++it)
    {
        if (*it == name)
        {
            position = count;
            break;
        }
        count++;
    }

    if (position != -1)
        bindHelper(position, data, type);
    else
    {
        std::ostringstream ss;
        ss << "Unable to find name '" << name << "' to bind to";
        throw SOCIError(ss.str().c_str());
    }

    statement_.boundByName_ = true;
}

void ODBCVectorUseTypeBackEnd::preUse(eIndicator const *ind)
{
    // first deal with data
    if (type_ == eXStdTm)
    {
        std::vector<std::tm> *vp
             = static_cast<std::vector<std::tm> *>(data_);

        std::vector<std::tm> &v(*vp);

        char *pos = buf_;
        std::size_t const vsize = v.size();
        for (std::size_t i = 0; i != vsize; ++i)
        {
            std::tm t = v[i];
            TIMESTAMP_STRUCT * ts = reinterpret_cast<TIMESTAMP_STRUCT*>(pos);

            ts->year = t.tm_year + 1900;
            ts->month = t.tm_mon + 1;
            ts->day = t.tm_mday;
            ts->hour = t.tm_hour;
            ts->minute = t.tm_min;
            ts->second = t.tm_sec;
            ts->fraction = 0;
            pos += sizeof(TIMESTAMP_STRUCT);
        }
    }

    // then handle indicators
    if (ind != NULL)
    {
        std::size_t const vsize = size();
        for (std::size_t i = 0; i != vsize; ++i, ++ind)
        {
            if (*ind == eNull)
            {
                indHolderVec_[i] = SQL_NULL_DATA; // null
            }
            else
            {
            // for strings we have already set the values
            if (type_ != eXStdString)
                {
                    indHolderVec_[i] = SQL_NTS;  // value is OK
                }
            }
        }
    }
    else
    {
        // no indicators - treat all fields as OK
        std::size_t const vsize = size();
        for (std::size_t i = 0; i != vsize; ++i, ++ind)
        {
            // for strings we have already set the values
            if (type_ != eXStdString)
            {
	            indHolderVec_[i] = SQL_NTS;  // value is OK
            }
        }
    }
}

std::size_t ODBCVectorUseTypeBackEnd::size()
{
    std::size_t sz = 0; // dummy initialization to please the compiler
    switch (type_)
    {
    // simple cases
    case eXChar:
        {
            std::vector<char> *vp = static_cast<std::vector<char> *>(data_);
            sz = vp->size();
        }
        break;
    case eXShort:
        {
            std::vector<short> *vp = static_cast<std::vector<short> *>(data_);
            sz = vp->size();
        }
        break;
    case eXInteger:
        {
            std::vector<int> *vp = static_cast<std::vector<int> *>(data_);
            sz = vp->size();
        }
        break;
    case eXUnsignedLong:
        {
            std::vector<unsigned long> *vp
                = static_cast<std::vector<unsigned long> *>(data_);
            sz = vp->size();
        }
        break;
    case eXDouble:
        {
            std::vector<double> *vp
                = static_cast<std::vector<double> *>(data_);
            sz = vp->size();
        }
        break;
    case eXStdString:
        {
            std::vector<std::string> *vp
                = static_cast<std::vector<std::string> *>(data_);
            sz = vp->size();
        }
        break;
    case eXStdTm:
        {
            std::vector<std::tm> *vp
                = static_cast<std::vector<std::tm> *>(data_);
            sz = vp->size();
        }
        break;

    case eXStatement: break; // not supported
    case eXRowID:     break; // not supported
    case eXBLOB:      break; // not supported
    case eXCString:   break; // not supported
    }

    return sz;
}

void ODBCVectorUseTypeBackEnd::cleanUp()
{
    if (buf_ != NULL)
    {
        delete [] buf_;
        buf_ = NULL;
    }
}
