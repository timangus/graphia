#ifndef ENRICHMENTHEATMAPITEM_H
#define ENRICHMENTHEATMAPITEM_H

#include <QQuickPaintedItem>
#include "thirdparty/qcustomplot/qcustomplot.h"
#include "attributes/enrichmentcalculator.h"

class EnrichmentHeatmapItem : public QQuickPaintedItem
{
    Q_OBJECT

    Q_PROPERTY(EnrichmentTableModel* model MEMBER _tableModel NOTIFY tableModelChanged)
    Q_PROPERTY(double scrollXAmount MEMBER _scrollXAmount WRITE setScrollXAmount NOTIFY scrollAmountChanged)
    Q_PROPERTY(double scrollYAmount MEMBER _scrollYAmount WRITE setScrollYAmount NOTIFY scrollAmountChanged)
    Q_PROPERTY(double horizontalRangeSize READ horizontalRangeSize NOTIFY horizontalRangeSizeChanged)
    Q_PROPERTY(double verticalRangeSize READ verticalRangeSize NOTIFY verticalRangeSizeChanged)
    Q_PROPERTY(double scrollYAmount MEMBER _scrollYAmount WRITE setScrollYAmount NOTIFY scrollAmountChanged)
    Q_PROPERTY(int elideLabelWidth MEMBER _elideLabelWidth WRITE setElideLabelWidth)

private:
    QCPLayer* _textLayer = nullptr;
    QCPColorMap* _colorMap = nullptr;
    QCPColorScale* _colorScale = nullptr;
    QCPItemText* _hoverLabel = nullptr;
    QCPAbstractPlottable* _hoverPlottable = nullptr;
    QPointF _hoverPoint;
    QCustomPlot _customPlot;
    QStringList _xAttributeValues;
    QStringList _yAttributeValues;
    std::map<int, QString> _xAxisToFullLabel;
    std::map<int, QString> _yAxisToFullLabel;


    EnrichmentTableModel* _tableModel = nullptr;
    QFont _defaultFont9Pt;
    int _attributeACount = 0;
    int _attributeBCount = 0;
    double _scrollXAmount = 0.0;
    double _scrollYAmount = 0.0;
    int _elideLabelWidth = 120;
    const double _HEATMAP_OFFSET = 0.5;

public:
    explicit EnrichmentHeatmapItem(QQuickItem* parent = nullptr);
    void setData(EnrichmentCalculator::Table table);

public:
    void paint(QPainter *painter);
    void updatePlotSize();
    double horizontalRangeSize();
    double verticalRangeSize();
    double columnAxisWidth();
    double columnLabelSize();
    void scaleXAxis();
    void scaleYAxis();
    double columnAxisHeight();
    void setScrollXAmount(double scrollAmount);
    void setScrollYAmount(double scrollAmount);
    void setElideLabelWidth(int elideLabelWidth);

    Q_INVOKABLE void buildPlot();
protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void hoverMoveEvent(QHoverEvent* event) override;
    void hoverLeaveEvent(QHoverEvent* event) override;
    void routeMouseEvent(QMouseEvent *event);

signals:
    void tableModelChanged();
    void scrollAmountChanged();
    void horizontalRangeSizeChanged();
    void verticalRangeSizeChanged();
    void plotValueClicked(int row);

public slots:
    void onCustomReplot();
    void showTooltip();
    void hideTooltip();
};

#endif // ENRICHMENTHEATMAPITEM_H
