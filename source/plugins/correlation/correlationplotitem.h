#ifndef CORRELATIONPLOTITEM_H
#define CORRELATIONPLOTITEM_H

#include "shared/utils/qmlenum.h"

#include "thirdparty/qcustomplot/qcustomplot_disable_warnings.h"
#include "thirdparty/qcustomplot/qcustomplot.h"
#include "thirdparty/qcustomplot/qcustomplot_enable_warnings.h"

#include <QQuickPaintedItem>
#include <QVector>
#include <QStringList>
#include <QVector>

DEFINE_QML_ENUM(Q_GADGET, PlotScaleType,
                Raw,
                Log,
                MeanCentre,
                UnitVariance,
                Pareto);

DEFINE_QML_ENUM(Q_GADGET, PlotAveragingType,
                Individual,
                MeanLine,
                MedianLine,
                MeanHistogram,
                IQRPlot);

DEFINE_QML_ENUM(Q_GADGET, PlotDispersionType,
                None,
                StdErr,
                StdDev);

DEFINE_QML_ENUM(Q_GADGET, PlotDispersionVisualType,
                Bars,
                Area,
                StdDev);

class CorrelationPlotItem : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(QVector<double> data MEMBER _data)
    Q_PROPERTY(double scrollAmount MEMBER _scrollAmount WRITE setScrollAmount NOTIFY scrollAmountChanged)
    Q_PROPERTY(double rangeSize READ rangeSize NOTIFY rangeSizeChanged)
    Q_PROPERTY(QVector<int> selectedRows MEMBER _selectedRows WRITE setSelectedRows)
    Q_PROPERTY(QVector<QColor> rowColors MEMBER _rowColors WRITE setRowColors)
    Q_PROPERTY(QStringList columnNames MEMBER _labelNames WRITE setLabelNames)
    Q_PROPERTY(QStringList rowNames MEMBER _graphNames)
    Q_PROPERTY(size_t columnCount MEMBER _columnCount WRITE setColumnCount)
    Q_PROPERTY(size_t rowCount MEMBER _rowCount)
    Q_PROPERTY(int elideLabelWidth MEMBER _elideLabelWidth WRITE setElideLabelWidth)
    Q_PROPERTY(bool showColumnNames MEMBER _showColumnNames WRITE setShowColumnNames)
    Q_PROPERTY(bool showGridLines MEMBER _showGridLines WRITE setShowGridLines NOTIFY plotOptionsChanged)
    Q_PROPERTY(bool showLegend MEMBER _showLegend WRITE setShowLegend NOTIFY plotOptionsChanged)
    Q_PROPERTY(int plotScaleType MEMBER _plotScaleType WRITE setPlotScaleType NOTIFY plotOptionsChanged)
    Q_PROPERTY(int plotAveragingType MEMBER _plotAveragingType WRITE setPlotAveragingType NOTIFY plotOptionsChanged)
    Q_PROPERTY(int plotDispersionType MEMBER _plotDispersionType WRITE setPlotDispersionType NOTIFY plotOptionsChanged)
    Q_PROPERTY(int plotDispersionVisualType MEMBER _plotDispersionVisualType WRITE setPlotDispersionVisualType NOTIFY plotOptionsChanged)
    Q_PROPERTY(QString xAxisLabel MEMBER _xAxisLabel WRITE setXAxisLabel NOTIFY plotOptionsChanged)
    Q_PROPERTY(QString yAxisLabel MEMBER _yAxisLabel WRITE setYAxisLabel NOTIFY plotOptionsChanged)

public:
    explicit CorrelationPlotItem(QQuickItem* parent = nullptr);
    void paint(QPainter* painter);

    Q_INVOKABLE void savePlotImage(const QUrl& url, const QStringList& extensions);

    void setPlotScaleType(const int& plotScaleType);
    void setPlotDispersionType(const int& plotDispersionType);
    void setXAxisLabel(const QString& plotXAxisLabel);
    void setYAxisLabel(const QString& plotYAxisLabel);
    void setPlotAveragingType(const int& plotAveragingType);
    void setPlotDispersionVisualType(const int& plotDispersionVisualType);

protected:
    void routeMouseEvent(QMouseEvent* event);
    void routeWheelEvent(QWheelEvent* event);

    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);
    virtual void mouseMoveEvent(QMouseEvent* event);
    virtual void hoverMoveEvent(QHoverEvent* event);
    virtual void hoverLeaveEvent(QHoverEvent* event);
    virtual void mouseDoubleClickEvent(QMouseEvent* event);
    virtual void wheelEvent(QWheelEvent* event);

    void buildPlot();

private:
    const int MAX_SELECTED_ROWS_BEFORE_MEAN = 1000;

    QCPLayer* _textLayer = nullptr;
    QCPLegend* _legend = nullptr;
    QCPAbstractPlottable* _hoverPlottable = nullptr;
    QPointF _hoverPoint;
    QCPItemText* _hoverLabel = nullptr;
    QCPItemRect* _hoverColorRect = nullptr;
    QCPItemTracer* _itemTracer = nullptr;
    QFont _defaultFont9Pt;

    QCustomPlot _customPlot;
    size_t _columnCount = 0;
    size_t _rowCount = 0;
    int _elideLabelWidth = 120;
    QStringList _labelNames;
    QStringList _graphNames;
    QVector<double> _data;
    QVector<int> _selectedRows;
    QVector<QColor> _rowColors;
    bool _showColumnNames = true;
    bool _showGridLines = true;
    bool _showLegend = false;
    int _plotScaleType = static_cast<int>(PlotScaleType::Raw);
    int _plotAveragingType = static_cast<int>(PlotAveragingType::Individual);
    int _plotDispersionType = static_cast<int>(PlotDispersionType::None);
    int _plotDispersionVisualType = static_cast<int>(PlotDispersionVisualType::Bars);
    double _scrollAmount = 0.0f;
    QString _xAxisLabel;
    QString _yAxisLabel;

    void populateMeanLinePlot();
    void populateMedianLinePlot();
    void populateLinePlot();
    void populateMeanHistogramPlot();
    void populateIQRPlot();
    void populateStdDevPlot();
    void populateStdErrorPlot();
    void plotDispersion(QVector<double> stdDevs, QString name);

public:
    Q_INVOKABLE void refresh();

private:
    void setSelectedRows(const QVector<int>& selectedRows);
    void setRowColors(const QVector<QColor>& rowColors);
    void setLabelNames(const QStringList& labelNames);
    void setElideLabelWidth(int elideLabelWidth);
    void setColumnCount(size_t columnCount);
    void setShowColumnNames(bool showColumnNames);
    void setShowGridLines(bool showGridLines);
    void setShowLegend(bool showLegend);
    void setScrollAmount(double scrollAmount);

    void scaleXAxis();
    QVector<double> meanAverageData(double &min, double &max);

    double rangeSize();
    double columnLabelSize();
    double columnAxisWidth();

    void configureLegend();

private slots:
    void onCustomReplot();
    void updatePlotSize();
    void showTooltip();
    void hideTooltip();

signals:
    void rightClick();
    void scrollAmountChanged();
    void rangeSizeChanged();
    void plotOptionsChanged();
};
#endif // CORRELATIONPLOTITEM_H
