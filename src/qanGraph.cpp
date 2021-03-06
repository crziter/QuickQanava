/*
    This file is part of Quick Qanava library.

    Copyright (C) 2008-2015 Benoit AUTHEMAN

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

//-----------------------------------------------------------------------------
// This file is a part of the QuickQanava software library.
//
// \file	qanGraph.cpp
// \author	benoit@destrat.io
// \date	2004 February 15
//-----------------------------------------------------------------------------

// Qt headers
#include <QQmlProperty>
#include <QVariant>
#include <QQmlEngine>
#include <QQmlComponent>

// GTpo headers
#include "gtpoRandomGraph.h"

// Qanava headers
#include "./qanGraph.h"
#include "./qanNavigable.h"

namespace qan { // ::qan

/* Graph Object Management *///------------------------------------------------
Graph::Graph( QQuickItem* parent ) noexcept :
    gtpo::GenGraph< qan::Config >( parent ),
    _styleManager{ SharedStyleManager{ new qan::StyleManager( this ) } }
{
    setContainerItem( this );
    setAntialiasing( true );
    setSmooth( true );
}

void    Graph::qmlClearGraph( ) noexcept
{
    qan::Graph::clear();
}

void    Graph::clear( ) noexcept
{
    _selectedNodes.clear();
    gtpo::GenGraph< qan::Config >::clear();
    _styleManager->clear();
}

void    Graph::addControlNode( qan::Node* node )
{
    if ( node != nullptr )
        gtpo::GenGraph< qan::Config >::addControlNode( node->shared_from_this( ) );
}

void    Graph::removeControlNode( qan::Node* node )
{
    if ( node == nullptr )
        return;
    gtpo::GenGraph< qan::Config >::removeControlNode( node->shared_from_this( ) );
}

QQuickItem* Graph::graphChildAt(qreal x, qreal y) const
{
    if ( getContainerItem() == nullptr )
        return nullptr;

    const QList<QQuickItem *> children = getContainerItem()->childItems();
    //qDebug() << "graphChildAt(): x=" << x << " y=" << y;
    //qDebug() << " \tchildren=" << children;
    for (int i = children.count()-1; i >= 0; --i) {
        QQuickItem *child = children.at(i);
        QPointF point = mapToItem( child, QPointF(x, y) );  // Map coordinates to the child element's coordinate space
        if ( child->isVisible() &&
             child->contains( point ) &&    // Note 20160508: childAt do not call contains()
             point.x() >= 0 &&
             child->width() > point.x() &&
             point.y() >= 0 &&
             child->height() > point.y() ) {
            if ( child->inherits( "qan::Group" ) ) {  // For group, look in group childs
                qan::Group* group = qobject_cast<qan::Group*>( child );
                if ( group != nullptr && group->getContainer() != nullptr ) {
                    const QList<QQuickItem *> groupChildren = group->getContainer()->childItems();
                    for (int gc = groupChildren.count()-1; gc >= 0; --gc) {
                        QQuickItem *groupChild = groupChildren.at(gc);
                        point = mapToItem( groupChild, QPointF(x, y) ); // Map coordinates to group child element's coordinate space
                        if ( groupChild->isVisible() &&
                             groupChild->contains( point ) &&
                             point.x() >= 0 &&
                             groupChild->width() > point.x() &&
                             point.y() >= 0 &&
                             groupChild->height() > point.y() ) {
                            return groupChild;
                        }
                    }
                }
            }
            return child;
        }
    }
    return nullptr;
}

qan::Group* Graph::groupAt( const QPointF& p, const QSizeF& s ) const
{
    for ( const auto& g : getGroups() ) {
        if ( g == nullptr )
            continue;
        if ( QRectF{ g->position(), QSizeF{ g->width(), g->height() } }.contains( QRectF{ p, s } ) )
            return g.get();
    }
    return nullptr;
}

void    Graph::setContainerItem( QQuickItem* containerItem )
{
    if ( containerItem != nullptr &&
         containerItem != _containerItem.data() ) {
        _containerItem = containerItem;
        emit containerItemChanged();
    }
}
//-----------------------------------------------------------------------------

/* Selection Management *///---------------------------------------------------
void    Graph::setSelectionPolicy( SelectionPolicy selectionPolicy )
{
    if ( selectionPolicy == _selectionPolicy )  // Binding loop protection
        return;
    _selectionPolicy = selectionPolicy;
    if ( selectionPolicy == SelectionPolicy::NoSelection )
        clearSelection();
    emit selectionPolicyChanged( );
}

void    Graph::setSelectionColor( QColor selectionColor )
{
    _selectionColor = selectionColor;
    for ( auto& node : _selectedNodes ) {   // Update visible selection hilight item
        if ( node != nullptr &&
             node->getSelectionItem() != nullptr ) {
            QQuickItem* selectionItem = node->getSelectionItem();
            selectionItem->setProperty( "color", QVariant::fromValue( QColor(0,0,0,0) ) );
            QObject* rectangleBorder = node->getSelectionItem()->property( "border" ).value<QObject*>();
            if ( rectangleBorder != nullptr )
                rectangleBorder->setProperty( "color", selectionColor );
        }
    }
    emit selectionColorChanged();
}

void    Graph::setSelectionWeight( qreal selectionWeight )
{
    if ( qFuzzyCompare( selectionWeight, _selectionWeight ) )   // Never 0
        return;
    _selectionWeight = selectionWeight;
    for ( auto& node : _selectedNodes ) {   // Update visible selection hilight item
        if ( node != nullptr )
            node->configureSelectionItem( selectionWeight, getSelectionMargin() );
    }
    emit selectionWeightChanged();
}

void    Graph::setSelectionMargin( qreal selectionMargin )
{
    if ( qFuzzyCompare( selectionMargin, _selectionMargin ) )   // Never 0
        return;
    _selectionMargin = selectionMargin;
    for ( auto& node : _selectedNodes ) {   // Update visible selection hilight item
        if ( node != nullptr )
            node->configureSelectionItem( getSelectionWeight(), selectionMargin );
    }
    emit selectionMarginChanged();
}

bool    Graph::selectNode( qan::Node& node, Qt::KeyboardModifiers modifiers )
{
    if ( getSelectionPolicy() == SelectionPolicy::NoSelection )
        return false;

    bool selectNode{ false };
    bool ctrlPressed = modifiers & Qt::ControlModifier;

    if ( node.getSelected() ) {
        if ( ctrlPressed )          // Click on a selected node + CTRL = deselect node
            removeFromSelection( node );
        /*else {
            clearSelection();       // Clicking on a node without CTRL disable the existing multiple selection
            addToSelection( node );
        }*/ // Note: no or we loose multiple selection dragging
    } else {
        switch ( getSelectionPolicy() ) {
        case SelectionPolicy::SelectOnClick:
            selectNode = true;        // Click on an unselected node with SelectOnClick = select node
            if ( !ctrlPressed )
                clearSelection();
            break;
        case SelectionPolicy::SelectOnCtrlClick:
            selectNode = ctrlPressed; // Click on an unselected node with CTRL pressed and SelectOnCtrlClick = select node
            break;
        default: break;
        }
    }
    if ( selectNode ) {
        addToSelection( node );
        return true;
    }
    return false;
}

void    Graph::addToSelection( qan::Node& node )
{
    if ( !_selectedNodes.contains( &node ) ) {
        _selectedNodes.append( &node );
        node.configureSelectionItem( getSelectionColor(), getSelectionWeight(), getSelectionMargin() );
        node.setSelected( true );
    }
}

void    Graph::removeFromSelection( qan::Node& node )
{
    if ( _selectedNodes.contains( &node ) )
        _selectedNodes.remove( &node );
    node.setSelected( false );
}

void    Graph::clearSelection()
{
    for ( auto& node : _selectedNodes )
        if ( node != nullptr )
            node->setSelected( false );
    _selectedNodes.clear();
}

void    Graph::mousePressEvent( QMouseEvent* event )
{
    if ( event->button() == Qt::LeftButton ) {
        clearSelection();
        forceActiveFocus();
    }
    event->ignore();
    qan::Config::GraphBase::mousePressEvent( event );
}
//-----------------------------------------------------------------------------

/* Delegates Management *///---------------------------------------------------
auto    Graph::createFromDelegate( QQmlComponent* component ) -> QQuickItem*
{
    if ( component == nullptr ) {
        qDebug() << "qan::Graph::createFromDelegate(): Error called with a nullptr delegate component";
        return nullptr;
    }
    QQuickItem* item = nullptr;
    if ( component->isReady() ) {
        QObject* object = component->create( );
        if ( object != nullptr && !component->isError() ) {
            QQmlEngine::setObjectOwnership( object, QQmlEngine::CppOwnership );
            item = qobject_cast< QQuickItem* >( object );
            item->setVisible( true );
            item->setParentItem( getContainerItem() );
        } else {
            qDebug() << "qan::Graph::createFromDelegate(): Failed to create a concrete QQuickItem from QML component:";
            qDebug() << "\t" << component->errorString();
        }
    } else {
        qDebug() << "qan::Graph::createFromDelegate(): QML component is not ready:";
        qDebug() << "\t" << component->errorString();
    }

    return item;
}

void Graph::setCppOwnership( QQuickItem* item )
{
    if ( item == nullptr )
        return;
    QQmlEngine::setObjectOwnership( item, QQmlEngine::CppOwnership );
    for ( auto childItem: item->childItems() )
        qan::Graph::setCppOwnership( childItem );
}

void    Graph::registerNodeDelegate( QString nodeClass, QQmlComponent* nodeComponent )
{
    if ( nodeClass.isEmpty() || nodeComponent == nullptr ) {
        qDebug() << "qan::Graph::registerNodeDelegate(): Error: Trying to register a node delegate with empty class name or null delegate component";
        return;
    }
    _nodeClassComponents.insert( nodeClass, nodeComponent );
}

void    Graph::registerEdgeDelegate( QString edgeClass, QQmlComponent* edgeComponent )
{
    if ( edgeClass.isEmpty() || edgeComponent == nullptr ) {
        qDebug() << "qan::Graph::registerEdgeDelegate(): Error: Trying to register an edge delegate with empty class name or null delegate component";
        return;
    }
    _edgeClassComponents.insert( edgeClass, edgeComponent );
}

void    Graph::registerGroupDelegate( QString groupClass, QQmlComponent* groupComponent )
{
    if ( groupClass.isEmpty() || groupComponent == nullptr ) {
        qDebug() << "qan::Graph::registerGroupDelegate(): Error: Trying to register a group delegate with empty class name or null delegate component";
        return;
    }
    _groupClassComponents.insert( groupClass, groupComponent );
}

QQuickItem* Graph::createNodeItem( QString nodeClass )
{
    if ( _nodeClassComponents.contains( nodeClass ) )
        return createFromDelegate( _nodeClassComponents.value( nodeClass ) );
    return new QQuickItem();    // Is should never happen, otherwise it is leaked.
}

QQuickItem* Graph::createEdgeItem( QString edgeClass )
{
    if ( _edgeClassComponents.contains( edgeClass ) )
        return createFromDelegate( _edgeClassComponents.value( edgeClass ) );
    return new QQuickItem();    // Is should never happen, otherwise it is leaked.
}

QQuickItem* Graph::createRectangle( QQuickItem* parent )
{
    // Initialize rectangle component for the first factory call.
    if ( _rectangleComponent == nullptr ) {
        QQmlEngine* engine = qmlEngine( this );
        if ( engine != nullptr ) {
            _rectangleComponent = std::make_unique<QQmlComponent>(engine);
            QString componentQml = "import QtQuick 2.6\n  Rectangle{ }";
            _rectangleComponent ->setData( componentQml.toUtf8(), QUrl() );
            if ( !_rectangleComponent->isReady() ) {
                qDebug() << "qan::Graph::createRectangle(): Error: Can't create at Qt Quick Rectangle object:";
                qDebug() << componentQml;
                qDebug() << "QML Component status=" << _rectangleComponent->status();
                qDebug() << "QML Component errors=" << _rectangleComponent->errors();
            }
        }
    }

    // Create a Qt Quick Rectangle object
    if ( _rectangleComponent != nullptr &&
         _rectangleComponent->isReady() ) {
        QObject* object = _rectangleComponent->create();
        QQuickItem* rectangle = qobject_cast< QQuickItem* >( object );
        if ( rectangle != nullptr ) {
            rectangle->setVisible( false );
            if ( parent != nullptr )
                rectangle->setParentItem( parent );
            QQmlEngine::setObjectOwnership( rectangle, QQmlEngine::CppOwnership );
        }
        if ( rectangle == nullptr && object != nullptr ) {
            qDebug() << "qan::Graph::createRectangle(): Error: A Qt Quick Object has been created, but it is not a Quick Canvas.";
            delete object;      // Somtehing has been created, but it is not a rectangle !
            return nullptr;
        }
        return rectangle;
    } else {
        qDebug() << "qan::Graph::createRectangle(): Error: Can't create a Qt Quick Rectangle Object.";
    }
    return nullptr;
}
//-----------------------------------------------------------------------------

/* Graph Factories *///--------------------------------------------------------
qan::Node*  Graph::insertNode( QQmlComponent* nodeComponent )
{
    if ( nodeComponent == nullptr ) {
        if ( _nodeClassComponents.contains( "qan::Node" ) )
            nodeComponent = _nodeClassComponents.value( "qan::Node", nullptr );
    }
    if ( nodeComponent == nullptr ) {
        qDebug() << "qan::Graph::insertNode(): Error: Can't find a valid node delegate component.";
        return nullptr;
    }
    qan::Node* node = static_cast< qan::Node* >( createFromDelegate( nodeComponent ) );
    node->setSelectionItem( createRectangle( node ) );
    if ( node != nullptr ) {
        GTpoGraph::insertNode( std::shared_ptr<qan::Node>{node} );

        connect( node, &qan::Node::nodeClicked, this, &qan::Graph::nodeClicked );
        connect( node, &qan::Node::nodeRightClicked, this, &qan::Graph::nodeRightClicked );
        connect( node, &qan::Node::nodeDoubleClicked, this, &qan::Graph::nodeDoubleClicked );
    } else
        qDebug() << "qan::Graph::insertNode(): Warning: Node creation failed with the corresponding delegate";
    return node;
}

qan::Graph::WeakNode    Graph::insertNode( SharedNode node )
{
    return GTpoGraph::insertNode( node );
}

qan::Node*  Graph::insertNode( QString nodeClassName )
{
    if ( nodeClassName.isEmpty() )
        return nullptr;
    QQmlComponent* nodeComponent = nullptr;
    if ( _nodeClassComponents.contains( nodeClassName ) )
        nodeComponent = _nodeClassComponents.value( nodeClassName, nullptr );
    if ( nodeComponent == nullptr )
        return nullptr;
    qan::Node* node = static_cast< qan::Node* >( createFromDelegate( nodeComponent ) );
    node->setSelectionItem( createRectangle( node ) );
    SharedNode sharedNode{ node };
    if ( node != nullptr ) {
        GTpoGraph::insertNode( sharedNode );
        qan::NodeStyle* defaultStyle = qobject_cast< qan::NodeStyle* >( getStyleManager()->getDefaultNodeStyle( nodeClassName ) );
        if ( defaultStyle != nullptr )
            node->setStyle( defaultStyle );
        connect( node, &qan::Node::nodeClicked, this, &qan::Graph::nodeClicked );
        connect( node, &qan::Node::nodeRightClicked, this, &qan::Graph::nodeRightClicked );
        connect( node, &qan::Node::nodeDoubleClicked, this, &qan::Graph::nodeDoubleClicked );
    }
    return node;
}

qan::Node*  Graph::insertNode( QVariant nodeArgument )
{
    if ( !nodeArgument.isValid() ) {
        qDebug() << "qan::Graph::insertNode(QVariant): Error: Called with invalid QVariant.";
        return nullptr;
    }
    if ( nodeArgument.type() == QVariant::String ) {
        return insertNode( nodeArgument.toString( ) );
    } else if ( nodeArgument.canConvert<QObject*>() ) {
        QQmlComponent* nodeComponent = qobject_cast< QQmlComponent* >( nodeArgument.value< QObject* >() );
        if ( nodeComponent != nullptr )
            return insertNode( nodeComponent );
    }
    qDebug() << "qan::Graph::insertNode(QVariant): Error: Called with unsupported QVariant argument type (must be either QString or QQmlComponent.";
    return nullptr;
}

Graph::WeakNode    Graph::createNode( const std::string& className )
{
    if ( className.size() == 0 )
        return WeakNode{};
    qan::Node* node = insertNode( QString::fromStdString( className ) );
    return ( node != nullptr ? node->shared_from_this() : WeakNode{} ); // Note 20160213: shared_from_this() could be used since the node has already been added to graph, it
}

void    Graph::removeNode( qan::Node* node )
{
    if ( node == nullptr )
        return;
    WeakNode weakNode;
    try {
        weakNode = WeakNode{ node->shared_from_this() };
    } catch ( std::bad_weak_ptr ) { return; }
    GTpoGraph::removeNode( weakNode );
}

bool    Graph::isNode( QQuickItem* item ) const
{
    if ( !item->inherits( "qan::Node" ) )
        return false;   // "Fast" exit if item is not a qan::Node...
    qan::Node* node = qobject_cast< qan::Node* >( item );
    if ( node == nullptr )
        return false;
    return GTpoGraph::contains( node->shared_from_this() );
}

bool    Graph::isEdge( QQuickItem* item ) const
{
    if ( !item->inherits( "qan::Edge" ) )
        return false;   // "Fast" exit if item is not a qan::Edge...
    qan::Edge* edge = qobject_cast< qan::Edge* >( item );
    if ( edge == nullptr )
        return false;
    return GTpoGraph::contains( edge->shared_from_this() );
}

bool    Graph::isHyperEdge( QQuickItem* item ) const
{
    if ( !isEdge(item)) // "Fast" exit if item is not a qan::Edge...
        return false;
    qan::Edge* edge = qobject_cast< qan::Edge* >( item );
    if ( edge == nullptr )
        return false;
    return ( edge == nullptr ||
             ( !edge->getHDst().expired() &&
               edge->getHDst().lock() != nullptr ) );
}
//-----------------------------------------------------------------------------

/* Graph Edge Management *///--------------------------------------------------
qan::Edge*  Graph::insertEdge( QObject* source, QObject* destination, QQmlComponent* edgeComponent )
{
    auto sourceNode = qobject_cast<qan::Node*>(source);
    if ( sourceNode != nullptr ) {
            if ( qobject_cast<qan::Node*>(destination) != nullptr )
                return insertEdge( sourceNode, qobject_cast<qan::Node*>( destination ), edgeComponent );
            else if ( qobject_cast<qan::Edge*>(destination) != nullptr )
                return insertEdge( sourceNode, qobject_cast<qan::Edge*>( destination ), edgeComponent );
    }
    qDebug() << "qan::Graph::insertEdge(): Error: Unable to find a valid insertEdge() method for arguments " << source << " and " << destination;
    return nullptr;
}

qan::Edge*  Graph::insertEdge( qan::Node* source, qan::Node* destination, QQmlComponent* edgeComponent )
{
    Q_UNUSED( edgeComponent );
    return insertEdge( "qan::Edge", source, destination );
}

qan::Edge*  Graph::insertEdge( qan::Node* source, qan::Edge* destination, QQmlComponent* edgeComponent )
{
    Q_UNUSED( edgeComponent );
    return insertEdge( "qan::Edge", source, destination );
}

qan::Edge*  Graph::insertEdge( QString edgeClassName, QObject* source, QObject* destination, QQmlComponent* edgeComponent )
{
    auto sourceNode = qobject_cast<qan::Node*>(source);
    if ( sourceNode != nullptr ) {
            if ( qobject_cast<qan::Node*>(destination) != nullptr )
                return insertEdge( edgeClassName, sourceNode, qobject_cast<qan::Node*>( destination ), edgeComponent );
            else if ( qobject_cast<qan::Edge*>(destination) != nullptr )
                return insertEdge( edgeClassName, sourceNode, qobject_cast<qan::Edge*>( destination ), edgeComponent );
    }
    qDebug() << "qan::Graph::insertEdge(): Error: Unable to find a valid insertEdge() method for arguments " << source << " and " << destination;
    return nullptr;
}

qan::Edge*  Graph::insertEdge( QString edgeClassName, qan::Node* source, qan::Node* destination, QQmlComponent* edgeComponent )
{
    if ( source == nullptr || destination == nullptr )
        return nullptr;
    if ( edgeComponent == nullptr && _edgeClassComponents.contains( edgeClassName ) )
        edgeComponent = _edgeClassComponents.value( edgeClassName, nullptr );
    if ( edgeComponent == nullptr )
        return nullptr;
    qan::Edge* edge = nullptr;
    try {
        edge = static_cast< qan::Edge* >( createFromDelegate( edgeComponent ) );
        if ( edge != nullptr ) {
            edge->setSourceItem( source );
            edge->setDestinationItem( destination );

            WeakNode sharedSource{ source->shared_from_this() };
            WeakNode sharedDestination{ destination->shared_from_this() };
            edge->setSrc( sharedSource );
            edge->setDst( sharedDestination );
            GTpoGraph::insertEdge( SharedEdge{edge} );  // Note: Do not use shared_from_this()

            qan::EdgeStyle* defaultStyle = qobject_cast< qan::EdgeStyle* >( getStyleManager()->getDefaultEdgeStyle( "qan::Edge" ) );
            if ( defaultStyle != nullptr )
                edge->setStyle( defaultStyle );

            edge->setVisible( true );
            edge->updateItem();

            connect( edge, SIGNAL( edgeClicked( QVariant, QVariant ) ), this, SIGNAL( edgeClicked( QVariant, QVariant ) ) );
            connect( edge, SIGNAL( edgeRightClicked( QVariant, QVariant ) ), this, SIGNAL( edgeRightClicked( QVariant, QVariant ) ) );
            connect( edge, SIGNAL( edgeDoubleClicked( QVariant, QVariant ) ), this, SIGNAL( edgeDoubleClicked( QVariant, QVariant ) ) );
        }
    } catch ( gtpo::bad_topology_error e ) {
        qDebug() << "qan::Graph::insertEdge(): Error: Topology error:" << e.what();
    }
    catch ( ... ) {
        qDebug() << "qan::Graph::insertEdge(): Error: Topology error.";
    }
    return edge;
}

qan::Edge*  Graph::insertEdge( QString edgeClassName, qan::Node* source, qan::Edge* destination, QQmlComponent* edgeComponent )
{
    if ( source == nullptr ||
         destination == nullptr )
        return nullptr;
    if ( edgeComponent == nullptr &&
         _edgeClassComponents.contains( edgeClassName ) )
        edgeComponent = _edgeClassComponents.value( edgeClassName, nullptr );
    if ( edgeComponent == nullptr )
        return nullptr;
    qan::Edge* edge = nullptr;
    try {
        edge = static_cast< qan::Edge* >( createFromDelegate( edgeComponent ) );
        if ( edge != nullptr ) {
            edge->setSourceItem( source );
            edge->setDestinationEdge( destination );

            WeakNode sourcePtr{ source->shared_from_this() };
            WeakEdge destinationPtr{ destination->shared_from_this() };
            edge->setSrc( sourcePtr );
            edge->setHDst( destinationPtr );
            GTpoGraph::insertEdge( SharedEdge{edge} );  // Note: Do not use shared_from_this()

            qan::EdgeStyle* defaultStyle = qobject_cast< qan::EdgeStyle* >( getStyleManager()->getDefaultEdgeStyle( "qan::Edge" ) );
            if ( defaultStyle != nullptr )
                edge->setStyle( defaultStyle );
            edge->setVisible( true );
            edge->updateItem();
            connect( edge, SIGNAL( edgeClicked( QVariant, QVariant ) ),
                     this, SIGNAL( edgeClicked( QVariant, QVariant ) ) );
            connect( edge, SIGNAL( edgeRightClicked( QVariant, QVariant ) ),
                     this, SIGNAL( edgeRightClicked( QVariant, QVariant ) ) );
            connect( edge, SIGNAL( edgeDoubleClicked( QVariant, QVariant ) ),
                     this, SIGNAL( edgeDoubleClicked( QVariant, QVariant ) ) );
        }
    } catch ( gtpo::bad_topology_error e ) {
        qDebug() << "qan::Graph::insertEdge(): Error: Topology error:" << e.what();
    }
    catch ( ... ) {
        qDebug() << "qan::Graph::insertEdge(): Error: Topology error.";
    }
    return edge;
}

Graph::WeakEdge    Graph::createEdge( const std::string& className, WeakNode source, WeakNode destination )
{
    if ( className.size() == 0 )
        return WeakEdge();
    qan::Node* src = static_cast< qan::Node* >( source.lock().get() );
    qan::Node* dst = static_cast< qan::Node* >( destination.lock().get() );
    qan::Edge* edge = nullptr;
    if ( src != nullptr && dst != nullptr )
        edge = insertEdge( QString::fromStdString( className ), src, dst );
    return ( edge != nullptr ? edge->shared_from_this() : WeakEdge() ); // Note 20160213: shared_from_this() could be used since the edge has already been added to graph, it
}

void    Graph::removeEdge( qan::Node* source, qan::Node* destination )
{
    if ( source == nullptr || destination == nullptr )
        return;
    WeakNode sharedSource;
    WeakNode sharedDestination;
    try {
        sharedSource = WeakNode{ source->shared_from_this() };
        sharedDestination = WeakNode{ destination->shared_from_this() };
    } catch ( std::bad_weak_ptr ) { return; }
    return GTpoGraph::removeEdge( sharedSource, sharedDestination );
}

void    Graph::removeEdge( qan::Edge* edge )
{
    if ( edge != nullptr )
        GTpoGraph::removeEdge( edge->shared_from_this() );
}

bool    Graph::hasEdge( qan::Node* source, qan::Node* destination ) const
{
    if ( source == nullptr || destination == nullptr )
        return false;
    WeakNode sharedSource;
    WeakNode sharedDestination;
    try {
        sharedSource = WeakNode{ source->shared_from_this() };
        sharedDestination = WeakNode{ destination->shared_from_this() };
    } catch ( std::bad_weak_ptr e ) { return false; }
    return GTpoGraph::hasEdge( sharedSource, sharedDestination );
}
//-----------------------------------------------------------------------------

/* Graph Group Management *///-------------------------------------------------
Graph::WeakGroup   Graph::createGroup( const std::string& className )
{
    if ( className.size() == 0 )
        return WeakGroup{};
    qan::Group* group = insertGroup( QString::fromStdString( className ) );
    if ( group != nullptr ) {
        connect( group, &qan::Group::groupClicked, this, &qan::Graph::groupClicked );
        connect( group, &qan::Group::groupRightClicked, this, &qan::Graph::groupRightClicked );
        connect( group, &qan::Group::groupDoubleClicked, this, &qan::Graph::groupDoubleClicked );
    }
    return ( group != nullptr ? group->shared_from_this() : WeakGroup{} );
}

qan::Group* Graph::insertGroup( QString className )
{
    QQmlComponent* groupComponent{ nullptr };
    groupComponent = _groupClassComponents.value( className, nullptr );
    if ( groupComponent == nullptr ) {
        qDebug() << "qan::Graph::insertGroup(): Error: Can't find a valid group delegate component.";
        return nullptr;
    }
    qan::Group* group = static_cast< qan::Group* >( createFromDelegate( groupComponent ) );
    if ( group != nullptr ) {
        GTpoGraph::insertGroup( std::shared_ptr<qan::Group>{group} );
        //QQmlEngine::setObjectOwnership( group, QQmlEngine::CppOwnership );

        connect( group, &qan::Group::groupClicked, this, &qan::Graph::groupClicked );
        connect( group, &qan::Group::groupRightClicked, this, &qan::Graph::groupRightClicked );
        connect( group, &qan::Group::groupDoubleClicked, this, &qan::Graph::groupDoubleClicked );
    } else
        qDebug() << "qan::Graph::insertGroup(): Warning: Group creation failed.";

    return group;
}

void    Graph::removeGroup( qan::Group* group )
{
    if ( group == nullptr )
        return;
    // Reparent all group childrens (ie node) to graph before destructing the group
    // otherwise all child iems get destructed too
    auto groupNodes{group->getNodes()};
    for ( auto& node : groupNodes ) {
        auto nodePtr{ node.lock() };
        if ( nodePtr != nullptr ) {
            nodePtr->ungroup();
            nodePtr->setParentItem( this );
        }
    }
    WeakGroup weakGroup = group->shared_from_this();
    if ( !weakGroup.expired() )
        gtpo::GenGraph< Config >::removeGroup( weakGroup );
}

bool    Graph::hasGroup( qan::Group* group ) const
{
    if ( group == nullptr )
        return false;
    WeakGroup weakGroup;
    try {
        weakGroup = WeakGroup{ group->shared_from_this() };
    } catch ( std::bad_weak_ptr ) { return false; }
    return gtpo::GenGraph< qan::Config >::hasGroup( weakGroup );
}
//-----------------------------------------------------------------------------

/* Graph Initialization Management *///----------------------------------------
void    Graph::initializeRandom( int nodeCount,
                                 int   minOutNodes, int maxOutNodes,
                                 qreal minWidth, qreal maxWidth,
                                 qreal minHeight, qreal maxHeight,
                                 QRectF br )
{
    if ( !br.isValid() || br.isEmpty() )
        return;
    RandomGraph::RandomConfig rc;
    rc.nodeClassName = "qan::Node";
    rc.edgeClassName = "qan::Edge";
    rc.nodeCount = nodeCount;
    rc.outNodeRng = std::make_pair( minOutNodes, maxOutNodes );
    rc.widthRng = std::make_pair( minWidth, maxWidth );
    rc.heightRng = std::make_pair( minHeight, maxHeight );
    rc.xRng = std::make_pair( 0, br.width() - maxWidth );
    rc.yRng = std::make_pair( 0, br.height() - maxHeight );
    RandomGraph::generate< qan::Graph >( *this, rc );
    qreal nz{0.};
    for ( auto& node : getNodes() ) {
        node->setZ(nz);   // Force valid z values
        nz += 1.0;
    }
}
//-----------------------------------------------------------------------------

} // ::qan

