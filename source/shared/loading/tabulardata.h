#ifndef TABULARDATA_H
#define TABULARDATA_H

#include "shared/graph/igraphmodel.h"
#include "shared/graph/imutablegraph.h"
#include "shared/loading/iparser.h"
#include "shared/utils/string.h"

#include "thirdparty/utfcpp/utf8.h"

#include <QObject>
#include <QUrl>

#include <string>
#include <vector>
#include <iostream>
#include <fstream>

class TabularData
{
private:
    std::vector<std::string> _data;
    size_t _columns = 0;
    size_t _rows = 0;
    bool _transposed = false;

    size_t index(size_t column, size_t row) const;

public:
    void reserve(size_t columns, size_t rows);

    size_t numColumns() const;
    size_t numRows() const;
    bool transposed() const { return _transposed; }
    const std::string& valueAt(size_t column, size_t row) const;
    QString valueAsQString(size_t column, size_t row) const;

    void setTransposed(bool transposed) { _transposed = transposed; }
    void setValueAt(size_t column, size_t row, std::string&& value, int progressHint = -1);

    void shrinkToFit();

    void reset();
};

template<const char Delimiter> class TextDelimitedTabularDataParser : public IParser
{
    static_assert(Delimiter != '\"', "Delimiter cannot be a quotemark");

private:
    TabularData _tabularData;
    const IParser* _parent = nullptr;

    template<typename TokenFn>
    bool tokenise(const QUrl& url, TokenFn tokenFn)
    {
        std::ifstream file(url.toLocalFile().toStdString());
        if(!file)
            return false;

        auto fileSize = file.tellg();
        file.seekg(0, std::ios::end);
        fileSize = file.tellg() - fileSize;

        std::string line;
        std::string currentToken;
        size_t currentColumn = 0;
        size_t currentRow = 0;

        setProgress(-1);

        file.seekg(0, std::ios::beg);
        while(u::getline(file, line))
        {
            auto progress = static_cast<int>(file.tellg() * 100 / fileSize);

            if(_parent != nullptr && _parent->cancelled())
                return false;

            bool inQuotes = false;

            std::string validatedLine;
            utf8::replace_invalid(line.begin(), line.end(), std::back_inserter(validatedLine));
            auto it = validatedLine.begin();
            auto end = validatedLine.end();
            while(it < end)
            {
                uint32_t codePoint = utf8::next(it, end);

                if(codePoint == '\"')
                {
                    if(inQuotes)
                    {
                        tokenFn(currentColumn++, currentRow, std::move(currentToken), progress);
                        currentToken.clear();

                        // Quote closed, but there is text before the delimiter
                        while(it < end && codePoint != Delimiter)
                            codePoint = utf8::next(it, end);
                    }

                    inQuotes = !inQuotes;
                }
                else
                {
                    bool delimiter = (codePoint == Delimiter);

                    if(delimiter && !inQuotes)
                    {
                        tokenFn(currentColumn++, currentRow, std::move(currentToken), progress);
                        currentToken.clear();
                    }
                    else
                        utf8::unchecked::append(codePoint, std::back_inserter(currentToken));
                }
            }

            if(!currentToken.empty())
            {
                tokenFn(currentColumn++, currentRow, std::move(currentToken), progress);
                currentToken.clear();
            }

            currentRow++;
            currentColumn = 0;

            setProgress(progress);
        }

        return true;
    }

public:
    explicit TextDelimitedTabularDataParser(IParser* parent = nullptr) :
        _parent(parent)
    {
        if(parent != nullptr)
        {
            setProgressFn(
            [parent](int percent)
            {
                parent->setProgress(percent);
            });
        }
    }

    bool parse(const QUrl& url, IGraphModel* graphModel = nullptr) override
    {
        if(graphModel != nullptr)
            graphModel->mutableGraph().setPhase(QObject::tr("Parsing"));

        auto result = tokenise(url,
        [this](size_t column, size_t row, auto&& token, int progress)
        {
            _tabularData.setValueAt(column, row,
                std::forward<decltype(token)>(token), progress);
        });

        // Free up any over-allocation
        _tabularData.shrinkToFit();

        return result;
    }

    TabularData& tabularData() { return _tabularData; }
};

using CsvFileParser = TextDelimitedTabularDataParser<','>;
using TsvFileParser = TextDelimitedTabularDataParser<'\t'>;

#endif // TABULARDATA_H
