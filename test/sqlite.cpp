#include <cstdio>
#include <utility>
#include "sqlite3.h"
#include "sqlite.h"

SQLiteDB::SQLiteDB()
    : m_path()
    , m_sqlite(nullptr)
{

}

SQLiteDB::~SQLiteDB()
{
    close();
}

bool SQLiteDB::open(const char * path)
{
    if (nullptr == path)
    {
        printf("open sqlite failed while path is nullptr\n");
        return (false);
    }

    close();

    m_path = path;

    int result = sqlite3_open(path, &m_sqlite);
    if (SQLITE_OK != result)
    {
        printf("open sqlite failed while sqlite open (%s) failed, error (%d: %s)\n", path, result, sqlite3_errstr(result));
        close();
        return (false);
    }

    return (true);
}

bool SQLiteDB::close()
{
    int result = SQLITE_OK;
    if (nullptr != m_sqlite)
    {
        int result = sqlite3_close(m_sqlite);
        if (SQLITE_OK != result)
        {
            printf("close sqlite (%s) failed, error (%d: %s)\n", m_path.c_str(), result, sqlite3_errstr(result));
        }
        m_sqlite = nullptr;
        m_path.clear();
    }
    return (SQLITE_OK == result);
}

bool SQLiteDB::is_open() const
{
    return (nullptr != m_sqlite);
}

int SQLiteDB::error() const
{
    return (nullptr != m_sqlite ? sqlite3_errcode(m_sqlite) : 999);
}

const char * SQLiteDB::what() const
{
    return (nullptr != m_sqlite ? sqlite3_errmsg(m_sqlite) : "sqlite is closed");
}

bool SQLiteDB::execute(const char * operate, const char * sql)
{
    if (nullptr == operate)
    {
        operate = "execute sql";
    }

    if (nullptr == sql)
    {
        printf("%s failed while sql is nullptr\n", operate);
        return (false);
    }

    if (nullptr == m_sqlite)
    {
        printf("%s failed while sqlite is closed\n", operate);
        return (false);
    }

    char * error_message = nullptr;
    int result = sqlite3_exec(m_sqlite, sql, nullptr, nullptr, &error_message);
    if (SQLITE_OK != result)
    {
        printf("%s failed while sqlite exec (%s) failed, error (%d: %s) message (%s)\n", operate, sql, result, sqlite3_errstr(result), nullptr != error_message ? error_message : "unknown\n");
    }
    if (nullptr != error_message)
    {
        sqlite3_free(error_message);
        error_message = nullptr;
    }

    return (SQLITE_OK == result);
}

bool SQLiteDB::begin_transaction()
{
    return (execute("begin transaction", "BEGIN TRANSACTION;"));
}

bool SQLiteDB::end_transaction()
{
    return (execute("end transaction", "END TRANSACTION;"));
}

SQLiteReader SQLiteDB::create_reader(const char * sql)
{
    return (SQLiteReader(m_sqlite, sql));
}

SQLiteWriter SQLiteDB::create_writer(const char * sql)
{
    return (SQLiteWriter(m_sqlite, sql));
}

SQLiteStatement::SQLiteStatement()
    : m_sql()
    , m_sqlite(nullptr)
    , m_statement(nullptr)
{

}

SQLiteStatement::SQLiteStatement(sqlite3 * sqlite, const char * sql)
    : m_sql()
    , m_sqlite(nullptr)
    , m_statement(nullptr)
{
    if (nullptr == sqlite)
    {
        printf("create statement failed while sqlite is nullptr\n");
    }
    else if (nullptr == sql)
    {
        printf("create statement failed while sql is nullptr\n");
    }
    else
    {
        m_sql = sql;
        m_sqlite = sqlite;
        int result = sqlite3_prepare_v2(m_sqlite, m_sql.c_str(), static_cast<int>(m_sql.size()), &m_statement, nullptr);
        if (SQLITE_OK != result)
        {
            printf("create statement failed while sqlite (%s) prepare failed, error (%d: %s)\n", m_sql.c_str(), result, sqlite3_errstr(result));
            m_sql.clear();
            m_sqlite = nullptr;
            m_statement = nullptr;
        }
    }
}

SQLiteStatement::SQLiteStatement(SQLiteStatement & other)
    : m_sql()
    , m_sqlite(nullptr)
    , m_statement(nullptr)
{
    std::swap(m_sql, other.m_sql);
    std::swap(m_sqlite, other.m_sqlite);
    std::swap(m_statement, other.m_statement);
}

SQLiteStatement & SQLiteStatement::operator = (SQLiteStatement & other)
{
    if (&other != this)
    {
        clear();
        std::swap(m_sql, other.m_sql);
        std::swap(m_sqlite, other.m_sqlite);
        std::swap(m_statement, other.m_statement);
    }
    return (*this);
}

SQLiteStatement::~SQLiteStatement()
{
    clear();
}

void SQLiteStatement::clear()
{
    if (nullptr != m_statement)
    {
        int result = sqlite3_finalize(m_statement);
        if (SQLITE_OK != result)
        {
            printf("destroy statement failed while sqlite (%s) finalize failed, error (%d: %s)\n", m_sql.c_str(), result, sqlite3_errstr(result));
        }
        m_sql.clear();
        m_sqlite = nullptr;
        m_statement = nullptr;
    }
}

bool SQLiteStatement::good() const
{
    return (nullptr != m_statement);
}

bool SQLiteStatement::reset()
{
    if (nullptr == m_statement)
    {
        printf("reset failed while statement is nullptr\n");
        return (false);
    }

    int result = sqlite3_reset(m_statement);
    if (SQLITE_OK != result)
    {
        printf("reset failed while sqlite (%s) reset failed, error (%d: %s)\n", m_sql.c_str(), result, sqlite3_errstr(result));
        return (false);
    }

    return (true);
}

bool SQLiteStatement::bind(int field_index, int field_value)
{
    if (nullptr == m_statement)
    {
        printf("bind failed while statement is nullptr\n");
        return (false);
    }

    int result = sqlite3_bind_int(m_statement, field_index + 1, field_value);
    if (SQLITE_OK != result)
    {
        printf("bind failed while sqlite (%s) bind int failed, error (%d: %s)\n", m_sql.c_str(), result, sqlite3_errstr(result));
        return (false);
    }

    return (true);
}

bool SQLiteStatement::bind(int field_index, int64_t field_value)
{
    if (nullptr == m_statement)
    {
        printf("bind failed while statement is nullptr\n");
        return (false);
    }

    int result = sqlite3_bind_int64(m_statement, field_index + 1, field_value);
    if (SQLITE_OK != result)
    {
        printf("bind failed while sqlite (%s) bind int64 failed, error (%d: %s)\n", m_sql.c_str(), result, sqlite3_errstr(result));
        return (false);
    }

    return (true);
}

bool SQLiteStatement::bind(int field_index, double field_value)
{
    if (nullptr == m_statement)
    {
        printf("bind failed while statement is nullptr\n");
        return (false);
    }

    int result = sqlite3_bind_double(m_statement, field_index + 1, field_value);
    if (SQLITE_OK != result)
    {
        printf("bind failed while sqlite (%s) bind double failed, error (%d: %s)\n", m_sql.c_str(), result, sqlite3_errstr(result));
        return (false);
    }

    return (true);
}

bool SQLiteStatement::bind(int field_index, const std::string & field_value)
{
    if (nullptr == m_statement)
    {
        printf("bind failed while statement is nullptr\n");
        return (false);
    }

    int result = sqlite3_bind_text(m_statement, field_index + 1, field_value.c_str(), static_cast<int>(field_value.size()), SQLITE_TRANSIENT);
    if (SQLITE_OK != result)
    {
        printf("bind failed while sqlite (%s) bind text failed, error (%d: %s)\n", m_sql.c_str(), result, sqlite3_errstr(result));
        return (false);
    }

    return (true);
}

SQLiteReader::SQLiteReader()
    : SQLiteStatement()
{

}

SQLiteReader::SQLiteReader(sqlite3 * db, const char * sql)
    : SQLiteStatement(db, sql)
{

}

bool SQLiteReader::read()
{
    if (nullptr == m_statement)
    {
        printf("read failed while statement is nullptr\n");
        return (false);
    }

    int result = sqlite3_step(m_statement);
    if (SQLITE_ROW != result && SQLITE_DONE != result)
    {
        printf("read failed while sqlite (%s) step failed, error (%d: %s, %d: %s)\n", m_sql.c_str(), result, sqlite3_errstr(result), sqlite3_errcode(m_sqlite), sqlite3_errmsg(m_sqlite));
    }
    return (SQLITE_ROW == result);
}

bool SQLiteReader::column(int column_index, int & column_value)
{
    if (nullptr == m_statement)
    {
        printf("column failed while statement is nullptr\n");
        return (false);
    }

    if (column_index >= sqlite3_column_count(m_statement))
    {
        printf("column failed while sqlite (%s) column index is overflow (%d >= %d)\n", m_sql.c_str(), column_index, sqlite3_column_count(m_statement));
        return (false);
    }

    column_value = sqlite3_column_int(m_statement, column_index);
    return (true);
}

bool SQLiteReader::column(int column_index, int64_t & column_value)
{
    if (nullptr == m_statement)
    {
        printf("column failed while statement is nullptr\n");
        return (false);
    }

    if (column_index >= sqlite3_column_count(m_statement))
    {
        printf("column failed while sqlite (%s) column index is overflow (%d >= %d)\n", m_sql.c_str(), column_index, sqlite3_column_count(m_statement));
        return (false);
    }

    column_value = sqlite3_column_int64(m_statement, column_index);
    return (true);
}

bool SQLiteReader::column(int column_index, double & column_value)
{
    if (nullptr == m_statement)
    {
        printf("column failed while statement is nullptr\n");
        return (false);
    }

    if (column_index >= sqlite3_column_count(m_statement))
    {
        printf("column failed while sqlite (%s) column index is overflow (%d >= %d)\n", m_sql.c_str(), column_index, sqlite3_column_count(m_statement));
        return (false);
    }

    column_value = sqlite3_column_double(m_statement, column_index);
    return (true);
}

bool SQLiteReader::column(int column_index, std::string & column_value)
{
    if (nullptr == m_statement)
    {
        printf("column failed while statement is nullptr\n");
        return (false);
    }

    if (column_index >= sqlite3_column_count(m_statement))
    {
        printf("column failed while sqlite (%s) column index is overflow (%d >= %d)\n", m_sql.c_str(), column_index, sqlite3_column_count(m_statement));
        return (false);
    }

    const char * column_text = reinterpret_cast<const char *>(sqlite3_column_text(m_statement, column_index));
    int column_size = sqlite3_column_bytes(m_statement, column_index);
    column_value.assign(column_text, column_size);
    return (true);
}

SQLiteWriter::SQLiteWriter()
    : SQLiteStatement()
{

}

SQLiteWriter::SQLiteWriter(sqlite3 * db, const char * sql)
    : SQLiteStatement(db, sql)
{

}

bool SQLiteWriter::write()
{
    if (nullptr == m_statement)
    {
        printf("write failed while statement is nullptr\n");
        return (false);
    }

    int result = sqlite3_step(m_statement);
    if (SQLITE_ERROR == result || SQLITE_MISUSE == result)
    {
        printf("write failed while sqlite (%s) step failed, error (%d: %s, %d: %s)\n", m_sql.c_str(), result, sqlite3_errstr(result), sqlite3_errcode(m_sqlite), sqlite3_errmsg(m_sqlite));
        return (false);
    }

    return (true);
}
