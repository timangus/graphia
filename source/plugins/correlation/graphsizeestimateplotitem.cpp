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

#include "graphsizeestimateplotitem.h"

#include "shared/utils/utils.h"
#include "shared/utils/string.h"
#include "shared/rendering/multisamples.h"

#include <QVariantMap>
#include <QVector>

#include <algorithm>

GraphSizeEstimatePlotItem::GraphSizeEstimatePlotItem(QQuickItem* parent) :
    QCustomPlotQuickItem(multisamples(), parent)
{
    connect(this, &GraphSizeEstimatePlotItem::uniqueEdgesOnlyChanged, [this] { buildPlot(); });
}

double GraphSizeEstimatePlotItem::threshold() const
{
    return _threshold;
}

void GraphSizeEstimatePlotItem::setThreshold(double threshold)
{
    if(threshold != _threshold)
    {
        if(!_keys.isEmpty())
        {
            threshold = std::clamp(threshold,
                std::as_const(_keys).first(),
                std::as_const(_keys).last());
        }

        _threshold = threshold;
        emit thresholdChanged();

        updateThresholdIndicator();
        customPlot().replot(QCustomPlot::rpQueuedReplot);
    }
}

void GraphSizeEstimatePlotItem::setGraphSizeEstimate(const QVariantMap& graphSizeEstimate)
{
    if(!graphSizeEstimate.contains(QStringLiteral("keys")))
        return;

    if(!graphSizeEstimate.contains(QStringLiteral("numNodes")))
        return;

    if(!graphSizeEstimate.contains(QStringLiteral("numEdges")))
        return;

    if(!graphSizeEstimate.contains(QStringLiteral("numUniqueEdges")))
        return;

    _keys = graphSizeEstimate.value(QStringLiteral("keys")).value<QVector<double>>();
    _numNodes = graphSizeEstimate.value(QStringLiteral("numNodes")).value<QVector<double>>();
    _numEdges = graphSizeEstimate.value(QStringLiteral("numEdges")).value<QVector<double>>();
    _numUniqueEdges = graphSizeEstimate.value(QStringLiteral("numUniqueEdges")).value<QVector<double>>();

    buildPlot();
}

void GraphSizeEstimatePlotItem::updateThresholdIndicator()
{
    if(_thresholdIndicator == nullptr || _keys.isEmpty())
        return;

    // Clamp the indicator x coordinate to within the bounds of the
    // plot by a small margin, so that it's always visible
    auto smallestWeight = std::as_const(_keys).first();
    auto largestWeight = std::as_const(_keys).last();
    const double keepVisibleOffset = (largestWeight - smallestWeight) * 0.002;

    auto x = std::clamp(_threshold,
        smallestWeight + keepVisibleOffset,
        largestWeight - keepVisibleOffset);

    _thresholdIndicator->point1->setCoords(x, 0.0);
    _thresholdIndicator->point2->setCoords(x, 1.0);

    int index = 0;
    while(_keys.at(index) < _threshold && index < _keys.size() - 1)
        index++;

    size_t numNodes = 0;
    size_t numEdges = 0;

    if(index < _keys.size())
    {
        numNodes = static_cast<size_t>(_numNodes.at(index));
        numEdges = _uniqueEdgesOnly ?
            static_cast<size_t>(_numUniqueEdges.at(index)) :
            static_cast<size_t>(_numEdges.at(index));
    }

    customPlot().xAxis->setLabel(tr("Estimated Graph Size: %1 Nodes, %2 Edges")
        .arg(u::formatNumberSIPostfix(static_cast<double>(numNodes)),
        u::formatNumberSIPostfix(static_cast<double>(numEdges))));
}

void GraphSizeEstimatePlotItem::buildPlot()
{
    if(_keys.isEmpty())
        return;

    customPlot().clearItems();
    customPlot().clearPlottables();

    auto* nodesGraph = customPlot().addGraph();
    auto* edgesGraph = customPlot().addGraph();

    nodesGraph->setData(_keys, _numNodes, true);
    nodesGraph->setPen(QPen(Qt::red));
    nodesGraph->setName(tr("Nodes"));
    edgesGraph->setData(_keys, _uniqueEdgesOnly ? _numUniqueEdges : _numEdges, true);
    edgesGraph->setPen(QPen(Qt::blue));
    edgesGraph->setName(tr("Edges"));

    _thresholdIndicator = new QCPItemStraightLine(&customPlot());

    QPen indicatorPen;
    indicatorPen.setStyle(Qt::DashLine);
    _thresholdIndicator->setPen(indicatorPen);

    updateThresholdIndicator();

    customPlot().yAxis->setScaleType(QCPAxis::stLogarithmic);
    QSharedPointer<QCPAxisTickerLog> logTicker(new QCPAxisTickerLog);
    logTicker->setLogBase(10);
    logTicker->setSubTickCount(3);
    customPlot().yAxis->setTicker(logTicker);
    customPlot().yAxis->setNumberFormat(QStringLiteral("eb"));
    customPlot().yAxis->setNumberPrecision(0);
    customPlot().yAxis->grid()->setSubGridVisible(true);

    auto smallestWeight = std::as_const(_keys).first();
    auto largestWeight = std::as_const(_keys).last();

    customPlot().xAxis->setRange(smallestWeight, largestWeight);

    customPlot().yAxis->rescale();

    customPlot().legend->setVisible(true);
    customPlot().legend->setBrush(QBrush(QColor(255, 255, 255, 127)));
    customPlot().axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignLeft|Qt::AlignBottom);

    customPlot().replot(QCustomPlot::rpQueuedReplot);
}

void GraphSizeEstimatePlotItem::mousePressEvent(QMouseEvent* event)
{
    routeMouseEvent(event);
    if(event->button() == Qt::MouseButton::LeftButton)
    {
        _dragging = true;
        auto xValue = customPlot().xAxis->pixelToCoord(event->pos().x());
        setThreshold(xValue);
    }
}

void GraphSizeEstimatePlotItem::mouseReleaseEvent(QMouseEvent* event)
{
    routeMouseEvent(event);
    if(event->button() == Qt::MouseButton::LeftButton)
        _dragging = false;
}

// NOLINTNEXTLINE readability-make-member-function-const
void GraphSizeEstimatePlotItem::mouseMoveEvent(QMouseEvent* event)
{
    routeMouseEvent(event);
    if(_dragging)
    {
        auto xValue = customPlot().xAxis->pixelToCoord(event->pos().x());
        setThreshold(xValue);
    }
}
