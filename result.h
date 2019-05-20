#ifndef RESULT_H
#define RESULT_H
#include <string>
#include <vector>

class Result
{
private:
    struct Column
    {
        std::string name;
        std::vector<std::string> values;
    };
    std::vector<Column> _result;
public:
    Result();
    Result(Result &other);
    Result(const Result &other) = delete;
    Result(Result &&other) = default;
    Result &operator = (const Result &other) = delete;
    Result &operator = (Result &&other) = default;
    void resize (const int numberOfColumns);
    void resize (const int numberOfColumns, const int numberOfRows);
    int getIndexOf(const std::string &columnName) const;
    void addColumn(const std::string &name, int index);
    void addValue(const std::string &value, int columnIndex);
    void addValue(const std::string &value, const std::string &columnName);
    const std::vector<Column> &result() const;
    std::string columns(const std::string delimiter);
    const std::vector<std::string> &rowsAt(const std::string &columnName) const;
    std::string valueAt(const std::string columnName, int row) const;
    std::string valueAt(int column, int row) const;
    void clear();
    unsigned int size() const;
    virtual std::string resultToString() const;
    virtual void resultFromString(const std::string &result);

    virtual ~Result() = default;
};

#endif // RESULT_H
