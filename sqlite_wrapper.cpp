#include "sqlite_wrapper.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <mutex>
Sqlite3Exception::Sqlite3Exception(const std::string &databaseName, const std::string &details, const std::string &msg)
{
    _msg = "Error from SQL on database ";
    _msg += std::move(databaseName);
    _msg += ": On ";
    _msg += std::move(details);
    _msg += "- ";
    _msg += std::move(msg);
}

const char *Sqlite3Exception::what() const noexcept
{
    return _msg.c_str();
}

CreateDatabaseException::CreateDatabaseException(const std::string &msg)
{
    _msg = std::move(msg);
}

const char *CreateDatabaseException::what() const noexcept
{
    return _msg.c_str();
}

TableException::TableException(const std::string &databaseName, const std::string &tableName, const std::string &msg)
{
    _msg = "Error on table ";
    _msg += std::move(tableName);
    _msg += " of database ";
    _msg += std::move(databaseName);
    _msg += ": ";
    _msg += std::move(msg);
}

const char *TableException::what() const noexcept
{
    return _msg.c_str();
}

ColumnException::ColumnException(const std::string &databaseName, const std::string &tableName,
                                 const std::string &columnName, const std::string &msg)
{
    _msg = "Error on column ";
    _msg += std::move(columnName);
    _msg += " of table ";
    _msg += std::move(tableName);
    _msg += " of databse ";
    _msg += std::move(databaseName);
    _msg += ": ";
    _msg += std::move(msg);
}

const char *ColumnException::what() const noexcept
{
    return _msg.c_str();
}
Result Sqlite_wrapper::_result;
bool Sqlite_wrapper::firstQuery = true;
int Sqlite_wrapper::callback(void *, int argc, char **argv, char **azColName)
{
    for (int i = 0; i < argc; i++)
    {
        if (firstQuery)
        {
            _result.resize(argc);
            _result.addColumn(azColName[i], i);
        }
        _result.addValue(std::string(argv[i] ? argv[i] : ""), i);
    }
    if (firstQuery)
        firstQuery = false;
    return 0;
}

Sqlite_wrapper::Sqlite_wrapper()
{
    currentTable = false;
    currentColumn = false;
    sqlite3Errmsg = nullptr;
}

void Sqlite_wrapper::_modifyingExec(ParamString &query)
{
    int status;
    status = sqlite3_exec(db, query.c_str(), nullptr, nullptr, &sqlite3Errmsg);
    while (status == SQLITE_BUSY)
    {
        sqlite3_free(sqlite3Errmsg);
        std::this_thread::sleep_for(std::chrono::seconds(5));
        status = sqlite3_exec(db, query.c_str(), nullptr, nullptr, &sqlite3Errmsg);
    }
    if (status != SQLITE_OK)
    {
        std::string msg = std::move(sqlite3Errmsg);
        sqlite3_free(sqlite3Errmsg);
        throw Sqlite3Exception(curTable.databaseName, query, msg);
    }
}

void Sqlite_wrapper::_readExec(ParamString &query)
{
    _result.clear();
    result.clear();
    firstQuery = true;
    int status;
    auto _callback = Sqlite_wrapper::callback;
    if ((status = sqlite3_exec(db, query.c_str(), _callback, nullptr, &sqlite3Errmsg)) != SQLITE_OK)
    {
        std::string msg = std::move(sqlite3Errmsg);
        sqlite3_free(sqlite3Errmsg);
        throw Sqlite3Exception(curTable.databaseName, query, msg);
    }
}

void Sqlite_wrapper::_createDatabase(ParamString &fileName)
{
    if (fileName == "")
        throw CreateDatabaseException("Filename wasn't provided");
    std::string path = std::move(fileName);
    curTable.databaseName = path.substr(path.find_last_of('/') + 1, path.length());
    if (fileName.substr(fileName.length()-3, fileName.length()) != ".db")
    {
        path += ".db";
    }
    int status;
    if ((status = sqlite3_open(path.c_str(), &db)))
        throw Sqlite3Exception(curTable.databaseName, path, sqlite3_errmsg(db));
}

void Sqlite_wrapper::_createTable(ParamString &table)
{
    if (currentTable)
        throw TableException(curTable.databaseName, table, "Work with previous table wasn't finished.\nPlease make sure that each createTable query has corresponding addTable query.");
    currentTable = true;
    curTable.name = std::move(table);
    curTable.isForeignKey = false;
    curTable.noRowID = false;
    curTable.pKisSet = false;
}

void Sqlite_wrapper::_createColumn(ParamString &column, ParamString &type)
{
    if (currentColumn)
        throw ColumnException(curTable.databaseName, curTable.name, column, "Work with previous column wasn't finished.\nPlease make sure that each createColumn query has corresponding addColumn query.");
    currentColumn = true;
    curColumn.name = std::move(column);
    curColumn.type = std::move(type);
    curColumn.isPK = false;
    curColumn.isUnique = false;
    curColumn.isNullable = true;
    curColumn.isDefaultValue = false;
}

void Sqlite_wrapper::_setAsPK()
{
    if (curColumn.isDefaultValue)
        throw ColumnException(curTable.databaseName, curTable.name, curColumn.name, "The default value was previosly set for this column.\nSetting it as Primary Key may provoke issues.");
    curColumn.isPK = true;
    curColumn.isUnique = true;
    curColumn.isNullable = false;
    curTable.pKisSet = true;
}

void Sqlite_wrapper::_setAsUnique()
{
    if (curColumn.isDefaultValue)
        throw ColumnException(curTable.databaseName, curTable.name, curColumn.name, "The default value was previosly set for this column.\nSetting it as Unique may provoke issues.");
    curColumn.isUnique = true;
    curColumn.isNullable = false;
}

void Sqlite_wrapper::_setAsNotNullable()
{
    curColumn.isNullable = false;
}

void Sqlite_wrapper::_setDefaultValue(ParamString &value)
{
    if (curColumn.isPK || curColumn.isUnique)
        throw ColumnException(curTable.databaseName, curTable.name, curColumn.name, "The column was previously set as Primery Key or Unique value.\n Setting a Default value may provoke issues.");
    curColumn.isDefaultValue = true;
    curColumn.defaultValue = std::move(value);
}

void Sqlite_wrapper::_setNoRowID()
{
    curTable.noRowID = true;
}

void Sqlite_wrapper::_addColumn()
{
    if (!currentColumn)
        throw ColumnException(curTable.databaseName, curTable.name, "undefined", "Work with column wasn't started.\ncreateColumn stetemnt should be used first");
    curTable.columns.push(std::move(curColumn));
    currentColumn = false;
    curColumn.clear();
}

void Sqlite_wrapper::_setForeinKey(ParamString &column, ParamString &refTable, ParamString &refColumn)
{
    if (!curTable.isForeignKey)
        curTable.isForeignKey = true;
    ForeignKey key;
    key.column = std::move(column);
    key.refTable = std::move(refTable);
    if (refColumn == "")
        key.refColumn = column;
    else
        key.refColumn = std::move(refColumn);
    curTable.foreignKeys.push(std::move(key));
}

void Sqlite_wrapper::_addTable()
{
    if (!currentTable)
        throw TableException(curTable.databaseName, "undefined", "Work with table wasn't started.\ncreateTable stetemnt should be used first");
    _modifyingExec(curTable.getQuery());
    curTable.clear();
    currentTable = false;
}

void Sqlite_wrapper::_getID(ParamString &IDName, ParamString &table, ParamString &columnName, ParamString &value)
{
    std::string query = "select ";
    query += std::move(IDName);
    query += " from ";
    query += std::move(table);
    query += " where ";
    query += std::move(columnName);
    query += " = ";
    query += '\'' + std::move(value) + '\'';

    _readExec(query);
}

void Sqlite_wrapper::_disconnectFromDatabase()
{
    int status;
    if ((status = sqlite3_close(db)) == SQLITE_BUSY)
        throw Sqlite3Exception(curTable.databaseName, "", sqlite3_errmsg(db));
}

void Sqlite_wrapper::sqlite3ExceptionHandler(std::exception &e)
{
    std::cerr << e.what() << std::endl;
}

void Sqlite_wrapper::sqlite3BusyExceptionHandler(std::exception &e)
{
    int count = 1;
    std::cerr << e.what() << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(5));
    try {
        count++;
        std::cerr << "Attempt to close connection to database #" << count << std::endl;
        _disconnectFromDatabase();
    } catch (std::exception &e) {
        sqlite3BusyExceptionHandler(e);
    }
}

void Sqlite_wrapper::createDatabaseExceptionHandler(std::exception &e)
{
    std::cerr << "createDatabase(): " << e.what() << std::endl;
}

void Sqlite_wrapper::createTableExceptionHandler(std::exception &e)
{
    std::cerr << "createTable(): " << e.what() << std::endl;
}

void Sqlite_wrapper::createColumnExceptionHandler(std::exception &e)
{
    std::cerr << "createColumn(): " << e.what() << std::endl;
}

void Sqlite_wrapper::setPKExceptionHandler(std::exception &e)
{
    std::cerr << "setAsPK(): " << e.what() << std::endl;
}

void Sqlite_wrapper::setUniqueExceptionHandler(std::exception &e)
{
    std::cerr << "setUnique(): " << e.what() << std::endl;
}

void Sqlite_wrapper::setDefaultValueExceptionHandler(std::exception &e)
{
    std::cerr << "setDefaultValue(): " << e.what() << std::endl;
}

void Sqlite_wrapper::addColumnExceptionHandler(std::exception &e)
{
    std::cerr << "addcolumn(): " << e.what() << std::endl;
}

void Sqlite_wrapper::setForeignKeyExceptionHandler(std::exception &e)
{
    std::cerr << "setForeignKey(): " << e.what() << std::endl;
}

void Sqlite_wrapper::addTableExceptionHandler(std::exception &e)
{
     std::cerr << "addTable(): " << e.what() << std::endl;
     curTable.clear();
     currentTable = false;
}

void Sqlite_wrapper::insertExceptionHandler(std::exception &e)
{
    std::cerr << "insertInto(): " << e.what() << std::endl;
}

void Sqlite_wrapper::selectFromExceptionHandler(std::exception &e)
{
    std::cerr << "selectFrom(): " << e.what() << std::endl;
}

void Sqlite_wrapper::updateExceptionHandler(std::exception &e)
{
    std::cerr << "updateTable(): " << e.what() << std::endl;
}

Sqlite_wrapper *Sqlite_wrapper::connectToDatabase(ParamString &fileName)
{
    Sqlite_wrapper *temp = new Sqlite_wrapper();
    try {
        temp->_createDatabase(fileName);
    } catch (std::exception &e) {
        temp->createDatabaseExceptionHandler(e);
        delete temp;
        return nullptr;
    }
    return temp;
}

void Sqlite_wrapper::createTable(ParamString &table)
{
    try {
        _createTable(table);
    } catch (TableException &e) {
        createTableExceptionHandler(e);
    } catch (Sqlite3Exception &e) {
        sqlite3ExceptionHandler(e);
    }
}

void Sqlite_wrapper::createColumn(ParamString &column, ParamString &type)
{
    try {
        _createColumn(column, type);
    } catch (ColumnException &e) {
        createColumnExceptionHandler(e);
    }
}

void Sqlite_wrapper::setAsPK()
{
    try {
        _setAsPK();
    } catch (ColumnException &e) {
        setPKExceptionHandler(e);
    }
}

void Sqlite_wrapper::setAsUnique()
{
    try {
        _setAsUnique();
    } catch (ColumnException &e) {
        setUniqueExceptionHandler(e);
    }
}

void Sqlite_wrapper::setAsNotNullable()
{
    _setAsNotNullable();
}

void Sqlite_wrapper::setDefaultValue(ParamString &value)
{
    try {
        _setDefaultValue(value);
    } catch (ColumnException &e) {
        setDefaultValueExceptionHandler(e);
    }
}

void Sqlite_wrapper::addColumn()
{
    try {
        _addColumn();
    } catch (ColumnException &e) {
        addColumnExceptionHandler(e);
    }
}

void Sqlite_wrapper::setForeinKey(ParamString &column, ParamString &refTable, ParamString &refColumn)
{
    try {
        _setForeinKey(column, refTable, refColumn);
    } catch (std::exception &e) {
        setForeignKeyExceptionHandler(e);
    }
}

void Sqlite_wrapper::addTable()
{
    try {
        _addTable();
    } catch (std::exception &e) {
        addTableExceptionHandler(e);
    }
}

void Sqlite_wrapper::printToShell(const Result &result)
{
    if (result.size() == 0)
        std::cout << "No rows were selected" << std::endl;
    else
        std::cout << result.resultToString() << std::endl;
}

std::string Sqlite_wrapper::getID(ParamString &table, ParamString &columnName, ParamString &value, ParamString &IDName)
{
    std::string _IDName, ID;
    if (IDName == "")
        _IDName = std::move(table) + "ID";
    else
        _IDName = std::move(IDName);
    try {
        _getID(_IDName, table, columnName, value);
        if (_result.size() == 0)
            ID = "";
        else
            ID = _result.valueAt(0, 0);
    } catch (std::exception &e) {
        sqlite3ExceptionHandler(e);
    }

    return ID;
}

void Sqlite_wrapper::modifyingExec(ParamString &query)
{
    try {
        _modifyingExec(query);
    } catch (std::exception &e) {
        sqlite3ExceptionHandler(e);
    }
}

Result &Sqlite_wrapper::readExec(ParamString &query)
{
    std::mutex lock;
    try {
        lock.lock();
        curTable.name.clear();
        _readExec(query);
        firstQuery = true;
        if (_result.size() != 0)
            result = std::move(_result);
        lock.unlock();
    } catch (std::exception &e) {
        lock.unlock();
        sqlite3ExceptionHandler(e);
    }
    return result;
}

Result &Sqlite_wrapper::getLastResult()
{
    return result;
}

void Sqlite_wrapper::disconnectFromDatabase()
{
    try {
        _disconnectFromDatabase();
    } catch (std::exception &e) {
        sqlite3BusyExceptionHandler(e);
    }
}

Sqlite_wrapper::~Sqlite_wrapper()
{
    disconnectFromDatabase();
}

std::string Sqlite_wrapper::Column::getQuery()
{
    std::string query = name;

    query += " ";
    query += type;
    if (isPK)
        query += " primary key";
    if (isUnique)
        query += " unique";
    if (isNullable)
        query += " null";
    else
        query += " not null";
    if (isDefaultValue)
    {
        query += " default '";
        query += defaultValue;
        query += "'";
    }

    return query;
}

void Sqlite_wrapper::Column::clear()
{
    name.clear();
    type.clear();
    defaultValue.clear();
    isPK = false;
    isUnique = false;
    isNullable = true;
    isDefaultValue = false;
}

std::string Sqlite_wrapper::ForeignKey::getQuery()
{
    std::string query = "foreign key (";

    query += column;
    query += " ) references ";
    query += refTable;
    query += " (";
    query += refColumn;
    query += ")";

    return query;
}

void Sqlite_wrapper::ForeignKey::clear()
{
    column.clear();
    refTable.clear();
    refColumn.clear();
}

std::string Sqlite_wrapper::Table::getQuery()
{
    if (columns.size() == 0)
        throw TableException(databaseName, name, "No columns were provided");

    std::string query = "create table ";

    query += name;
    query += " (";
    if (!pKisSet && noRowID)
        query += name + "ID integer not null unique, ";
    while (columns.size() > 1)
    {
        query += columns.front().getQuery();
        query += ", ";
        columns.pop();
    }
    query += columns.front().getQuery();
    columns.pop();
    if (isForeignKey)
    {
        query += ", ";
        while (foreignKeys.size() > 1)
        {
            query += foreignKeys.front().getQuery();
            query += ", ";
            foreignKeys.pop();
        }
        query += foreignKeys.front().getQuery();
        foreignKeys.pop();
    }
    if (noRowID)
        query += ") without rowid;";
    else
        query += ");";

    return query;
}

void Sqlite_wrapper::Table::clear()
{
    name.clear();
    while (columns.size() > 0)
        columns.pop();
    isForeignKey = false;
    while (foreignKeys.size() > 0)
        foreignKeys.pop();
    noRowID = false;
    pKisSet = false;
}
