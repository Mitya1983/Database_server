#ifndef SQLITE_WRAPPER_H
#define SQLITE_WRAPPER_H
#include <sqlite3.h>
#include <exception>
#include <string>
#include <queue>
#include <vector>
#include <memory>

#include "result.h"

using ParamVector = const std::vector<std::string>;
using ParamString = const std::string;

//Exceptions
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
//END_Exceptions


//struct resultColumn
//{
//    std::string name;
//    std::vector<std::string> values;
//};
//using _Result = std::vector<resultColumn>;


class Sqlite_wrapper
{
    Sqlite_wrapper();
    Sqlite_wrapper(const Sqlite_wrapper &other) = delete;
    Sqlite_wrapper(const Sqlite_wrapper &&other) = delete;
    Sqlite_wrapper& operator=(const Sqlite_wrapper &other) = delete;
    Sqlite_wrapper& operator=(const Sqlite_wrapper &&other) = delete;
    sqlite3 *db;
    char *sqlite3Errmsg;
    static bool firstQuery;
    static int callback(void*, int argc, char **argv, char **azColName);
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
        std::string databaseName;
        std::string name;
        std::queue<Column> columns;
        bool pKisSet;
        bool isForeignKey;
        std::queue<ForeignKey> foreignKeys;
        bool noRowID;
        std::string getQuery();
        void clear();
    };
    Column curColumn;
    bool currentColumn;
    Table curTable;
    bool currentTable;

    static Result _result;
    Result result;


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
    void _getID(ParamString &IDName, ParamString &table, ParamString &columnName, ParamString &value);
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
    static void printToShell(const Result &result);

    std::string getID(ParamString &table, ParamString &columnName, ParamString &value, ParamString &IDName = "");
    void modifyingExec(ParamString &query);
    Result &readExec(ParamString &query);
    Result &getLastResult();
    //If IDName is not provided the IDName will be automatically set to table name with ID ending.
    //E.g. If table name is Test then IDName will be set to TestID
    void disconnectFromDatabase();
    virtual ~Sqlite_wrapper();
};

#endif // SQLITE_WRAPPER_H
