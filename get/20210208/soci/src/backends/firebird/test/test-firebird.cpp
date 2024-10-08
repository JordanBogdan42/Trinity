//
// Copyright (C) 2004-2006 Maciej Sobczak, Stephen Hutton, Rafal Bobrowski
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
//

#include "soci.h"
#include "soci-firebird.h"
#include "error.h"            // SOCI::details::Firebird::throwISCError()
#include "common-tests.h"
#include <iostream>
#include <string>
#include <cassert>
#include <ctime>

using namespace SOCI;
using namespace SOCI::tests;


std::string connectString;
BackEndFactory const &backEnd = firebird;

// fundamental tests - transactions in Firebird
void test1()
{
    {
        Session sql(backEnd, connectString);

        // In Firebird transaction is always required and is started
        // automatically when session is opened. There is no need to
        // call Session::begin(); it will do nothing if there is active
        // transaction.

        // sql.begin();

        try
        {
            sql << "drop table test1";
        }
        catch (SOCIError const &)
        {} // ignore if error

        sql << "create table test1 (id integer)";

        // After DDL statement transaction must be commited or changes
        // won't be visible to active transaction.
        sql.commit();

        // After commit or rollback, transaction must be started manually.
        sql.begin();

        sql << "insert into test1(id) values(5)";
        sql << "drop table test1";

        // Transaction is automatically commited in Session's destructor
    }

    std::cout << "test 1 passed" << std::endl;
}

// character types
void test2()
{
    Session sql(backEnd, connectString);

    try
    {
        sql << "drop table test2";
    }
    catch (SOCIError const &)
    {} // ignore if error

    sql << "create table test2 (p1 char(10), p2 varchar(10))";
    sql.commit();

    sql.begin();

    {
        char a('a'), b('b'), c1, c2;

        sql << "insert into test2(p1,p2) values(?,?)", use(a), use(b);

        sql << "select p1,p2 from test2", into(c1), into(c2);
        assert(c1 == 'a' && c2 == 'b');

        sql << "delete from test2";
    }

    {
        char msg[] = "Hello, Firebird!";
        char buf1[100], buf2[100], buf3[100];
        char *b1 = buf1, *b2 = buf2, *b3 = buf3;

        strcpy(b1, msg);

        sql << "insert into test2(p1, p2) values (?,?)", use(b1,100), use(b1,100);
        sql << "select p1, p2 from test2", into(b2,100), into(b3,100);

        assert(
            buf2[0] == 'H' && buf3[0] == 'H' &&
            buf2[1] == 'e' && buf3[1] == 'e' &&
            buf2[2] == 'l' && buf3[2] == 'l' &&
            buf2[3] == 'l' && buf3[3] == 'l' &&
            buf2[4] == 'o' && buf3[4] == 'o' &&
            buf2[5] == ',' && buf3[5] == ',' &&
            buf2[6] == ' ' && buf3[6] == ' ' &&
            buf2[7] == 'F' && buf3[7] == 'F' &&
            buf2[8] == 'i' && buf3[8] == 'i' &&
            buf2[9] == 'r' && buf3[9] == 'r' &&
            buf2[10] == '\0' && buf3[10] == '\0');

        sql << "delete from test2";
    }

    {
        char msg[] = "Hello, Firebird!";
        char buf1[100], buf2[100], buf3[100];
        strcpy(buf1, msg);

        sql << "insert into test2(p1, p2) values (?,?)",
        use(buf1), use(buf1);
        sql << "select p1, p2 from test2", into(buf2), into(buf3);

        assert(
            buf2[0] == 'H' && buf3[0] == 'H' &&
            buf2[1] == 'e' && buf3[1] == 'e' &&
            buf2[2] == 'l' && buf3[2] == 'l' &&
            buf2[3] == 'l' && buf3[3] == 'l' &&
            buf2[4] == 'o' && buf3[4] == 'o' &&
            buf2[5] == ',' && buf3[5] == ',' &&
            buf2[6] == ' ' && buf3[6] == ' ' &&
            buf2[7] == 'F' && buf3[7] == 'F' &&
            buf2[8] == 'i' && buf3[8] == 'i' &&
            buf2[9] == 'r' && buf3[9] == 'r' &&
            buf2[10] == '\0' && buf3[10] == '\0');

        sql << "delete from test2";
    }

    {
        // verify blank padding in CHAR fields
        // In Firebird, CHAR fields are always padded with whitespaces.
        char msg[] = "Hello";
        sql << "insert into test2(p1) values(\'" << msg << "\')";

        char buf[20];
        sql << "select p1 from test2", into(buf);

        assert(std::strncmp(buf, msg, 5) == 0);
        assert(std::strncmp(buf+5, "     ", 5) == 0);

        sql << "delete from test2";
    }

    {
        std::string str1("Hello, Firebird!"), str2, str3;
        sql << "insert into test2(p1, p2) values (?, ?)",
        use(str1), use(str1);

        sql << "select p1, p2 from test2", into(str2), into(str3);
        assert(str2 == "Hello, Fir" && str3 == "Hello, Fir");

        sql << "delete from test2";
    }

    sql << "drop table test2";
    std::cout << "test 2 passed" << std::endl;
}

// date and time
void test3()
{
    Session sql(backEnd, connectString);

    try
    {
        sql << "drop table test3";
    }
    catch (SOCIError const &)
    {} // ignore if error

    sql << "create table test3 (p1 timestamp, p2 date, p3 time)";
    sql.commit();

    sql.begin();

    std::tm t1, t2, t3;
    std::time_t now = std::time(NULL);
    std::tm t = *std::localtime(&now);

    sql << "insert into test3(p1, p2, p3) "
    << "values (?,?,?)", use(t), use(t), use(t);

    sql << "select p1, p2, p3 from test3", into(t1), into(t2), into(t3);

    // timestamp
    assert(t1.tm_year == t.tm_year);
    assert(t1.tm_mon  == t.tm_mon);
    assert(t1.tm_mday == t.tm_mday);
    assert(t1.tm_hour == t.tm_hour);
    assert(t1.tm_min  == t.tm_min);
    assert(t1.tm_sec  == t.tm_sec);

    // date
    assert(t2.tm_year == t.tm_year);
    assert(t2.tm_mon  == t.tm_mon);
    assert(t2.tm_mday == t.tm_mday);
    assert(t2.tm_hour == 0);
    assert(t2.tm_min  == 0);
    assert(t2.tm_sec  == 0);

    // time
    assert(t3.tm_year == 0);
    assert(t3.tm_mon  == 0);
    assert(t3.tm_mday == 0);
    assert(t3.tm_hour == t.tm_hour);
    assert(t3.tm_min  == t.tm_min);
    assert(t3.tm_sec  == t.tm_sec);

    sql << "drop table test3";
    std::cout << "test 3 passed" << std::endl;
}

// floating points
void test4()
{
    Session sql(backEnd, connectString);

    try
    {
        sql << "drop table test4";
    }
    catch (SOCIError const &)
    {} // ignore if error

    sql << "create table test4 (p1 numeric(8,2), "
    << "p2 decimal(14,8), p3 double precision, p4 integer)";
    sql.commit();

    sql.begin();

    double d1 = 1234.23, d2 = 1e8, d3 = 1.0/1440.0,
                                        d4, d5, d6;

    sql << "insert into test4(p1, p2, p3) values (?,?,?)",
    use(d1), use(d2), use(d3);

    sql << "select p1, p2, p3 from test4",
    into(d4), into(d5), into(d6);

    assert(d1 == d4 && d2 == d5 && d3 == d6);

    // verify an exception is thrown when fetching non-integral value
    // to integral variable
    try
    {
        int i;
        sql << "select p1 from test4", into(i);

        // expecting error
        assert(false);
    }
    catch (SOCIError const &e)
    {
        std::string error = e.what();
        assert(error ==
               "Can't convert value with scale 2 to integral type");
    }

    // verify an exception is thrown when inserting non-integral value
    // to integral column
    try
    {
        sql << "insert into test4(p4) values(?)", use(d1);

        // expecting error
        assert(false);
    }
    catch (SOCIError const &e)
    {
        std::string error = e.what();
        assert(error ==
               "Can't convert non-integral value to integral column type");
    }

    sql << "drop table test4";
    std::cout << "test 4 passed" << std::endl;
}

// integer types and indicators
void test5()
{
    Session sql(backEnd, connectString);

    {
        short sh(0);
        sql << "select 3 from rdb$database", into(sh);
        assert(sh == 3);
    }

    {
        int i(0);
        sql << "select 5 from rdb$database", into(i);
        assert(i == 5);
    }

    {
        unsigned long ul(0);
        sql << "select 7 from rdb$database", into(ul);
        assert(ul == 7);
    }

    {
        // test indicators
        eIndicator ind;
        int i;

        sql << "select 2 from rdb$database", into(i, ind);
        assert(ind == eOK);

        sql << "select NULL from rdb$database", into(i, ind);
        assert(ind == eNull);

        char buf[4];
        sql << "select \'Hello\' from rdb$database", into(buf, ind);
        assert(ind == eTruncated);

        sql << "select 5 from rdb$database where 0 = 1", into(i, ind);
        assert(ind == eNoData);

        try
        {
            // expect error
            sql << "select NULL from rdb$database", into(i);
            assert(false);
        }
        catch (SOCIError const &e)
        {
            std::string error = e.what();
            assert(error ==
                   "Null value fetched and no indicator defined.");
        }

        try
        {
            // expect error
            sql << "select 5 from rdb$database where 0 = 1", into(i);
            assert(false);
        }
        catch (SOCIError const &e)
        {
            std::string error = e.what();
            assert(error ==
                   "No data fetched and no indicator defined.");
        }
    }

    std::cout << "test 5 passed" << std::endl;
}

// repeated fetch and bulk operations for character types
void test6()
{
    Session sql(backEnd, connectString);

    try
    {
        sql << "drop table test6";
    }
    catch (SOCIError const &)
    {} // ignore if error

    sql << "create table test6 (p1 char(10), p2 varchar(10))";
    sql.commit();

    sql.begin();

    for (char c = 'a'; c <= 'z'; ++c)
    {
        sql << "insert into test6(p1, p2) values(?,?)", use(c), use(c);
    }

    {
        char c='a', c1, c2;

        Statement st = (sql.prepare <<
                        "select p1,p2 from test6 order by p1", into(c1), into(c2));

        st.execute();
        while (st.fetch())
        {
            assert(c == c1 && c == c2);
            ++c;
        }
        assert(c == 'z'+1);
    }

    {
        char c='a';

        std::vector<char> c1(10), c2(10);

        Statement st = (sql.prepare <<
                        "select p1,p2 from test6 order by p1", into(c1), into(c2));

        st.execute();
        while (st.fetch())
        {
            for (std::size_t i = 0; i != c1.size(); ++i)
            {
                assert(c == c1[i] && c == c2[i]);
                ++c;
            }
        }
        assert(c == 'z' + 1);
    }

    {
        // verify an exception is thrown when empty vector is used
        std::vector<char> vec;
        try
        {
            sql << "select p1 from test6", into(vec);
            assert(false);
        }
        catch (SOCIError const &e)
        {
            std::string msg = e.what();
            assert(msg == "Vectors of size 0 are not allowed.");
        }
    }

    sql << "delete from test6";

    // verifying std::string
    int const rowsToTest = 10;
    for (int i = 0; i != rowsToTest; ++i)
    {
        std::ostringstream ss;
        ss << "Hello_" << i;

        std::string const &x = ss.str();

        sql << "insert into test6(p1, p2) values(\'"
        << x << "\', \'" << x << "\')";
    }

    int count;
    sql << "select count(*) from test6", into(count);
    assert(count == rowsToTest);

    {
        int i = 0;
        std::string s1, s2;
        Statement st = (sql.prepare <<
                        "select p1, p2 from test6 order by p1", into(s1), into(s2));

        st.execute();
        while (st.fetch())
        {
            std::ostringstream ss;
            ss << "Hello_" << i;
            std::string const &x = ss.str();

            // Note: CHAR fields are always padded with whitespaces
            ss << "   ";
            assert(s1 == ss.str() && s2 == x);
            ++i;
        }
        assert(i == rowsToTest);
    }

    {
        int i = 0;

        std::vector<std::string> s1(4), s2(4);
        Statement st = (sql.prepare <<
                        "select p1, p2 from test6 order by p1", into(s1), into(s2));
        st.execute();
        while (st.fetch())
        {
            for (std::size_t j = 0; j != s1.size(); ++j)
            {
                std::ostringstream ss;
                ss << "Hello_" << i;
                std::string const &x = ss.str();

                // Note: CHAR fields are always padded with whitespaces
                ss << "   ";
                assert(ss.str() == s1[j] && x == s2[j]);
                ++i;
            }
        }
        assert(i == rowsToTest);
    }

    sql << "drop table test6";
    std::cout << "test 6 passed" << std::endl;
}

// BLOB test
void test7()
{
    Session sql(backEnd, connectString);

    try
    {
        sql << "drop table test7";
    }
    catch (std::runtime_error &)
    {} // ignore if error

    sql << "create table test7(id integer, img blob)";
    sql.commit();

    sql.begin();
    {
        // verify empty blob
        BLOB b(sql);
        eIndicator ind;

        sql << "insert into test7(id, img) values(1,?)", use(b);
        sql << "select img from test7 where id = 1", into(b, ind);

        assert(ind == eOK);
        assert(b.getLen() == 0);

        sql << "delete from test7";
    }

    {
        // create a new blob
        BLOB b(sql);

        char str1[] = "Hello";
        b.write(0, str1, strlen(str1));

        char str2[20];
        std::size_t i = b.read(3,str2,2);
        str2[i] = '\0';
        assert(str2[0] == 'l' && str2[1] == 'o' && str2[2] == '\0');

        char str3[] = ", Firebird!";
        b.append(str3, strlen(str3));

        sql << "insert into test7(id, img) values(1,?)", use(b);
    }

    {
        // read & update blob
        BLOB b(sql);

        sql << "select img from test7 where id = 1", into(b);

        std::vector<char> text(b.getLen());
        b.read(0, &text[0], b.getLen());
        assert(strncmp(&text[0], "Hello, Firebird!", b.getLen()) == 0);

        char str1[] = "FIREBIRD";
        b.write(7, str1, strlen(str1));

        // after modification BLOB must be written to database
        sql << "update test7 set img=? where id=1", use(b);
    }

    {
        // read BLOB from database, modify and write to another record
        BLOB b(sql);

        sql << "select img from test7 where id = 1", into(b);

        std::vector<char> text(b.getLen());
        b.read(0, &text[0], b.getLen());

        char str1[] = "HELLO";
        b.write(0,str1, strlen(str1));

        b.read(0, &text[0], b.getLen());
        assert(strncmp(&text[0], "HELLO, FIREBIRD!", b.getLen()) == 0);

        b.trim(5);
        sql << "insert into test7(id, img) values(2,?)", use(b);
    }

    {
        BLOB b(sql);
        Statement st = (sql.prepare << "select img from test7", into(b));

        st.execute();

        st.fetch();
        std::vector<char> text(b.getLen());
        b.read(0, &text[0], b.getLen());
        assert(strncmp(&text[0], "Hello, FIREBIRD!", b.getLen()) == 0);

        st.fetch();
        text.resize(b.getLen());
        b.read(0, &text[0], b.getLen());
        assert(strncmp(&text[0], "HELLO", b.getLen()) == 0);
    }

    {
        // delete BLOB
        BLOB b(sql);
        eIndicator ind=eNull;
        sql << "update test7 set img=? where id = 1", use(b, ind);

        sql << "select img from test7 where id = 2", into(b, ind);
        assert(ind==eOK);

        sql << "select img from test7 where id = 1", into(b, ind);
        assert(ind==eNull);
    }

    sql << "drop table test7";
    std::cout << "test 7 passed" << std::endl;
}

// named parameters
void test8()
{
    Session sql(backEnd, connectString);

    try
    {
        sql << "drop table test8";
    }
    catch (std::runtime_error &)
    {} // ignore if error

    sql << "create table test8(id1 integer, id2 integer)";
    sql.commit();

    sql.begin();

    int j = 13, k = 4, i, l;
    sql << "insert into test8(id1, id2) values(:id1, :id2)",
    use(k, "id2"), use(j, "id1");
    sql << "select id1, id2 from test8", into(i), into(l);
    assert(i == j && l == k);

    sql << "delete from test8";

    std::vector<int> in1(3), in2(3);
    in1[0] = 3;
    in1[1] = 2;
    in1[2] = 1;
    in2[0] = 4;
    in2[1] = 5;
    in2[2] = 6;

    {
        Statement st = (sql.prepare <<
                        "insert into test8(id1, id2) values(:id1, :id2)",
                        use(k, "id2"), use(j, "id1"));

        std::size_t s = in1.size();
        for (std::size_t x = 0; x < s; ++x)
        {
            j = in1[x];
            k = in2[x];
            st.execute();
        }
    }

    {
        Statement st = (
                           sql.prepare << "select id1, id2 from test8", into(i), into(l));
        st.execute();

        std::size_t x(0);
        while (st.fetch())
        {
            assert(i = in1[x] && l == in2[x]);
            ++x;
        }
    }

    sql << "delete from test8";

    // test vectors
    sql << "insert into test8(id1, id2) values(:id1, :id2)",
    use(in1, "id1"), use(in2, "id2");

    std::vector<int> out1(3), out2(3);

    sql << "select id1, id2 from test8", into(out1), into(out2);
    std::size_t s = out1.size();
    assert(s == 3);

    for (std::size_t x = 0; x<s; ++x)
    {
        assert(out1[x] == in1[x] && out2[x] == in2[x]);
    }

    sql << "drop table test8";
    std::cout << "test 8 passed" << std::endl;
}

// Dynamic binding to Row objects
void test9()
{
    Session sql(backEnd, connectString);

    try
    {
        sql << "drop table test9";
    }
    catch (std::runtime_error &)
    {} // ignore if error

    sql << "create table test9(id integer, msg varchar(20), ntest numeric(10,2))";
    sql.commit();

    sql.begin();

    {
        Row r;
        sql << "select * from test9", into(r);
        assert(r.indicator(0) ==  eNoData);
    }

    std::string msg("Hello");
    int i(1);
    double d(3.14);
    eIndicator ind(eOK);

    {
        Statement st((sql.prepare << "insert into test9(id, msg, ntest) "
                      << "values(:id,:msg,:ntest)",
                      use(i,"id"), use(msg,"msg"), use(d,ind,"ntest")));

        st.execute(1);

        i = 2;
        msg = "Firebird";
        ind = eNull;
        st.execute(1);
    }

    Row r;
    Statement st = (sql.prepare <<
                    "select * from test9", into(r));
    st.execute(1);

    assert(r.size() == 3);

    // get properties by position
    assert(r.getProperties(0).getName() == "ID");
    assert(r.getProperties(1).getName() == "MSG");
    assert(r.getProperties(2).getName() == "NTEST");

    assert(r.getProperties(0).getDataType() == eInteger);
    assert(r.getProperties(1).getDataType() == eString);
    assert(r.getProperties(2).getDataType() == eDouble);

    // get properties by name
    assert(r.getProperties("ID").getName() == "ID");
    assert(r.getProperties("MSG").getName() == "MSG");
    assert(r.getProperties("NTEST").getName() == "NTEST");

    assert(r.getProperties("ID").getDataType() == eInteger);
    assert(r.getProperties("MSG").getDataType() == eString);
    assert(r.getProperties("NTEST").getDataType() == eDouble);

    // get values by position
    assert(r.get<int>(0) == 1);
    assert(r.get<std::string>(1) == "Hello");
    assert(r.get<double>(2) == d);

    // get values by name
    assert(r.get<int>("ID") == 1);
    assert(r.get<std::string>("MSG") == "Hello");
    assert(r.get<double>("NTEST") == d);

    st.fetch();
    assert(r.get<int>(0) == 2);
    assert(r.get<std::string>("MSG") == "Firebird");
    assert(r.indicator(2) == eNull);

    // verify default values
    assert(r.get<double>("NTEST", 2) == 2);
    bool caught = false;
    try
    {
        double d1 = r.get<double>("NTEST");
        std::cout << d1 << std::endl;     // just for compiler
    }
    catch (SOCIError&)
    {
        caught = true;
    }
    assert(caught);

    // verify exception thrown on invalid get<>
    caught = false;
    try
    {
        r.get<std::string>(0);
    }
    catch (std::bad_cast const &)
    {
        caught = true;
    }
    assert(caught);

    sql << "drop table test9";
    std::cout << "test 9 passed" << std::endl;
}

// stored procedures
void test10()
{
    Session sql(backEnd, connectString);

    try
    {
        sql << "drop procedure sp_test10";
    }
    catch (std::runtime_error &)
    {} // ignore if error

    try
    {
        sql << "drop procedure sp_test10a";
    }
    catch (std::runtime_error &)
    {} // ignore if error

    try
    {
        sql << "drop table test10";
    }
    catch (std::runtime_error &)
    {} // ignore if error

    sql << "create table test10(id integer, id2 integer)";

    sql << "create procedure sp_test10\n"
    << "returns (rid integer, rid2 integer)\n"
    << "as begin\n"
    << "for select id, id2 from test10 into rid, rid2 do begin\n"
    << "suspend;\n"
    << "end\n"
    << "end;\n";

    sql << "create procedure sp_test10a (pid integer, pid2 integer)\n"
    << "as begin\n"
    << "insert into test10(id, id2) values (:pid, :pid2);\n"
    << "end;\n";

    sql.commit();

    sql.begin();

    Row r;
    int p1 = 3, p2 = 4;

    // calling procedures that do not return values requires
    // 'execute procedure ...' statement
    sql << "execute procedure sp_test10a ?, ?", use(p1), use(p2);

    // calling procedures that return values requires
    // 'select ... from ...' statement
    sql << "select * from sp_test10", into(r);

    assert(r.get<int>(0) == p1 && r.get<int>(1) == p2);

    sql << "delete from test10";

    p1 = 5;
    p2 = 6;
    {
        Procedure proc = (
                             sql.prepare << "sp_test10a :p1, :p2",
                             use(p2, "p2"), use(p1, "p1"));
        proc.execute(1);
    }

    {
        Row rw;
        Procedure proc = (sql.prepare << "sp_test10", into(rw));
        proc.execute(1);

        assert(rw.get<int>(0) == p1 && rw.get<int>(1) == p2);
    }

    sql << "delete from test10";

    // test vectors
    std::vector<int> in1(3), in2(3);
    in1[0] = 3;
    in1[1] = 2;
    in1[2] = 1;
    in2[0] = 4;
    in2[1] = 5;
    in2[2] = 6;

    {
        Procedure proc = (
                             sql.prepare << "sp_test10a :p1, :p2",
                             use(in2, "p2"), use(in1, "p1"));
        proc.execute(1);
    }

    {
        Row rw;
        Procedure proc = (sql.prepare << "sp_test10", into(rw));

        proc.execute(1);
        assert(rw.get<int>(0) == in1[0] && rw.get<int>(1) == in2[0]);
        proc.fetch();
        assert(rw.get<int>(0) == in1[1] && rw.get<int>(1) == in2[1]);
        proc.fetch();
        assert(rw.get<int>(0) == in1[2] && rw.get<int>(1) == in2[2]);
        assert(proc.fetch() == false);
    }

    {
        std::vector<int> out1(3), out2(3);
        Procedure proc = (sql.prepare << "sp_test10", into(out1), into(out2));
        proc.execute(1);

        std::size_t s = out1.size();
        assert(s == 3);

        for (std::size_t x = 0; x < s; ++x)
        {
            assert(out1[x] == in1[x] && out2[x] == in2[x]);
        }
    }

    sql.rollback();

    sql.begin();
    sql << "drop procedure sp_test10";
    sql << "drop procedure sp_test10a";
    sql << "drop table test10";

    std::cout << "test 10 passed" << std::endl;
}

// direct access to Firebird using handles exposed by
// SOCI::FirebirdStatmentBackend
namespace SOCI
{
    enum eRowCountType {
        eRowsSelected = isc_info_req_select_count,
        eRowsInserted = isc_info_req_insert_count,
        eRowsUpdated  = isc_info_req_update_count,
        eRowsDeleted  = isc_info_req_delete_count
    };

    // Returns number of rows afected by last statement
    // or -1 if there is no such counter available.
    long getRowCount(SOCI::Statement & statement, eRowCountType type)
    {
        ISC_STATUS stat[20];
        char cnt_req[2], cnt_info[128];

        cnt_req[0]=isc_info_sql_records;
        cnt_req[1]=isc_info_end;

        FirebirdStatementBackEnd *statementBackEnd
        = static_cast<FirebirdStatementBackEnd *>(statement.getBackEnd());

        // Note: This is very poorly documented function.
        // It can extract number of rows returned by select statement,
        // but it appears that this is only number of rows prefetched by
        // client library, not total number of selected rows.
        if (isc_dsql_sql_info(stat, &statementBackEnd->stmtp_, sizeof(cnt_req),
                              cnt_req, sizeof(cnt_info), cnt_info))
        {
            SOCI::details::Firebird::throwISCError(stat);
        }

        long count = -1;
        char type_ = static_cast<char>(type);
        for (char *ptr = cnt_info + 3; *ptr != isc_info_end;)
        {
            char count_type = *ptr++;
            int l = isc_vax_integer(ptr, 2);
            ptr += 2;
            count = isc_vax_integer(ptr, l);

            if (count_type == type_)
            {
                // this is requested number
                break;
            }
            ptr += l;
        }

        return count;
    }
} // namespace SOCI

void test11()
{
    Session sql(backEnd, connectString);

    try
    {
        sql << "drop table test11";
    }
    catch (std::runtime_error &)
    {} // ignore if error

    sql << "create table test11(id integer)";
    sql.commit();

    sql.begin();

    {
        std::vector<int> in(3);
        in[0] = 3;
        in[1] = 2;
        in[2] = 1;

        Statement st = (sql.prepare << "insert into test11(id) values(?)",
                        use(in));
        st.execute(1);

        // Note: Firebird backend inserts every row with separate insert
        // statement to achieve the effect of inserting vectors of values.
        // Since getRowCount() returns number of rows affected by the *last*
        // statement, it will return 1 here.
        assert(getRowCount(st, eRowsInserted) == 1);
    }

    {
        int i = 5;
        Statement st = (sql.prepare << "update test11 set id = ? where id<3",
                        use(i));
        st.execute(1);
        assert(getRowCount(st, eRowsUpdated) == 2);

        // verify that no rows were deleted
        assert(getRowCount(st, eRowsDeleted) == 0);
    }

    {
        std::vector<int> out(3);
        Statement st = (sql.prepare << "select id from test11", into(out));
        st.execute(1);

        assert(getRowCount(st, eRowsSelected) == 3);
    }

    {
        Statement st = (sql.prepare << "delete from test11 where id=10");
        st.execute(1);
        assert(getRowCount(st, eRowsDeleted) == 0);
    }

    {
        Statement st = (sql.prepare << "delete from test11");
        st.execute(1);
        assert(getRowCount(st, eRowsDeleted) == 3);
    }

    sql << "drop table test11";
    std::cout << "test 11 passed" << std::endl;
}

//
// Support for SOCI Common Tests
//

struct TableCreator1 : public TableCreatorBase
{
    TableCreator1(Session& session)
            : TableCreatorBase(session)
    {
        session << "create table soci_test(id integer, val integer, c char, "
        "str varchar(20), sh smallint, ul decimal(9,0), d double precision, "
        "tm timestamp, i1 integer, i2 integer, i3 integer, name varchar(20))";
        session.commit();
        session.begin();
    }
};

struct TableCreator2 : public TableCreatorBase
{
    TableCreator2(Session& session)
            : TableCreatorBase(session)
    {
        session  << "create table soci_test(\"num_float\" float, \"num_int\" integer, "
        "\"name\" varchar(20), \"sometime\" timestamp, \"chr\" char)";
        session.commit();
        session.begin();
    }
};

struct TableCreator3 : public TableCreatorBase
{
    TableCreator3(Session& session)
            : TableCreatorBase(session)
    {
        // CommonTest uses lower-case column names,
        // so we need to enforce such names here.
        // That's why column names are enclosed in ""
        session << "create table soci_test(\"name\" varchar(100) not null, "
        "\"phone\" varchar(15))";
        session.commit();
        session.begin();
    }
};

class TestContext :public TestContextBase
{
    public:
        TestContext(BackEndFactory const &backEnd,
                    std::string const &connectString)
                : TestContextBase(backEnd, connectString)
        {}

        TableCreatorBase* tableCreator1(Session& s) const
        {
            return new TableCreator1(s);
        }

        TableCreatorBase* tableCreator2(Session& s) const
        {
            return new TableCreator2(s);
        }

        TableCreatorBase* tableCreator3(Session& s) const
        {
            return new TableCreator3(s);
        }

        std::string toDateTime(std::string const &dateString) const
        {
            return "'" + dateString + "'";
        }
};


int main(int argc, char** argv)
{

#ifdef _MSC_VER
    // Redirect errors, unrecoverable problems, and assert() failures to STDERR,
    // instead of debug message window.
    // This hack is required to run asser()-driven tests by Buildbot.
    // NOTE: Comment this 2 lines for debugging with Visual C++ debugger to catch assertions inside.
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);
#endif //_MSC_VER

    if (argc == 2)
    {
        connectString = argv[1];
    }
    else
    {
        std::ostringstream msg;
        msg << "usage: " << argv[0]
        << " connectstring\n"
        << "example: " << argv[0]
        << " \"service=/usr/local/firebird/db/test.fdb user=SYSDBA password=masterkey\"\n";

        std::cout << msg.str().c_str();
        exit(1);
    }

    try
    {
        TestContext tc(backEnd, connectString);
        CommonTests tests(tc);
        tests.run();

        test1();
        test2();
        test3();
        test4();
        test5();
        test6();
        test7();
        test8();
        test9();
        test10();
        test11();

        std::cout << "\nOK, all tests passed.\n\n";
    }
    catch (std::exception const & e)
    {
        std::cout << e.what() << '\n';
    }
}
