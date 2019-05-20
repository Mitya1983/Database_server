#include "result.h"

Result::Result()
{

}

Result::Result(Result &other)
{
    _result = std::move(other._result);
}

void Result::resize(const int numberOfColumns)
{
    _result.resize(numberOfColumns);
}

void Result::resize(const int numberOfColumns, const int numberOfRows)
{
    _result.resize(numberOfColumns);
    for(auto &i : _result)
    {
        i.values.resize(numberOfRows);
    }
}

int Result::getIndexOf(const std::string &columnName) const
{
    int index = -1;
    unsigned int size = _result.size();
    if (size == 0)
        return index;
    for (unsigned int i = 0; i < size; i++)
    {
        if (_result[i].name == columnName)
        {
            index = i;
            break;
        }
    }

    return index;
}

void Result::addColumn(const std::string &name, int index)
{
    _result[index].name = name;
    if (_result[index].values.capacity() < 50)
        _result[index].values.reserve(50);
}

void Result::addValue(const std::string &value, int columnIndex)
{
    _result[columnIndex].values.push_back(value);
}

void Result::addValue(const std::string &value, const std::string &columnName)
{
    _result[getIndexOf(columnName)].values.push_back(value);
}

void Result::clear()
{
    _result.clear();
}

unsigned int Result::size() const
{
    return _result.size();
}

const std::vector<Result::Column> &Result::result() const
{
    return _result;
}

std::string Result::columns(const std::string delimiter)
{
    std::string _columns;
    for (unsigned int i = 0, n = _result.size(); i < n; i++)
    {
        _columns += _result[i].name;
        if (i < n - 1)
            _columns += delimiter;
    }
    return _columns;
}

const std::vector<std::string> &Result::rowsAt(const std::string &columnName) const
{
    return _result[getIndexOf(columnName)].values;
}

std::string Result::valueAt(const std::string columnName, int row) const
{
    return _result[getIndexOf(columnName)].values[row];
}

std::string Result::valueAt(int column, int row) const
{
    return _result[column].values[row];
}

std::string Result::resultToString() const
{
    std::string _resultToString;
    unsigned int numberOfColumns = _result.size();
    unsigned int numberOfRows = _result[0].values.size();
    _resultToString += "Columns:" + std::to_string(numberOfColumns) + '\n';
    _resultToString += "Rows:" + std::to_string(numberOfRows) + '\n';
    for (unsigned int i = 0; i < numberOfColumns; i++)
    {
        _resultToString += "Column:" + _result[i].name + '\n';
        for (unsigned int j = 0; j < numberOfRows; j++)
        {
            _resultToString += _result[i].values[j];
            if (j < numberOfRows - 1)
                _resultToString += '\n';
        }
        if (i < numberOfColumns - 1)
            _resultToString += '\n';
        else
            _resultToString += EOF;
    }
    return _resultToString;
}

void Result::resultFromString(const std::string &result)
{
    auto start = result.find("Columns:") + 8;
    auto end = result.find_first_of('\n');
    unsigned int numberOfColumns = std::atoi(result.substr(start, end - start).c_str());
    start = result.find("Rows:", end) + 5;
    end = result.find_first_of('\n', start);
    unsigned int numberOfRows = std::atoi(result.substr(start, end - start).c_str());
    resize(numberOfColumns, numberOfRows);
    for (unsigned int i = 0; i < numberOfColumns; i++)
    {
        start = result.find_first_of("Column:", end) + 7;
        end = result.find_first_of('\n', start);
        _result[i].name = result.substr(start, end - start);
        for (unsigned int j = 0; j < numberOfRows; j++)
        {
            start = end + 1;
            if (j == numberOfRows - 1 && i == numberOfColumns - 1)
                end = result.find_last_of(EOF);
            else
                end = result.find_first_of('\n', start);
            _result[i].values[j] = result.substr(start, end - start);
        }
    }

}
