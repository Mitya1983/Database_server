#ifndef SQLITE_WRAPPER_H
#define SQLITE_WRAPPER_H
#include <sqlite3.h>
#include <exception>
#include <string>
#include <queue>
#include <map>
#include <vector>
#include <memory>

class Sqlite3Exception : public std::exception
{
    std::string _msg;
public:
    Sqlite3Exception(const std::string &databaseName, const std::string &details, const std::string &msg);
    virtual const char *what() const noexcept;
};

class CreateDatabaseException : public std::exception
{
    std::string _msg;
public:
    CreateDatabaseException(const std::string &msg);
    virtual const char *what() const noexcept;
};
class TableException : public std::exception
{
    std::string _msg;
public:
    TableException(const std::string &databaseName, const std::string &tableName, const std::string &msg);
    virtual const char *what() const noexcept;
};
class ColumnException : public std::exception
{
    std::string _msg;
public:
    ColumnException(const std::string &databaseName, const std::string &tableName,
                    const std::string &columnName, const std::string &msg);
    virtual const char *what() const noexcept;
};
class InsertException : std::exception
{
    std::string _msg;
public:
    InsertException(const std::string &databaseName, const std::string &tableName, const std::string &msg);
    virtual const char *what() const noexcept;
};
class SelectException : public std::exception
{
    std::string _msg;
public:
    SelectException(const std::string &databaseName, const std::string &tableName, const std::string &msg);
    virtual const char *what() const noexcept;
};


struct resultColumn
{
    std::string name;
    std::vector<std::string> values;
};
using Query = std::vector<resultColumn>;
using ParamVector = const std::vector<std::string>;
using ParamString = const std::string;


class Sqlite_wrapper
{
    struct Column
    {
        std::string name;
        std::string type;
        std::string defaultValue;
        bool isPK;
        bool isUnique;
        bool isNullable;
        bool isDefaultValue;
        std::string getQuery();
        void clear();
    };
    struct ForeignKey
    {
        std::string column;
        std::string refTable;
        std::string refColumn;
        std::string getQuery();
        void clear();
    };

    struct Table
    {
        std::shared_ptr<std::string> databaseName;
        std::string name;
        std::queue<Column> columns;
        bool pKisSet;
        bool isForeignKey;
        std::queue<ForeignKey> foreignKeys;
        bool noRowID;
        std::string getQuery();
        void clear();
    };

    using Query = std::vector<resultColumn>;
    std::shared_ptr<std::string> name;
    static Query _result;
    Query result;
    std::string _query;
    static bool firstQuery;
    static int callback(void*, int argc, char **argv, char **azColName);
    char *sqlite3Errmsg;
    Sqlite_wrapper();
    Sqlite_wrapper(const Sqlite_wrapper &other) = delete;
    Sqlite_wrapper(const Sqlite_wrapper &&other) = delete;
    Sqlite_wrapper& operator=(const Sqlite_wrapper &other) = delete;
    Sqlite_wrapper& operator=(const Sqlite_wrapper &&other) = delete;
    sqlite3 *db;
    Column curColumn;
    bool currentColumn;
    Table curTable;
    bool currentTable;
    void _modifyingExec(ParamString &query);
    void _readExec(ParamString &query);
    void _createDatabase(ParamString &fileName);
    void _createTable(ParamString &table);
    void _createColumn(ParamString &column, ParamString &type);
    void _setAsPK();
    void _setAsUnique();
    void _setAsNotNullable();
    void _setDefaultValue(ParamString &value);
    void _setNoRowID();//If Primary Key is not set the [Table name]ID column will be created with INTEGER type. Autoincrement will not work
    void _addColumn();
    void _setForeinKey(ParamString &column, ParamString &refTable, ParamString &refColumn);
    void _addTable();
    void _dropTable(ParamString &table);//TODO
    void _insertInto(ParamString &table, ParamVector &columns, ParamVector &value);
    void _where(ParamVector &columnToCheck, ParamVector &compareOperator, ParamVector &value, ParamString &operand);
    void _like(ParamVector &columnToCheck, ParamVector &like, ParamString &operand);
    void _glob(ParamVector &columnToCheck, ParamVector &glob, ParamString &operand);
    void _innerJoin(ParamString &table, ParamVector &columnToCheck, ParamVector &compareOperator, ParamVector &value, ParamString &operand);
    void _outerJoin(ParamString &table, ParamVector &columnToCheck, ParamVector &compareOperator, ParamVector &value, ParamString &operand);
    void _getID(ParamString &IDName, ParamString &table, ParamString &columnName, ParamString &value);
    void _updateTable(ParamString &table, ParamVector &columns, ParamVector &values, ParamString &where);
    void _deleteRowFromTable(ParamString &table, ParamString &ID, ParamString &value);
    void _clearTable(ParamString &table);
    void _disconnectFromDatabase();

protected:
    virtual void sqlite3ExceptionHandler(std::exception &e);
    virtual void sqlite3BusyExceptionHandler(std::exception &e);
    virtual void createDatabaseExceptionHandler(std::exception &e);
    virtual void createTableExceptionHandler(std::exception &e);
    virtual void createColumnExceptionHandler(std::exception &e);
    virtual void setPKExceptionHandler(std::exception &e);
    virtual void setUniqueExceptionHandler(std::exception &e);
    virtual void setDefaultValueExceptionHandler(std::exception &e);
    virtual void addColumnExceptionHandler(std::exception &e);
    virtual void setForeignKeyExceptionHandler(std::exception &e);
    virtual void addTableExceptionHandler(std::exception &e);
    virtual void insertExceptionHandler(std::exception &e);
    virtual void selectFromExceptionHandler(std::exception &e);
    virtual void updateExceptionHandler(std::exception &e);
public:
    static Sqlite_wrapper *connectToDatabase(ParamString &fileName);

    void createTable(ParamString &table);
    void createColumn(ParamString &column, ParamString &type);
    void setAsPK();
    void setAsUnique();
    void setAsNotNullable();
    void setDefaultValue(ParamString &value);
    void addColumn();
    void setForeinKey(ParamString &column, ParamString &refTable, ParamString &refColumn = "");
    void addTable();
    void dropTable(ParamString &table);//TODO
    void insertInto(ParamString &table, ParamVector &columns, ParamVector &values);
    static void printToShell(const Query &result);
    void selectFrom(ParamString table, ParamVector &columns);
    void where(ParamVector &columnToCheck, ParamVector &compareOperator, ParamVector &value, ParamString &operand = ""/*and/or*/);
    void like(ParamVector &columnToCheck, ParamVector &like, ParamString &operand = ""/*and/or*/);
    void glob(ParamVector &columnToCheck, ParamVector &glob, ParamString &operand = ""/*and/or*/);
    void groupBy(ParamVector &columns);
    void orderBy(ParamVector &orderByColumn, ParamString &order = "asc");
    void limit(int limit);
    void crossJoin(ParamString &table);
    void innerJoin(ParamString &table, ParamVector &columnToCheck, ParamVector &compareOperator, ParamVector &value, ParamString &operand = ""/*and/or*/);
    void outerJoin(ParamString &table, ParamVector &columnToCheck, ParamVector &compareOperator, ParamVector &value, ParamString &operand = ""/*and/or*/);
    std::string getID(ParamString &table, ParamString &columnName, ParamString &value, ParamString &IDName = "");
    void modifyingExec(ParamString &query);
    Query &readExec(ParamString &query);
    Query &execSelect();
    Query &getLastResult();
    //If IDName is not provided the IDName will be automatically set to table name with ID ending.
    //E.g. If table name is Test then IDName will be set to TestID
    void updateTable(ParamString &table, ParamVector &columns, ParamVector &values, ParamString &where);
    void deleteRowFromTable(ParamString &table, ParamString &ID, ParamString &value);
    void clearTable(ParamString &table);
    void disconnectFromDatabase();

    virtual ~Sqlite_wrapper();
};

#endif // SQLITE_WRAPPER_H
