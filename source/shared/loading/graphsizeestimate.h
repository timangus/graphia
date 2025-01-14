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

#ifndef GRAPHSIZEESTIMATE_H
#define GRAPHSIZEESTIMATE_H

#include "shared/graph/edgelist.h"

#include <QVariantMap>

#include <limits>

QVariantMap graphSizeEstimate(EdgeList edgeList,
    double nodesScale = 1.0, double edgesScale = 1.0,
    double nodesMax = std::numeric_limits<double>::max(),
    double edgesMax = std::numeric_limits<double>::max());

#endif // GRAPHSIZEESTIMATE_H
