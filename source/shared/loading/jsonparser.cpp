/* Copyright © 2013-2022 Graphia Technologies Ltd.
 *
 * This file is part of Graphia.
 *
 * Graphia is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Graphia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Graphia.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "jsonparser.h"

#include <QFile>
#include <QUrl>
#include <QByteArray>
#include <QDataStream>

#include <vector>

bool JsonParser::parse(const QUrl& url, IGraphModel* graphModel)
{
    QFile file(url.toLocalFile());
    QByteArray byteArray;

    if(!file.open(QIODevice::ReadOnly))
        return false;

    auto totalBytes = file.size();

    if(totalBytes == 0)
        return false;

    qint64 bytesRead = 0;
    QDataStream input(&file);

    do
    {
        const int ChunkSize = 2 << 16;
        std::vector<unsigned char> buffer(ChunkSize);

        auto numBytes = input.readRawData(reinterpret_cast<char*>(buffer.data()), ChunkSize);
        byteArray.append(reinterpret_cast<char*>(buffer.data()), numBytes);

        bytesRead += numBytes;

        setProgress(static_cast<int>((bytesRead * 100) / totalBytes));
    } while(!input.atEnd());

    auto jsonBody = parseJsonFrom(byteArray, this);

    if(jsonBody.is_null() || jsonBody.is_discarded())
        return false;

    if(cancelled())
        return false;

    return parseJson(jsonBody, graphModel);
}
