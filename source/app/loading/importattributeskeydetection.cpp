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

#include "importattributeskeydetection.h"

#include <QFuture>
#include <QtConcurrent/QtConcurrent>
#include <QQmlEngine>

#include "shared/utils/static_block.h"

ImportAttributesKeyDetection::ImportAttributesKeyDetection() // NOLINT modernize-use-equals-default
{
    connect(&_watcher, &QFutureWatcher<void>::started, this, &ImportAttributesKeyDetection::busyChanged);
    connect(&_watcher, &QFutureWatcher<void>::finished, this, &ImportAttributesKeyDetection::busyChanged);
}

ImportAttributesKeyDetection::~ImportAttributesKeyDetection() // NOLINT modernize-use-equals-default
{
    _watcher.waitForFinished();
}

void ImportAttributesKeyDetection::start()
{
    uncancel();

    QFuture<void> future = QtConcurrent::run([this]
    {
        QString bestAttributeName;
        size_t bestColumnIndex = 0;
        int bestPercent = 0;

        auto attributeNames = _document->availableAttributeNames(
            static_cast<int>(ElementType::NodeAndEdge), static_cast<int>(ValueType::Identifier));

        auto typeIdentities = _tabularData->typeIdentities();

        for(size_t columnIndex = 0; columnIndex < _tabularData->numColumns(); columnIndex++)
        {
            auto type = typeIdentities.at(columnIndex).type();
            if(type != TypeIdentity::Type::String && type != TypeIdentity::Type::Int)
                continue;

            for(const auto& attributeName : attributeNames)
            {
                // Don't match against any non-user defined attributes
                if(!_document->attribute(attributeName).value(QStringLiteral("userDefined")).toBool())
                    continue;

                auto values = _document->allAttributeValues(attributeName);

                if(values.empty())
                    continue;

                auto percent = _tabularData->columnMatchPercentage(columnIndex, values);

                // If we already have an equivalent match, prefer the one with the shorter attribute name
                if(percent == bestPercent && attributeName.size() > bestAttributeName.size())
                    continue;

                // If we already have an equivalent match, prefer the earlier column
                if(percent == bestPercent && columnIndex > bestColumnIndex)
                    continue;

                if(percent >= bestPercent)
                {
                    bestAttributeName = attributeName;
                    bestColumnIndex = columnIndex;
                    bestPercent = percent;
                }

                // Can't improve on 100%!
                if(bestPercent >= 100 || cancelled())
                    break;
            }

            if(bestPercent >= 100 || cancelled())
                break;
        }

        _result.clear();

        if(!cancelled())
        {
            _result.insert(QStringLiteral("attributeName"), bestAttributeName);
            _result.insert(QStringLiteral("column"), static_cast<int>(bestColumnIndex));
            _result.insert(QStringLiteral("percent"), bestPercent);
        }

        emit resultChanged();
    });

    _watcher.setFuture(future);
}

void ImportAttributesKeyDetection::reset()
{
    _attributeValues = {};
    _result = {};
    emit resultChanged();
}

static_block
{
    qmlRegisterType<ImportAttributesKeyDetection>(APP_URI, APP_MAJOR_VERSION,
        APP_MINOR_VERSION, "ImportAttributesKeyDetection");
}
