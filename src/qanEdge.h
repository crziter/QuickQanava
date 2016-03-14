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
// This file is a part of the Qanava software.
//
// \file	qanEdge.h
// \author	benoit@qanava.org
// \date	2004 February 15
//-----------------------------------------------------------------------------

#ifndef qanEdge_h
#define qanEdge_h

// Qt headers
#include <QLineF>

// Qanava headers
#include "./qanConfig.h"
#include "./qanStyle.h"

namespace qan { // ::qan

class Node;

//! Weighted directed edge linking two nodes in a graph.
/*!
    \nosubgrouping
 */
class Edge : public gtpo::GenEdge< QGraphConfig >
{
    /*! \name Edge Object Management *///--------------------------------------
    //@{
    Q_OBJECT
public:
    //! Edge constructor with source, destination and weight initialization.
    Edge( QQuickItem* parent = nullptr );

    //! Return "qan::Edge".
    std::string getClassName() { return "qan::Edge"; }
    //@}
    //-------------------------------------------------------------------------

    /*! \name Edge Topology Management *///------------------------------------
    //@{
public:
    void        setSourceItem( qan::Node* source );
    void        setDestinationItem( qan::Node* destination );
public:
    Q_PROPERTY( qan::Node* sourceItem READ getSourceItem WRITE setSourceItem NOTIFY sourceItemChanged )
    qan::Node*  getSourceItem( ) { return static_cast< qan::Node* >( getSrc().lock().get() ); }
signals:
    void        sourceItemChanged( );

public:
    Q_PROPERTY( qan::Node* destinationItem READ getDestinationItem() WRITE setDestinationItem NOTIFY destinationItemChanged )
    qan::Node*  getDestinationItem( ) { return static_cast< qan::Node* >( getDst().lock().get() ); }
signals:
    void        destinationItemChanged( );
    //@}
    //-------------------------------------------------------------------------

    /*! \name Edge Drawing Management *///-------------------------------------
    //@{
public:
    virtual void        paint( QPainter* painter ) override;
public slots:
    virtual void        updateItem( );
public:
    //! Internally used from QML to set src and dst and display an unitialized edge for previewing edges styles.
    Q_INVOKABLE void    setLine( QPoint src, QPoint dst );
private:
    QLineF              _line;
protected:
    QLineF              getPolyLineIntersection( const QLineF line, const QPolygonF srcBp, const QPolygonF dstBp ) const;
    void                drawArrow( QPainter* painter, QLineF line, QColor color, float arrowSize ) const;
    //@}
    //-------------------------------------------------------------------------

    /*! \name Mouse Management *///--------------------------------------------
    //@{
protected:
    virtual void    mouseDoubleClickEvent( QMouseEvent* event ) override;
    virtual void    mousePressEvent( QMouseEvent* event ) override;
signals:
    void            edgeHoverPopupEvent( QPointF p );
    void            edgeHoverPopupLeaveEvent( QPointF p );
    void            edgeClicked( QVariant edge, QVariant pos );
    void            edgeRightClicked( QVariant edge, QVariant pos );
    void            edgeDoubleClicked( QVariant edge, QVariant pos );
private:
    inline qreal    distanceFromLine( const QPointF& p, const QLineF& line ) const;

public:
    //! Edge label position.
    Q_PROPERTY( QPointF labelPos READ getLabelPos WRITE setLabelPos NOTIFY labelPosChanged )
    //! Get edge label position.
    QPointF		getLabelPos( ) const { return _labelPos; }
    //! Set edge label position.
    void		setLabelPos( QPointF labelPos ) { _labelPos = labelPos; emit labelPosChanged(); }
protected:
    //! \sa labelPos
    QPointF     _labelPos;
signals:
    //! \sa labelPos
    void        labelPosChanged( );
    //@}
    //-------------------------------------------------------------------------

    /*! \name Style and Properties Management *///-----------------------------
    //@{
private:
    using SharedEdgeStyle = QSharedPointer< qan::EdgeStyle >;
    SharedEdgeStyle _defaultStyle;
public:
    //! Edge current style object (this property is never null, a default style is returned when no style has been manually set).
    Q_PROPERTY( qan::EdgeStyle* style READ getStyle WRITE setStyle NOTIFY styleChanged )
    void            setStyle( EdgeStyle* style );
    qan::EdgeStyle* getStyle( ) const { return _style; }
private:
    qan::EdgeStyle* _style;
signals:
    void            styleChanged( );
private slots:
    //! Called when the style associed to this edge is destroyed.
    void            styleDestroyed( QObject* style );

public:
    Q_PROPERTY( QString label READ getLabel WRITE setLabel NOTIFY labelChanged )
    //! Set this edge label.
    void            setLabel( const QString& label ) { _label = label; emit labelChanged( ); }
    //! Get this edge label.
    const QString&  getLabel( ) const { return _label; }
protected:
    QString         _label = QString( "" );
signals:
    void            labelChanged( );

public:
    Q_PROPERTY( qreal weight READ getWeight WRITE setWeight NOTIFY weightChanged )
    //! Get edge's weight.
    float		getWeight( ) const { return _weight; }
    //! Set edge's weight.
    void		setWeight( float weight ) { _weight = weight; }
protected:
    qreal       _weight = 1.0;
signals:
    void        weightChanged( );
    //@}
    //-------------------------------------------------------------------------

    /*! \name Drag'nDrop Management  *///--------------------------------------
    //@{
public:
    /*! Define if the edge actually accept drops.
     *
     * When set to false, the edge will not be styleable by DnD and hyper edge drop connector
     * won't works.
     *
     * Default to true.
     *
     * Setting this property to false may lead to a significant performance improvement if DropNode support is not needed.
     */
    Q_PROPERTY( bool acceptDrops READ getAcceptDrops WRITE setAcceptDrops NOTIFY acceptDropsChanged )
    void             setAcceptDrops( bool acceptDrops ) { _acceptDrops = acceptDrops; setFlag( QQuickItem::ItemAcceptsDrops, acceptDrops ); emit acceptDropsChanged( ); }
    bool             getAcceptDrops( ) { return _acceptDrops; }
private:
    bool            _acceptDrops = true;
signals:
    void            acceptDropsChanged( );

protected:

    virtual bool    contains( const QPointF& point ) const override;

    /*! Internally used to manage drag and drop over nodes, override with caution, and call base class implementation.
     *
     * Drag enter event are not restricted to the edge bounding rect but to the edge line with a distance delta, computing
     * that value is quite coslty, if you don't need to accept drops, use setAcceptDrops( false ).
     */
    virtual void    dragEnterEvent( QDragEnterEvent* event ) override;
    //! Internally used to manage drag and drop over nodes, override with caution, and call base class implementation.
    virtual void    dragMoveEvent( QDragMoveEvent* event ) override;
    //! Internally used to manage drag and drop over nodes, override with caution, and call base class implementation.
    virtual void    dragLeaveEvent( QDragLeaveEvent* event ) override;
    //! Internally used to accept style drops.
    virtual void    dropEvent( QDropEvent* event ) override;
    //@}
    //-------------------------------------------------------------------------
};

} // ::qan

QML_DECLARE_TYPE( qan::Edge )

#endif // qanEdge_h