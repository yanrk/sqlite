#ifndef SQLITE_H
#define SQLITE_H


#include <cstdint>
#include <string>

struct sqlite3;
struct sqlite3_stmt;

class SQLiteReader;
class SQLiteWriter;

class SQLiteDB
{
public:
    SQLiteDB();
    ~SQLiteDB();

public:
    bool open(const char * path);
    bool close();

public:
    bool is_open() const;
    int error() const;
    const char * what() const;

public:
    bool execute(const char * operate, const char * sql);
    bool begin_transaction();
    bool end_transaction();

public:
    SQLiteReader create_reader(const char * sql);
    SQLiteWriter create_writer(const char * sql);

private:
    std::string                             m_path;
    sqlite3                               * m_sqlite;
};

class SQLiteStatement
{
public:
    SQLiteStatement();
    SQLiteStatement(sqlite3 * sqlite, const char * sql);
    SQLiteStatement(SQLiteStatement & other);
    SQLiteStatement & operator = (SQLiteStatement & other);
    ~SQLiteStatement();

public:
    bool good() const;
    bool reset();
    bool bind(int field_index, int field_value);
    bool bind(int field_index, int64_t field_value);
    bool bind(int field_index, double field_value);
    bool bind(int field_index, const std::string & field_value);
    void clear();

protected:
    std::string                             m_sql;
    sqlite3                               * m_sqlite;
    sqlite3_stmt                          * m_statement;
};

class SQLiteReader : public SQLiteStatement
{
public:
    SQLiteReader();
    SQLiteReader(sqlite3 * db, const char * sql);

public:
    bool read();
    bool column(int column_index, int & column_value);
    bool column(int column_index, int64_t & column_value);
    bool column(int column_index, double & column_value);
    bool column(int column_index, std::string & column_value);
};

class SQLiteWriter : public SQLiteStatement
{
public:
    SQLiteWriter();
    SQLiteWriter(sqlite3 * db, const char * sql);

public:
    bool write();
};


#endif // SQLITE_H
