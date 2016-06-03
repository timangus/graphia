#ifndef GRAPHARRAY_H
#define GRAPHARRAY_H

#include "igrapharray.h"
#include "igraph.h"

#include <vector>
#include <mutex>

template<typename Index, typename Element, typename Locking = void>
class GenericGraphArray : public IGraphArray
{
private:
    static_assert(std::is_nothrow_move_constructible<Element>::value,
                  "GraphArray Element needs a noexcept move constructor");

    using MaybeLock = u::MaybeLock<std::recursive_mutex, Locking>;

protected:
    const IGraph* _graph;
    std::vector<Element> _array;
    mutable std::recursive_mutex _mutex;
    Element _defaultValue;

public:
    explicit GenericGraphArray(const IGraph& graph) :
        _graph(&graph), _defaultValue()
    {}

    GenericGraphArray(const IGraph& graph, const Element& defaultValue) :
        _graph(&graph), _defaultValue(defaultValue)
    {
        fill(defaultValue);
    }

    GenericGraphArray(const GenericGraphArray& other) :
        _graph(other._graph),
        _array(other._array),
        _mutex(),
        _defaultValue(other._defaultValue)
    {}

    GenericGraphArray(GenericGraphArray&& other) :
        _graph(other._graph),
        _array(std::move(other._array)),
        _mutex(),
        _defaultValue(other._defaultValue)
    {}

    virtual ~GenericGraphArray() {}

    GenericGraphArray& operator=(const GenericGraphArray& other)
    {
        Q_ASSERT(_graph == other._graph);
        _array = other._array;
        _mutex.native_handle();
        _defaultValue = other._defaultValue;

        return *this;
    }

    GenericGraphArray& operator=(GenericGraphArray&& other)
    {
        Q_ASSERT(_graph == other._graph);
        _array = std::move(other._array);
        _mutex.native_handle();
        _defaultValue = std::move(other._defaultValue);

        return *this;
    }

    Element& operator[](Index index)
    {
        MaybeLock lock(_mutex);
        Q_ASSERT(index >= 0 && index < size());
        return _array[index];
    }

    const Element& operator[](Index index) const
    {
        MaybeLock lock(_mutex);
        Q_ASSERT(index >= 0 && index < size());
        return _array[index];
    }

    Element& at(Index index)
    {
        MaybeLock lock(_mutex);
        Q_ASSERT(index >= 0 && index < size());
        return _array.at(index);
    }

    const Element& at(Index index) const
    {
        MaybeLock lock(_mutex);
        Q_ASSERT(index >= 0 && index < size());
        return _array.at(index);
    }

    Element get(Index index) const
    {
        MaybeLock lock(_mutex);
        Q_ASSERT(index >= 0 && index < size());
        return _array[index];
    }

    void set(Index index, const Element& value)
    {
        MaybeLock lock(_mutex);
        Q_ASSERT(index >= 0 && index < size());
        _array[index] = value;
    }

    //FIXME these iterators do not lock when locking is enabled; need to wrap in own iterator types
    typename std::vector<Element>::iterator begin() { return _array.begin(); }
    typename std::vector<Element>::const_iterator begin() const { return _array.begin(); }
    typename std::vector<Element>::iterator end() { return _array.end(); }
    typename std::vector<Element>::const_iterator end() const { return _array.end(); }

    int size() const
    {
        MaybeLock lock(_mutex);
        return static_cast<int>(_array.size());
    }

    bool empty() const
    {
        MaybeLock lock(_mutex);
        return _array.empty();
    }

    void fill(const Element& value)
    {
        MaybeLock lock(_mutex);
        std::fill(_array.begin(), _array.end(), value);
    }

    void resetElements()
    {
        MaybeLock lock(_mutex);
        fill(_defaultValue);
    }

    void dumpToQDebug(int detail) const
    {
        qDebug() << "GraphArray size" << _array.size();

        if(detail > 0)
        {
            for(Element e : _array)
                qDebug() << e;
        }
    }

protected:
    void resize(int size)
    {
        MaybeLock lock(_mutex);
        resize_(size);
    }

    template<typename T = Element> typename std::enable_if<std::is_copy_constructible<T>::value>::type
    resize_(int size)
    {
        _array.resize(size, _defaultValue);
    }

    template<typename T = Element> typename std::enable_if<!std::is_copy_constructible<T>::value>::type
    resize_(int size)
    {
        _array.resize(size);
    }

    void invalidate()
    {
        MaybeLock lock(_mutex);
        _graph = nullptr;
    }
};

template<typename Element, typename Locking = void>
class NodeArray : public GenericGraphArray<NodeId, Element, Locking>
{
public:
    explicit NodeArray(const IGraph& graph) :
        GenericGraphArray<NodeId, Element, Locking>(graph)
    {
        this->resize(graph.nextNodeId());
        graph.insertNodeArray(this);
    }

    NodeArray(const IGraph& graph, const Element& defaultValue) :
        GenericGraphArray<NodeId, Element, Locking>(graph, defaultValue)
    {
        this->resize(graph.nextNodeId());
        graph.insertNodeArray(this);
    }

    NodeArray(const NodeArray& other) :
        GenericGraphArray<NodeId, Element, Locking>(other)
    {
        this->_graph->insertNodeArray(this);
    }

    NodeArray(NodeArray&& other) :
        GenericGraphArray<NodeId, Element, Locking>(std::move(other))
    {
        this->_graph->insertNodeArray(this);
    }

    NodeArray& operator=(const NodeArray& other)
    {
        GenericGraphArray<NodeId, Element, Locking>::operator=(other);
        return *this;
    }

    NodeArray& operator=(NodeArray&& other)
    {
        GenericGraphArray<NodeId, Element, Locking>::operator=(std::move(other));
        return *this;
    }

    ~NodeArray()
    {
        if(this->_graph != nullptr)
            this->_graph->eraseNodeArray(this);
    }
};

template<typename Element, typename Locking = void>
class EdgeArray : public GenericGraphArray<EdgeId, Element, Locking>
{
public:
    explicit EdgeArray(const IGraph& graph) :
        GenericGraphArray<EdgeId, Element, Locking>(graph)
    {
        this->resize(graph.nextEdgeId());
        graph.insertEdgeArray(this);
    }

    EdgeArray(const IGraph& graph, const Element& defaultValue) :
        GenericGraphArray<EdgeId, Element, Locking>(graph, defaultValue)
    {
        this->resize(graph.nextEdgeId());
        graph.insertEdgeArray(this);
    }

    EdgeArray(const EdgeArray& other) :
        GenericGraphArray<EdgeId, Element, Locking>(other)
    {
        this->_graph->insertEdgeArray(this);
    }

    EdgeArray(EdgeArray&& other) :
        GenericGraphArray<EdgeId, Element, Locking>(std::move(other))
    {
        this->_graph->insertEdgeArray(this);
    }

    EdgeArray& operator=(const EdgeArray& other)
    {
        GenericGraphArray<EdgeId, Element, Locking>::operator=(other);
        return *this;
    }

    EdgeArray& operator=(EdgeArray&& other)
    {
        GenericGraphArray<EdgeId, Element, Locking>::operator=(std::move(other));
        return *this;
    }

    ~EdgeArray()
    {
        if(this->_graph != nullptr)
            this->_graph->eraseEdgeArray(this);
    }
};

template<typename Element, typename Locking = void>
class ComponentArray : public GenericGraphArray<ComponentId, Element, Locking>
{
public:
    explicit ComponentArray(const IGraph& graph) :
        GenericGraphArray<ComponentId, Element, Locking>(graph)
    {
        Q_ASSERT(graph.isComponentManaged());
        this->resize(graph.numComponentArrays());
        graph.insertComponentArray(this);
    }

    ComponentArray(const IGraph& graph, const Element& defaultValue) :
        GenericGraphArray<ComponentId, Element, Locking>(graph, defaultValue)
    {
        Q_ASSERT(graph.isComponentManaged());
        this->resize(graph.numComponentArrays());
        graph.insertComponentArray(this);
    }

    ComponentArray(const ComponentArray& other) :
        GenericGraphArray<ComponentId, Element, Locking>(other)
    {
        Q_ASSERT(this->_graph->isComponentManaged());
        this->_graph->insertComponentArray(this);
    }

    ComponentArray(ComponentArray&& other) :
        GenericGraphArray<ComponentId, Element, Locking>(std::move(other))
    {
        Q_ASSERT(this->_graph->isComponentManaged());
        this->_graph->insertComponentArray(this);
    }

    ComponentArray& operator=(const ComponentArray& other)
    {
        GenericGraphArray<ComponentId, Element, Locking>::operator=(other);
        return *this;
    }

    ComponentArray& operator=(ComponentArray&& other)
    {
        GenericGraphArray<ComponentId, Element, Locking>::operator=(std::move(other));
        return *this;
    }

    ~ComponentArray()
    {
        if(this->_graph != nullptr)
            this->_graph->eraseComponentArray(this);
    }
};

#endif // GRAPHARRAY_H
