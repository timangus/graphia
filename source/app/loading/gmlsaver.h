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

#ifndef GMLEXPORTER_H
#define GMLEXPORTER_H

#include "graph/graphmodel.h"
#include "loading/saverfactory.h"

class GMLSaver : public ISaver
{
private:
    const QUrl& _url;
    IGraphModel* _graphModel;
    static QString indent(int level) { return QStringLiteral("    ").repeated(level); }

public:
    static QString name() { return QStringLiteral("GML"); }
    static QString extension() { return QStringLiteral("gml"); }
    GMLSaver(const QUrl& url, IGraphModel* graphModel) : _url(url), _graphModel(graphModel) {}
    bool save() override;
};

using GMLSaverFactory = SaverFactory<GMLSaver>;

#endif // GMLEXPORTER_H
