#include "nodepositions.h"

QVector3D MeanPosition::mean(int samples) const
{
    samples = std::min(samples, static_cast<int>(size()));

    QVector3D result;
    float reciprocal = 1.0f / samples;

    for(int i = 0; i > -samples; i--)
        result += at(i) * reciprocal;

    return result;
}

NodePositions::NodePositions(MutableGraph& graph) :
    NodeArray<MeanPosition>(graph),
    _updated(false),
    _scale(1.0f),
    _smoothing(1)
{}

const QVector3D& NodePositions::get(NodeId nodeId) const
{
    return _array[nodeId].front();
}

const QVector3D NodePositions::getScaledAndSmoothed(NodeId nodeId) const
{
    return _array[nodeId].mean(_smoothing) * _scale;
}

void NodePositions::update(const Graph& graph, std::function<QVector3D(NodeId, const QVector3D&)> f,
                           float scale, int smoothing)
{
    Q_ASSERT(smoothing <= MAX_SMOOTHING);
    std::unique_lock<std::recursive_mutex> lock(_mutex);

    setScale(scale);
    setSmoothing(smoothing);

    for(NodeId nodeId : graph.nodeIds())
        _array.at(nodeId).push_back(f(nodeId, _array.at(nodeId).front()));

    _updated = true;
}

void NodePositions::executeIfUpdated(std::function<void()> f)
{
    std::unique_lock<std::recursive_mutex> lock(_mutex);
    if(_updated)
    {
        f();
        _updated = false;
    }
}

QVector3D NodePositions::centreOfMass(const NodePositions& nodePositions,
                                      const std::vector<NodeId>& nodeIds)
{
    float reciprocal = 1.0f / nodeIds.size();
    QVector3D centreOfMass = QVector3D();

    for(auto nodeId : nodeIds)
        centreOfMass += (nodePositions.get(nodeId) * reciprocal);

    return centreOfMass;
}

QVector3D NodePositions::centreOfMassScaledAndSmoothed(const NodePositions& nodePositions, const std::vector<NodeId>& nodeIds)
{
    float reciprocal = 1.0f / nodeIds.size();
    QVector3D centreOfMass = QVector3D();

    for(auto nodeId : nodeIds)
        centreOfMass += (nodePositions.getScaledAndSmoothed(nodeId) * reciprocal);

    return centreOfMass;
}

std::vector<QVector3D> NodePositions::positionsVector(const NodePositions& nodePositions, const std::vector<NodeId>& nodeIds)
{
    std::vector<QVector3D> positionsVector;
    for(NodeId nodeId : nodeIds)
        positionsVector.push_back(nodePositions.get(nodeId));

    return positionsVector;
}

std::vector<QVector3D> NodePositions::positionsVectorScaled(const NodePositions& nodePositions, const std::vector<NodeId>& nodeIds)
{
    std::vector<QVector3D> positionsVector;
    for(NodeId nodeId : nodeIds)
        positionsVector.push_back(nodePositions.getScaledAndSmoothed(nodeId));

    return positionsVector;
}

// http://stackoverflow.com/a/24818473
BoundingSphere NodePositions::boundingSphere(const NodePositions& nodePositions, const std::vector<NodeId>& nodeIds)
{
    QVector3D center = nodePositions.getScaledAndSmoothed(nodeIds.front());
    float radius = 0.0001f;
    QVector3D pos, diff;
    float len, alpha, alphaSq;

    for(int i = 0; i < 2; i++)
    {
        for(auto& nodeId : nodeIds)
        {
            pos = nodePositions.getScaledAndSmoothed(nodeId);
            diff = pos - center;
            len = diff.length();

            if(len > radius)
            {
                alpha = len / radius;
                alphaSq = alpha * alpha;
                radius = 0.5f * (alpha + 1.0f / alpha) * radius;
                center = 0.5f * ((1.0f + 1.0f / alphaSq) * center + (1.0f - 1.0f / alphaSq) * pos);
            }
        }
    }

    for(auto& nodeId : nodeIds)
    {
        pos = nodePositions.getScaledAndSmoothed(nodeId);
        diff = pos - center;
        len = diff.length();
        if(len > radius)
        {
            radius = (radius + len) / 2.0f;
            center = center + ((len - radius) / len * diff);
        }
    }

    return BoundingSphere(center, radius);
}
